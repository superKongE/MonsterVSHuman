// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "Net/UnrealNetwork.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/NetSerialization.h"
#include "DrawDebugHelpers.h"
#include "Camera/CameraComponent.h"
#include "Sound/SoundCue.h"
#include "GameFramework/Pawn.h"

#include "CombatState.h" 
#include "MyGame/PlayerController/CharacterController.h"
#include "MyGame/Character/MyAnimInstance.h"
#include "MyGame/Weapon.h"
#include "MyGame/Weapon/ShotgunWeapon.h"
#include "MyGame/Character/FPSCharacter.h"
#include "MyGame/Character/FPSMonster.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	CombatState = ECombatState::CS_Idle;

	CarriedAmmo.Add(EWeaponType::ET_Rifle, CurrentCarriedWeaponAmmo);
	CarriedAmmo.Add(EWeaponType::ET_Rocket, CurrentCarriedRocketAmmo);
	CarriedAmmo.Add(EWeaponType::ET_Gun, CurrentCarriedGunAmmo);
	CarriedAmmo.Add(EWeaponType::ET_ShotGun, CurrentCarriedShotGunAmmo);
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (Character != nullptr)
	{
		if (Character->GetCamera())
		{
			DefaultFOV = Character->GetCamera()->FieldOfView; // FieldOfView ���� ī�޶��� �þ߰�
			CurrentFOV = DefaultFOV;
		}
	}
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, CombatState);
	DOREPLIFETIME(UCombatComponent, bAiming);
	DOREPLIFETIME_CONDITION(UCombatComponent, CurrentCarriedAmmo, COND_OwnerOnly);
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (Character && Character->IsLocallyControlled())
	{
		TraceUnderCrosshair(HitResult);
		HitTarget = HitResult.ImpactPoint; // �浹 ���� ����

		SetCrosshairHUD(DeltaTime); // ũ�ν���� ����

		InterpCameraFOV(DeltaTime); // ī�޶� �� �� �ƿ� ����
	}
}

// ���������� ȣ���
// ���� ���� �� ����
void UCombatComponent::EquipWeapon(AWeapon* Weapon)
{
	if (Character == nullptr || Weapon == nullptr) return;

	if (EquippedWeapon)
	{
		EquippedWeapon->DropWeapon();
	}

	EquippedWeapon = Weapon;
	EquippedWeapon->SetWeaponState(EWeaponState::WS_Equipped);

	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("WeaponAttachPointR"));
	if (HandSocket)
	{
		if (HandSocket->AttachActor(EquippedWeapon, Character->GetMesh()))
		{
			Controller = Controller == nullptr ? Cast<ACharacterController>(Character->Controller) : Controller;
			if (Controller)
			{
				UGameplayStatics::PlaySoundAtLocation(this, EquippedWeapon->GetEquippedSoundCue(), EquippedWeapon->GetActorLocation());
				EquippedWeapon->SetWeaponState(EWeaponState::WS_Equipped);//
				EquippedWeapon->SetOwner(Character);
				Character->SetCurrentWeaponType(EquippedWeapon->GetWeaponType());

				if (Character->IsLocallyControlled())
				{
					Controller->SetHUDWeaponType(EquippedWeapon->GetWeaponType());
					CurrentCarriedAmmo = CarriedAmmo[EquippedWeapon->GetWeaponType()];					
					Controller->SetHUDCarriedAmmo(CurrentCarriedAmmo);
					EquippedWeapon->SetHUDAmmo();
				}
			}
		}
		else
		{
			EquippedWeapon = nullptr;
		}
	}
}
void UCombatComponent::OnRep_EquippedWeapon()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;

	Character->SetCurrentWeaponType(EquippedWeapon->GetWeaponType());
	UGameplayStatics::PlaySoundAtLocation(this, EquippedWeapon->GetEquippedSoundCue(), EquippedWeapon->GetActorLocation());

	Controller = Controller == nullptr ? Cast<ACharacterController>(Character->GetController()) : Controller;
	if (Controller == nullptr) return;
	if (Character->IsLocallyControlled())
	{
		Controller->SetHUDWeaponType(EquippedWeapon->GetWeaponType());
		CurrentCarriedAmmo = CarriedAmmo[EquippedWeapon->GetWeaponType()];
		Controller->SetHUDCarriedAmmo(CurrentCarriedAmmo);
		EquippedWeapon->SetHUDAmmo();
	}
}


// �Ѿ� �߻� ���� �Լ���
void UCombatComponent::TraceUnderCrosshair(FHitResult& TraceHitResult)
{
	if (EquippedWeapon == nullptr) return;

	// ����Ʈ ũ�� ���ϱ�
	FVector2D ViewportSize = FVector2D::ZeroVector;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	// ũ�ν���� ��ġ ����
	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	// 2d ���� ������ ũ�ν���� ��ġ��  3d ���� ������ ��ȯ
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection
	);

	if (bScreenToWorld)
	{
		// ũ�ν��� 3d ���������� ������
		FVector Start = CrosshairWorldPosition;

		if (Character)
		{
			// ũ�ν����� ĳ���Ͱ��� �Ÿ�
			float DistanceToCharacter = (Character->GetActorLocation() - Start).Size();
			Start += CrosshairWorldDirection * (DistanceToCharacter + 100.f);
		}

		// ũ�ν��� 3d ���������� ����
		FVector End = Start + CrosshairWorldDirection * 8000.f;

		// Start ���� End ���� �浹�˻�
		FCollisionQueryParams FQS;
		FQS.AddIgnoredActor(GetOwner());
		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility,
			FQS
		);

		// �浹�� ����� ���� ���
		if (!TraceHitResult.bBlockingHit)
		{
			IsEnemyAimed = false;

			TraceHitResult.ImpactPoint = End;
			HUDPackage.CrosshairColor = FLinearColor::White;
		}
		else
		{
			if (AFPSMonster* tempMonster = Cast<AFPSMonster>(TraceHitResult.GetActor()))
			{
				if (tempMonster->GetIsDissovle())
				{
					HUDPackage.CrosshairColor = FLinearColor::White;
				}
				else
				{
					HUDPackage.CrosshairColor = FLinearColor::Red;
				}

				IsEnemyAimed = true;
			}
			else
			{
				IsEnemyAimed = false;
				HUDPackage.CrosshairColor = FLinearColor::White;
			}
		}
	}
}
// �� �߻� ��ư�� ������ ȣ���
void UCombatComponent::Fire(bool bfire)
{
	FireButtonPressed = bfire;

	// ���� �����ϰ� �ְ�, �� ��� �����̰� ���� ���°�, ���� ��� �ִ� ���¸�
	if (FireButtonPressed && CanShoot && EquippedWeapon && CanFire())
	{
		CanShoot = false;

		// �� ������ ���� ȣ��Ǵ� �Լ��� ����
		switch (EquippedWeapon->GetFireType())
		{
		case EFireType::EFT_Projectile: // ����, ����
			FireProjectileHitWeapon();
			break;

		case EFireType::EFT_HitScan: // ����
			FireHitScanWeapon();
			break;

		case EFireType::EFT_Shotgun: // ����
			FireShotgun();
			break;
		}

		FireStart(); // ������ ����
	}
}
// �� ��� �����̸� ���� Ÿ�̸� ȣ��(ó������ �ٷ� �����)
void UCombatComponent::FireStart()
{
	if (EquippedWeapon)
	{
		Character->GetWorldTimerManager().SetTimer(
			FireTimer,
			this,
			&UCombatComponent::FireEnd,
			EquippedWeapon->GetFireDelay()
		);
	}
}
// �����̰� ������ �߻�
void UCombatComponent::FireEnd()
{
	CanShoot = true;

	// �ڵ����� ������ �ٽ� Frie() ȣ��
	if (EquippedWeapon && FireButtonPressed && EquippedWeapon->IsAutomaticShoot())
	{
		Fire(FireButtonPressed);
	}
}
// ���� �߻��� �� �ִ� �������� ��ȯ
bool UCombatComponent::CanFire()
{
	if (Character == nullptr || EquippedWeapon == nullptr || Character->GetCharacterMovement()->IsFalling() ||
		Character->GetMoveStateHuman() == MoveStateHuman::EMS_Run || EquippedWeapon->GetAmmo() <= 0) return false;

	// ������ ��� �������϶� �߻� ����
	if (EquippedWeapon->GetWeaponType() == EWeaponType::ET_ShotGun && CombatState == ECombatState::CS_Reloading)
	{
		IsReloading = false;
		return true;
	}
	else
	{
		if (CombatState != ECombatState::CS_Idle) return false;
	}

	return true;
}
void UCombatComponent::FireShotgun()
{
	AShotgunWeapon* Shotgun = Cast<AShotgunWeapon>(EquippedWeapon);
	if (Shotgun)
	{
		TArray<FVector_NetQuantize> HitTargets;
		if (!bAiming)
		{
			Shotgun->ShotgunTraceEndWithScatter(HitTarget, HitTargets);
		}
		else
		{
			Shotgun->ShotgunTraceEndWithScatter_Zoomed(HitTarget, HitTargets);
		}

		// ���� ��Ű��� �ʾ����� Ŭ���̾�Ʈ �󿡼��� �ٷ� ���̱� ���� Local �Լ� ȣ��
		LocalShotgunFire(HitTargets);
		ServerShotgunFire(HitTargets);
	}
}
void UCombatComponent::FireHitScanWeapon()
{
	if (EquippedWeapon)
	{
		HitTarget = EquippedWeapon->GetUserScatter() ? EquippedWeapon->TraceEndWidthScatter(HitTarget) : HitTarget;
		LocalFire(HitTarget);
		ServerFire(HitTarget);
	}
}
void UCombatComponent::FireProjectileHitWeapon()
{
	LocalFire(HitTarget);
	ServerFire(HitTarget);
}
void UCombatComponent::LocalFire(const FVector_NetQuantize& TraceHitTarget)
{
	if (EquippedWeapon == nullptr && Character && Character->HasAuthority()) return;

	if (Character && CombatState == ECombatState::CS_Reloading && EquippedWeapon->GetWeaponType() == EWeaponType::ET_ShotGun)
	{
		Character->PlayFireAnimation();
		EquippedWeapon->Fire(TraceHitTarget);
		CombatState = ECombatState::CS_Idle;
		return;
	}

	if (Character && EquippedWeapon)
	{
		Character->PlayFireAnimation();
		EquippedWeapon->Fire(TraceHitTarget);
	}
}
void UCombatComponent::LocalShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	if (Character && EquippedWeapon)
	{
		AShotgunWeapon* Shotgun = Cast<AShotgunWeapon>(EquippedWeapon);
		if (Shotgun)
		{
			Character->PlayFireAnimation();
			Shotgun->FireShotgun(TraceHitTargets);
			CombatState = ECombatState::CS_Idle;
		}
	}
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	MultiCastFire(TraceHitTarget);
}
void UCombatComponent::MultiCastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	// �̹� LocalFire()�� ȣ�������ν� Ŭ���̾�Ʈ���� ����Ǿ��� ������ Ŭ���̾�Ʈ�� ��� ��ŵ
	if (EquippedWeapon == nullptr || Character == nullptr || Character->IsLocallyControlled()) return;

	/*if (Character && CombatState == ECombatState::CS_Reloading && EquippedWeapon->GetWeaponType() == EWeaponType::ET_ShotGun)
	{
		Character->PlayFireAnimation();
		EquippedWeapon->Fire(TraceHitTarget);
		CombatState = ECombatState::CS_Idle;
		return;
	}*/

	if (Character && EquippedWeapon)
	{
		Character->PlayFireAnimation();
		EquippedWeapon->Fire(TraceHitTarget);
	}
}
void UCombatComponent::ServerShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	MulticastShotgunFire(TraceHitTargets);
}
void UCombatComponent::MulticastShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	if (Character == nullptr || Character->IsLocallyControlled()) return;

	if (EquippedWeapon)
	{
		AShotgunWeapon* Shotgun = Cast<AShotgunWeapon>(EquippedWeapon);
		if (Shotgun)
		{
			Character->PlayFireAnimation();
			Shotgun->FireShotgun(TraceHitTargets);
			CombatState = ECombatState::CS_Idle;
		}
	}
}


// ������ ���� �Լ���
// �������Ʈ���� ȣ��Ǵ� �Լ�
void UCombatComponent::shotgunShellReload()
{
	if (Character && Character->HasAuthority())
	{
		UpdateShotgunAmmoValue();
	}
}
// �������Ʈ���� ȣ��Ǵ� �Լ�
void UCombatComponent::shotgunShellReloadEnd()
{
	if (Character == nullptr) return;

	if (EquippedWeapon && !EquippedWeapon->IsFull() && CarriedAmmo[EquippedWeapon->GetWeaponType()] > 0)
	{
		UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
		if (AnimInstance && Character->GetReloadMontage())
		{
			AnimInstance->Montage_JumpToSection(FName("ReloadShotgunStartHip"));
		}
	}
}
// �÷��̾� ���¼� ���� ȣ���
void UCombatComponent::Reload()
{
	//									������ �Ѿ��� �ְ�             �Ѿ��� ���� ���� ������
	if (Character && EquippedWeapon && CurrentCarriedAmmo > 0 && EquippedWeapon->GetAmmo() < EquippedWeapon->GetMagCapcity())
	{
		// �Ѿ� �� ���
		ServerReload();
		Character->PlayReloadAnimation();
		IsReloading = true;
	}
}
void UCombatComponent::ServerReload_Implementation()
{
	// OnRep_CombatState ȣ���
	CombatState = ECombatState::CS_Reloading;
	if (!Character->IsLocallyControlled())
	{
		Character->PlayReloadAnimation();
		IsReloading = true;
	}
}
void UCombatComponent::OnRep_CombatState()
{
	switch (CombatState)
	{
	case ECombatState::CS_Idle:
		IsReloading = false;
		if (FireButtonPressed)
		{
			Fire(true);
		}
		break;

	case ECombatState::CS_Reloading:
		if (Character && !Character->IsLocallyControlled())
		{
			IsReloading = true;
			Character->PlayReloadAnimation();
		}
		break;
	}
}
// ���� �ִϸ��̼��� ������ �߻��ϴ� Notify�� ���� ȣ��Ǵ� �Լ�
void UCombatComponent::EndReload()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;
	if (Character->HasAuthority())
	{
		UpdateAmmoValue();
	}

	CombatState = ECombatState::CS_Idle;
	IsReloading = false;

	if (FireButtonPressed)
	{
		Fire(true);
	}
}
// ���� �ִϸ��̼��� ������ �߻��ϴ� Notify�� ���� ȣ��Ǵ� �Լ�
void UCombatComponent::ShotgunEndReload()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;

	CombatState = ECombatState::CS_Idle;
	IsReloading = false;

	if (FireButtonPressed)
	{
		Fire(true);
	}
}
// ������ �ʿ��� �Ѿ� ������ ��ȯ
int32 UCombatComponent::AmountToReload()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return 0;

	int32 RoomInMag = EquippedWeapon->GetMagCapcity() - EquippedWeapon->GetAmmo();
	if (CarriedAmmo.Contains(EquippedWeapon->GetWeaponType()))
	{
		int32 AmmountCarried = CarriedAmmo[EquippedWeapon->GetWeaponType()];
		int32 Least = FMath::Min(RoomInMag, AmmountCarried);

		return FMath::Clamp(RoomInMag, 0, Least);
	}

	return 0;
}
void UCombatComponent::UpdateAmmoValue()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;

	// �������� �ʿ��� �Ѿ� ��
	int ReloadAmount = AmountToReload();

	if (CarriedAmmo.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmo[EquippedWeapon->GetWeaponType()] -= ReloadAmount;
		CurrentCarriedAmmo = CarriedAmmo[EquippedWeapon->GetWeaponType()];
	}

	Controller = Controller == nullptr ? Cast<ACharacterController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CurrentCarriedAmmo);
	}

	EquippedWeapon->AddAmmo(ReloadAmount);
}
void UCombatComponent::UpdateShotgunAmmoValue()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;

	if (CarriedAmmo.Contains(EquippedWeapon->GetWeaponType()) && CarriedAmmo[EquippedWeapon->GetWeaponType()] > 0)
	{
		CarriedAmmo[EquippedWeapon->GetWeaponType()] -= 1;
		CurrentCarriedAmmo = CarriedAmmo[EquippedWeapon->GetWeaponType()];
	}

	Controller = Controller == nullptr ? Cast<ACharacterController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CurrentCarriedAmmo);
	}

	EquippedWeapon->AddAmmo(1);
	JumpToShotgfunEnd();
}
// ���� ������ ������ �� �ưų� ���� ź���� ������ ���� ����
void UCombatComponent::JumpToShotgfunEnd()
{
	if (EquippedWeapon->IsFull() || CurrentCarriedAmmo == 0)
	{
		UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
		if (AnimInstance && Character->GetReloadMontage())
		{
			AnimInstance->Montage_JumpToSection(FName("ReloadShotgunHipEnd"));
		}
	}
}


// ũ�ν���� ������ ���� ����(�ٰų�, �����ϸ� ������ ������)
void UCombatComponent::SetCrosshairHUD(float DeltaTime)
{
	if (Character == nullptr || Character->Controller == nullptr) return;

	Controller = Controller == nullptr ? Cast<ACharacterController>(Character->Controller) : Controller;

	if (Controller)
	{
		HUD = HUD == nullptr ? Cast<ACharacterHUD>(Controller->GetHUD()) : HUD;

		if (HUD)
		{
			if (EquippedWeapon)
			{
				HUDPackage.CrosshairCenter = EquippedWeapon->CrosshairCeneter;
				HUDPackage.CrosshairTop = EquippedWeapon->CrosshairTop;
				HUDPackage.CrosshairRight = EquippedWeapon->CrosshairRight;
				HUDPackage.CrosshairLeft = EquippedWeapon->CrosshairLeft;
				HUDPackage.CrosshairBottom = EquippedWeapon->CrosshairBottom;
			}
			else
			{
				HUDPackage.CrosshairCenter = nullptr;
				HUDPackage.CrosshairTop = nullptr;
				HUDPackage.CrosshairRight = nullptr;
				HUDPackage.CrosshairLeft = nullptr;
				HUDPackage.CrosshairBottom = nullptr;

				HUD->SetHUDPackage(HUDPackage);
				return;
			}

			if (Character->GetVelocity().Size() > 400)
			{
				VelocityFactor = FMath::FInterpTo(VelocityFactor, 1.5f, DeltaTime, 1.5f);
			}
			else
			{
				VelocityFactor = FMath::FInterpTo(VelocityFactor, 0.f, DeltaTime, 30.f);
			}

			if (Character->GetIsInAir())
			{
				AirFactor = FMath::FInterpTo(AirFactor, 2.25f, DeltaTime, 2.25f);
			}
			else
			{
				AirFactor = FMath::FInterpTo(AirFactor, 0.f, DeltaTime, 30.f);
			}

			if (bAiming)
			{
				AimFactor = FMath::FInterpTo(AimFactor, 0.58f, DeltaTime, 30.f);
			}
			else
			{
				AimFactor = FMath::FInterpTo(AimFactor, 0.f, DeltaTime, 30.f);
			}

			HUDPackage.CrosshairSpread = 1.f + VelocityFactor + AirFactor - AimFactor;

			HUD->SetHUDPackage(HUDPackage);
		}
	}
}
// ī�޶� �þ߰� Ȯ�� �� ��� (Ŭ���̾�Ʈ������ ȣ���)
void UCombatComponent::InterpCameraFOV(float DeltaTime)
{
	if (EquippedWeapon == nullptr) return;

	// ���� ���¸� Ȯ��
	if (bAiming)
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, 50, DeltaTime, EquippedWeapon->GetZoomInterpSpeed());
	}

	else
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomInterpSpeed);
	}

	if (Character)
	{
		// ī�޶� �þ߰� ����(Ȯ�� �� ���)
		Character->GetCamera()->SetFieldOfView(CurrentFOV);
	}
}
void UCombatComponent::SetAiming(bool bIsAiming)
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;

	bAiming = bIsAiming;
	ServerSetAiming(bIsAiming);

	//Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : Character->GetWalkSpeed();

	if (Character->IsLocallyControlled())
	{
		bAimButtonPressed = bIsAiming;
	}
}
void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
}


void UCombatComponent::OnRep_CurrentCarriedAmmo()
{
	Controller = Controller == nullptr ? Cast<ACharacterController>(Character->Controller) : Controller;

	if (Controller && EquippedWeapon)
	{
		//EquippedWeapon->SetHUDAmmo();
		CarriedAmmo[EquippedWeapon->GetWeaponType()] = CurrentCarriedAmmo;
		Controller->SetHUDCarriedAmmo(CurrentCarriedAmmo);
	}

	if (CombatState == ECombatState::CS_Reloading && EquippedWeapon && EquippedWeapon->GetWeaponType() == EWeaponType::ET_ShotGun && CurrentCarriedAmmo == 0)
	{
		UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
		if (AnimInstance && Character->GetReloadMontage())
		{
			AnimInstance->Montage_JumpToSection(FName("ReloadShotgunHipEnd"));
		}
	}
}
