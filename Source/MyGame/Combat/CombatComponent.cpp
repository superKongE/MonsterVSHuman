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
			DefaultFOV = Character->GetCamera()->FieldOfView; // FieldOfView 현재 카메라의 시야각
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
		HitTarget = HitResult.ImpactPoint; // 충돌 지점 저장

		SetCrosshairHUD(DeltaTime); // 크로스헤어 설정

		InterpCameraFOV(DeltaTime); // 카메라 줌 인 아웃 설정
	}
}

// 서버에서만 호출됨
// 무기 장착 및 해제
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


// 총알 발사 관련 함수들
void UCombatComponent::TraceUnderCrosshair(FHitResult& TraceHitResult)
{
	if (EquippedWeapon == nullptr) return;

	// 뷰포트 크기 구하기
	FVector2D ViewportSize = FVector2D::ZeroVector;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	// 크로스헤어 위치 지정
	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	// 2d 공간 정보인 크로스헤어 위치를  3d 공간 정보로 변환
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection
	);

	if (bScreenToWorld)
	{
		// 크로스헤어가 3d 공간에서의 시작점
		FVector Start = CrosshairWorldPosition;

		if (Character)
		{
			// 크로스헤어와 캐릭터간의 거리
			float DistanceToCharacter = (Character->GetActorLocation() - Start).Size();
			Start += CrosshairWorldDirection * (DistanceToCharacter + 100.f);
		}

		// 크로스헤어가 3d 공간에서의 끝점
		FVector End = Start + CrosshairWorldDirection * 8000.f;

		// Start 부터 End 까지 충돌검사
		FCollisionQueryParams FQS;
		FQS.AddIgnoredActor(GetOwner());
		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility,
			FQS
		);

		// 충돌한 대상이 없을 경우
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
// 총 발사 버튼을 누르면 호출됨
void UCombatComponent::Fire(bool bfire)
{
	FireButtonPressed = bfire;

	// 총을 소지하고 있고, 총 쏘는 딜레이가 끝난 상태고, 총을 쏠수 있는 상태면
	if (FireButtonPressed && CanShoot && EquippedWeapon && CanFire())
	{
		CanShoot = false;

		// 총 종류에 따라 호출되는 함수를 설정
		switch (EquippedWeapon->GetFireType())
		{
		case EFireType::EFT_Projectile: // 소총, 로켓
			FireProjectileHitWeapon();
			break;

		case EFireType::EFT_HitScan: // 권총
			FireHitScanWeapon();
			break;

		case EFireType::EFT_Shotgun: // 샷건
			FireShotgun();
			break;
		}

		FireStart(); // 딜레이 설정
	}
}
// 총 쏘는 딜레이를 위해 타이머 호출(처음에는 바로 실행됨)
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
// 딜레이가 끝나면 발사
void UCombatComponent::FireEnd()
{
	CanShoot = true;

	// 자동연사 무기라면 다시 Frie() 호출
	if (EquippedWeapon && FireButtonPressed && EquippedWeapon->IsAutomaticShoot())
	{
		Fire(FireButtonPressed);
	}
}
// 총을 발사할 수 있는 상태인지 반환
bool UCombatComponent::CanFire()
{
	if (Character == nullptr || EquippedWeapon == nullptr || Character->GetCharacterMovement()->IsFalling() ||
		Character->GetMoveStateHuman() == MoveStateHuman::EMS_Run || EquippedWeapon->GetAmmo() <= 0) return false;

	// 샷건일 경우 장전중일때 발사 가능
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

		// 서버 통신간에 늦어져도 클라이언트 상에서는 바로 보이기 위해 Local 함수 호출
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
	// 이미 LocalFire()을 호출함으로써 클라이언트에서 실행되었기 때문에 클라이언트일 경우 스킵
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


// 재장전 관련 함수들
// 블루프린트에서 호출되는 함수
void UCombatComponent::shotgunShellReload()
{
	if (Character && Character->HasAuthority())
	{
		UpdateShotgunAmmoValue();
	}
}
// 블루프린트에서 호출되는 함수
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
// 플레이어 한태서 직접 호출됨
void UCombatComponent::Reload()
{
	//									소지한 총알이 있고             총알이 꽉차 있지 않으면
	if (Character && EquippedWeapon && CurrentCarriedAmmo > 0 && EquippedWeapon->GetAmmo() < EquippedWeapon->GetMagCapcity())
	{
		// 총알 양 계산
		ServerReload();
		Character->PlayReloadAnimation();
		IsReloading = true;
	}
}
void UCombatComponent::ServerReload_Implementation()
{
	// OnRep_CombatState 호출됨
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
// 장전 애니메이션이 끝나면 발생하는 Notify에 의해 호출되는 함수
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
// 장전 애니메이션이 끝나면 발생하는 Notify에 의해 호출되는 함수
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
// 장전에 필요한 총알 갯수를 반환
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

	// 재장전에 필요한 총알 양
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
// 샷건 장전중 장전이 다 됐거나 남은 탄약이 없으면 장전 종료
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


// 크로스헤어 벌어짐 정도 설정(뛰거나, 점프하면 에임이 벌어짐)
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
// 카메라 시야각 확대 및 축소 (클라이언트에서만 호출됨)
void UCombatComponent::InterpCameraFOV(float DeltaTime)
{
	if (EquippedWeapon == nullptr) return;

	// 조준 상태면 확대
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
		// 카메라 시야각 설정(확대 및 축소)
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
