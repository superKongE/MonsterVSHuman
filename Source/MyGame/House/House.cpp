#include "MyGame/House/House.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Containers/List.h"

#include "MyGame/Character/FPSCharacter.h"
#include "MyGame/GameMode/MainGameMode.h"

AHouse::AHouse()
{
	PrimaryActorTick.bCanEverTick = false;
	SetReplicates(true);

	HouseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HouseMesh"));
	SetRootComponent(HouseMesh);

	SphereArea = CreateDefaultSubobject<USphereComponent>(TEXT("SphereArea"));
	SphereArea->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereArea->SetCollisionObjectType(ECollisionChannel::ECC_Pawn);
	SphereArea->SetupAttachment(RootComponent);
}
void AHouse::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		SphereArea->OnComponentBeginOverlap.AddDynamic(this, &AHouse::SphereAreaBeginOverlap);
		SphereArea->OnComponentEndOverlap.AddDynamic(this, &AHouse::SphereAreaEndOverlap);
	
		GetWorldTimerManager().SetTimer(
			RecoveryHealthTimer,
			this,
			&AHouse::Recovery,
			RecoveryHealthDelay,
			true
		);
	}

	if (HasAuthority())
	{
		MainGameMode = GetWorld()->GetAuthGameMode<AMainGameMode>();
	}
}
void AHouse::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// �ΰ��� �� ��ó�� ������
void AHouse::SphereAreaBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	AFPSCharacter* Human = Cast<AFPSCharacter>(OtherActor);
	if (Human != nullptr)
	{
		// ��ũ�� ����Ʈ�� �� ��ó�� �ִ� �ΰ� ����
		RecoveryHumanList.AddTail(Human);
	}
}
// �ΰ��� �� ��ó���� �������
void AHouse::SphereAreaEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AFPSCharacter* Human = Cast<AFPSCharacter>(OtherActor);
	if (Human != nullptr)
	{
		RecoveryHumanList.RemoveNode(Human);
	}
}

// �� ȸ�� ��Ű��
void AHouse::Recovery()
{
	MainGameMode = MainGameMode == nullptr ? GetWorld()->GetAuthGameMode<AMainGameMode>() : nullptr;
	if (MainGameMode == nullptr) return;

	// ��ħ�� ��쿡��
	if (MainGameMode->GetCurrentMatchState() == EMatchState::EMS_Morning)
	{
		for (AFPSCharacter* Human : RecoveryHumanList)
		{
			Human->RecoveryHealth(RecoveryHealth);
		}
	}
}


