
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

// ���������� ȣ���
void AWeapon::OnSphereOverlap(class UPrimitiveComponent* OverlappedComponent, class AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AFPSCharacter* TempCharacter = Cast<AFPSCharacter>(OtherActor);

	if (TempCharacter)
	{
		TempCharacter->SetOverlappingWeapon(this);
	}
}
// ���������� ȣ���
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

// �� ���� ����
void AWeapon::SetWeaponState(EWeaponState State)
{
	WeaponState = State;

	switch (WeaponState)
	{
		// ���� ������ ������(�浹 Ű��, ���� ����)
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

		// ���� �������Ͻ�(�浹 ����, ���� ����)
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

// ���������� ȣ���
void AWeapon::DropWeapon()
{
	SetWeaponState(EWeaponState::WS_Dropped);

	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	WeaponMesh->DetachFromComponent(DetachRules);

	SetOwner(nullptr);
}

// Multicast�� ���� ȣ��Ǵ� �Լ�
void AWeapon::Fire(const FVector& HitTarget)
{
	if (FireAnimation)
	{
		WeaponMesh->PlayAnimation(FireAnimation, false);
	}
	
	SpendAmmo();
}

// �� �ȿ��� ������ ��ġ ���͸� ��ȯ
//                                                �ѱ� ��ġ ����,          Ÿ�� ��ġ ����
FVector AWeapon::TraceEndWidthScatter(const FVector& HitTarget)
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket) return FVector();

	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();

	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal(); // Ÿ�� ���� (���� ����)
	const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;  // ���� �߽�
	const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius); // �� ���� ������ ���� ��ġ(���� ����)
	const FVector EndLoc = SphereCenter + RandVec; // �� ������ ��� ��ġ
	const FVector ToEndLoc = (EndLoc - TraceStart); // �� ������ ��� ��ġ�� ���� ����

	return FVector(TraceStart + ToEndLoc * 8000.f / ToEndLoc.Size());
}

// źâ�� ������
bool AWeapon::IsFull()
{
	return Ammo == MagCapacity;
}

// �Ѿ� �߰�
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
// �Ѿ� �Һ�
void AWeapon::SpendAmmo()
{
	// �Ѿ� HUD�� ������Ʈ
	Ammo = FMath::Clamp(Ammo - 1, 0, MagCapacity);
	SetHUDAmmo();

	// ������� Ŭ���̾�Ʈ�� ������Ʈ ����
	if (HasAuthority())
	{
		ClientUpdateAmmo(Ammo);
	}
	else
	{
		// Ammo �� ���� ó������ ���� ���� ��û �� ����
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

		// Ŭ���̾�Ʈ������ ����ȴ�
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
