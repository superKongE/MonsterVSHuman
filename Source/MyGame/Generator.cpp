// Fill out your copyright notice in the Description page of Project Settings.


#include "Generator.h"
#include "MyGame/Character/FPSCharacter.h"
#include "MyGame/GameMode/MainGameMode.h"
#include "MyGame/HUD/GeneratorWidget.h"
#include "MyGame/PlayerController/CharacterController.h"
#include "MyGame/Weapon/RifleWeapon.h"
#include "MyGame/Character/FPSMonster.h"
#include "MyGame/Weapon.h"
#include "MyGame/Weapon/RocketWeapon.h"
#include "MyGame/Weapon/WeaponType.h"
#include "MyGame/Weapon/ShotgunWeapon.h"
#include "MyGame/MyGameState.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInstance.h"
#include "Components/ProgressBar.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

AGenerator::AGenerator()
{
	PrimaryActorTick.bCanEverTick = true;
	NetUpdateFrequency = 5.f;
	bReplicates = true;

	SphereArea = CreateDefaultSubobject<USphereComponent>(TEXT("SphereArea"));
	RootComponent = SphereArea;

	GeneratorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GeneratorMesh"));
	GeneratorMesh->SetupAttachment(RootComponent);

	GeneratorWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("GeneratorWidget"));
	GeneratorWidget->SetupAttachment(GeneratorMesh);
}
void AGenerator::BeginPlay()
{
	Super::BeginPlay();

	DynamicDissolveMaterialInstance1 = UMaterialInstanceDynamic::Create(DissolveMaterialInstance1, this);

	GeneratorMesh->SetMaterial(0, DynamicDissolveMaterialInstance1);

	DynamicDissolveMaterialInstance1->SetScalarParameterValue(TEXT("Power"), 0.55f);

	if (GetLocalRole() == ENetRole::ROLE_Authority)
	{
		SphereArea->OnComponentBeginOverlap.AddDynamic(this, &AGenerator::OnSphereOverlap);
		SphereArea->OnComponentEndOverlap.AddDynamic(this, &AGenerator::OnSphereEndOverlap);
	}
	if (GeneratorWidget)
	{
		GeneratorWidget->SetVisibility(false);
	}
}
void AGenerator::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGenerator, CurrentGernerateAmount);
	//DOREPLIFETIME(AGenerator, ChargeUserCount);
	DOREPLIFETIME(AGenerator, ChargeFinish);
}
void AGenerator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (HasAuthority() && CurrentGernerateAmount >= GenerateMaxAmount && !ChargeFinish)
	{
		ChargeFinish = true;
		FullCharge();
	}
}

// �ΰ��� ������ ��ó�� ����
void AGenerator::OnSphereOverlap(class UPrimitiveComponent* OverlappedComponent, class AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	AFPSCharacter* TempCharacter = Cast<AFPSCharacter>(OtherActor);
	if (TempCharacter)
	{
		TempCharacter->OverlappingGenerator(this);
	}
}
// �ΰ��� ������ ��ó���� �����
void AGenerator::OnSphereEndOverlap(class UPrimitiveComponent* OverlappedComponent, class AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AFPSCharacter* TempCharacter = Cast<AFPSCharacter>(OtherActor);
	if (TempCharacter)
	{
		TempCharacter->OverlappingGenerator(nullptr);
	}
}
// ������ ������ UI ǥ��
void AGenerator::ShowGenerateWidget(bool bShow)
{
	if (GeneratorWidget != nullptr)
	{
		GeneratorWidget->SetVisibility(bShow);
	}
}

// ������ ����
bool AGenerator::AddChargingUser()
{
	if (CurrentGernerateAmount <= GenerateMaxAmount)
	{
		CurrentGernerateAmount += ChargeAmount;
		return true;
	}

	return false;
}


// ���� �Ϸ�� (Tick���� ȣ��ȴ�)(����)
void AGenerator::FullCharge()
{
	// �������� ������ ������ �Ϸ�Ǿ��ٰ� �˷���
	GetWorld()->GetAuthGameMode<AMainGameMode>()->CompleteCharge();

	SphereArea->OnComponentBeginOverlap.RemoveDynamic(this, &AGenerator::OnSphereOverlap);
	SphereArea->OnComponentEndOverlap.RemoveDynamic(this, &AGenerator::OnSphereEndOverlap);

	MulticastFullCharge();

	UWorld* World = GetWorld();
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;

	FVector SpawnLocation = GetActorLocation() + FVector(0.f, 0.f, 200.f);
	if (World)
	{
		switch (WeaponType)
		{
		case EWeaponType::ET_Rifle:
			if(WeaponClass)
				World->SpawnActor<ARifleWeapon>(WeaponClass, SpawnLocation, GetActorRotation(), SpawnParams);
			break;

		case EWeaponType::ET_Rocket:
			if (WeaponClass)
				World->SpawnActor<ARocketWeapon>(WeaponClass, SpawnLocation, GetActorRotation(), SpawnParams);
			break;

		case EWeaponType::ET_ShotGun:
			if (WeaponClass)
				World->SpawnActor<AShotgunWeapon>(WeaponClass, SpawnLocation, GetActorRotation(), SpawnParams);
			break;
		}
	}
}
// �����Ϸ�Ǹ� ���� ����
void AGenerator::MulticastFullCharge_Implementation()
{
	DynamicDissolveMaterialInstance1->SetScalarParameterValue(TEXT("Power"), -0.55f);
}
