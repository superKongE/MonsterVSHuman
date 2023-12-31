
#include "Weapon.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "MyGame/Character/FPSCharacter.h"
#include "MyGame/Combat/CombatComponent.h"
#include "Net/UnrealNetwork.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "MyGame/PlayerController/CharacterController.h"
#include "Kismet/KismetMathLibrary.h"

AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Weapon Mesh"));
	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RootComponent = WeaponMesh;

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Capsule Component"));
	AreaSphere->SetupAttachment(RootComponent);
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PickUpWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickUpWidget->SetupAttachment(RootComponent);

	WeaponState = EWeaponState::WS_Dropped;
}
void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	Owner = nullptr;
	
	if (GetLocalRole() == ENetRole::ROLE_Authority)
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap);
		AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOverlap);
	}

	if (PickUpWidget)
	{
		PickUpWidget->SetVisibility(false);
	}
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon, WeaponState);
	//DOREPLIFETIME(AWeapon, Ammo);
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// 서버에서만 호출됨
void AWeapon::OnSphereOverlap(class UPrimitiveComponent* OverlappedComponent, class AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AFPSCharacter* TempCharacter = Cast<AFPSCharacter>(OtherActor);

	if (TempCharacter)
	{
		TempCharacter->SetOverlappingWeapon(this);
	}
}
// 서버에서만 호출됨
void AWeapon::OnSphereEndOverlap(class UPrimitiveComponent* OverlappedComponent, class AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AFPSCharacter* TempCharacter = Cast<AFPSCharacter>(OtherActor);

	if (TempCharacter)
	{
		TempCharacter->SetOverlappingWeapon(nullptr);
	}
}

void AWeapon::ShowPickUpWidget(bool show)
{
	if (PickUpWidget != nullptr)
	{
		PickUpWidget->SetVisibility(show);
	}
}

// 총 상태 설정
void AWeapon::SetWeaponState(EWeaponState State)
{
	WeaponState = State;

	switch (WeaponState)
	{
		// 총이 떨어져 있을시(충돌 키고, 물리 적용)
	case EWeaponState::WS_Dropped:
		if (HasAuthority())
		{
			AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		}

		WeaponMesh->SetRenderCustomDepth(true);
		WeaponMesh->SetSimulatePhysics(true);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		break;

		// 총이 장착중일시(충돌 끄고, 물리 끄기)
	case EWeaponState::WS_Equipped:
		ShowPickUpWidget(false);

		WeaponMesh->SetRenderCustomDepth(false);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		WeaponMesh->SetSimulatePhysics(false);
		WeaponMesh->SetEnableGravity(false);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	}
}
void AWeapon::OnRep_SetWeaponState()
{
	switch (WeaponState)
	{
	case EWeaponState::WS_Dropped:
		WeaponMesh->SetRenderCustomDepth(true);
		WeaponMesh->SetSimulatePhysics(true);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		break;

	case EWeaponState::WS_Equipped:
		ShowPickUpWidget(false);

		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		WeaponMesh->SetRenderCustomDepth(false);
		WeaponMesh->SetSimulatePhysics(false);
		WeaponMesh->SetEnableGravity(false);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	}
}

// 서버에서만 호출됨
void AWeapon::DropWeapon()
{
	SetWeaponState(EWeaponState::WS_Dropped);

	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	WeaponMesh->DetachFromComponent(DetachRules);

	SetOwner(nullptr);
}

// Multicast에 의해 호출되는 함수
void AWeapon::Fire(const FVector& HitTarget)
{
	if (FireAnimation)
	{
		WeaponMesh->PlayAnimation(FireAnimation, false);
	}
	
	SpendAmmo();
}

// 구 안에서 랜덤한 위치 벡터를 반환
//                                                총구 위치 벡터,          타겟 위치 벡터
FVector AWeapon::TraceEndWidthScatter(const FVector& HitTarget)
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket) return FVector();

	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();

	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal(); // 타겟 방향 (유닛 벡터)
	const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;  // 원의 중심
	const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius); // 원 범위 내에서 랜덤 위치(유닛 벡터)
	const FVector EndLoc = SphereCenter + RandVec; // 원 내부의 어느 위치
	const FVector ToEndLoc = (EndLoc - TraceStart); // 원 내부의 어느 위치의 방향 벡터

	return FVector(TraceStart + ToEndLoc * 8000.f / ToEndLoc.Size());
}

// 탄창이 꽉찬지
bool AWeapon::IsFull()
{
	return Ammo == MagCapacity;
}

// 총알 추가
void AWeapon::AddAmmo(int32 ReloadAmount)
{
	Ammo = FMath::Clamp(Ammo + ReloadAmount, 0, MagCapacity);
	SetHUDAmmo();

	ClientAddAmmo(ReloadAmount);
}
void AWeapon::ClientAddAmmo_Implementation(int32 AmmoToAdd)
{
	if (HasAuthority()) return;

	Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, MagCapacity);

	Character = Character == nullptr ? Cast<AFPSCharacter>(GetOwner()) : Character;
	if (Character && Character->GetCombatComponent() && IsFull())
	{
		Character->GetCombatComponent()->JumpToShotgfunEnd();
	}

	SetHUDAmmo();
}
// 총알 소비
void AWeapon::SpendAmmo()
{
	// 총알 HUD를 업데이트
	Ammo = FMath::Clamp(Ammo - 1, 0, MagCapacity);
	SetHUDAmmo();

	// 서버라면 클라이언트로 업데이트 지시
	if (HasAuthority())
	{
		ClientUpdateAmmo(Ammo);
	}
	else
	{
		// Ammo 에 대해 처리되지 않은 서버 요청 수 증가
		Sequence++;
	}
}
void AWeapon::ClientUpdateAmmo_Implementation(int32 ServerAmmo)
{
	if (HasAuthority()) return;

	Ammo = ServerAmmo;
	Sequence--;
	Ammo -= Sequence;
	SetHUDAmmo();
}

void AWeapon::SetHUDAmmo()
{
	Character = Character == nullptr ? Cast<AFPSCharacter>(GetOwner()) : Character;
	if (Character)
	{
		CharacterController = CharacterController == nullptr ? Cast<ACharacterController>(Character->Controller) : CharacterController;

		// 클라이언트에서만 수행된다
		if (CharacterController && CharacterController->IsLocalController())
		{
			CharacterController->SetHUDAmmo(Ammo);
		}
	}
}

void AWeapon::OnRep_Ammo()
{
	SetHUDAmmo();
}
void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();

	if (Owner)
	{
		SetHUDAmmo();
	}
	else
	{
		Character = nullptr;
		CharacterController = nullptr;
	}
}
