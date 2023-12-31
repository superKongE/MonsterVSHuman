// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyPawn.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/Button.h"
#include "Components/WidgetComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "MyGame/PlayerController/CharacterController.h"
#include "MyGame/PlayerState/CharacterState.h"
#include "MyGame/Lobby/LobbyHUD.h"
#include "MyGame/HUD/LobbyMenu.h"
#include "MyGame/Lobby/LobbyActor.h"
#include "MyGame/Lobby/LobbyMonster.h"
#include "MyGame/HumanGameInstance.h"
#include "MyGame/GameMode/MainGameMode.h"
#include "MyGame/HUD/LobbyPawnNameWidget.h"
#include "Components/BoxComponent.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "Components/EditableText.h"
#include "Kismet/GameplayStatics.h"

ALobbyPawn::ALobbyPawn()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	StaticMesh1 = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("StaticMesh1"));
	StaticMesh2 = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("StaticMesh2"));
	StaticMesh3 = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("StaticMesh3"));
	StaticMesh4 = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("StaticMesh4"));

	StaticMesh1->SetupAttachment(RootComponent);
	StaticMesh2->SetupAttachment(StaticMesh1);
	StaticMesh3->SetupAttachment(StaticMesh2);
	StaticMesh4->SetupAttachment(StaticMesh3);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(StaticMesh1);

	NameWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	NameWidget->SetupAttachment(StaticMesh1);

	StaticMesh1->SetVisibility(true);
	StaticMesh2->SetVisibility(false);
	StaticMesh3->SetVisibility(false);
	StaticMesh4->SetVisibility(false);

	StaticMeshArray.Add(StaticMesh1);
	StaticMeshArray.Add(StaticMesh2);
	StaticMeshArray.Add(StaticMesh3);
	StaticMeshArray.Add(StaticMesh4);
}

void ALobbyPawn::BeginPlay()
{
	Super::BeginPlay();

	UWorld* World = GetWorld();
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;

	FVector StartLocation = GetActorLocation();
	StartLocation.Z = -2417.6f;

	LobbyPawnCamera = World->SpawnActor<ALobbyActor>(LobyActorClass, FVector(-12555.f, 22130.f, -2195.f), FRotator(-15.f, -90.f, 0.f), SpawnParams);
	SetActorRotation(FRotator(0.f, -15.f, 0.f));
	SetActorLocation(StartLocation);

	HumanGameInstance = HumanGameInstance == nullptr ? Cast<UHumanGameInstance>(GetGameInstance()) : HumanGameInstance;
	PlayerController = PlayerController == nullptr ? Cast<ACharacterController>(Controller) : PlayerController;

	if (IsLocallyControlled())
	{
		Server_SetName(HumanGameInstance->GetUserName());
	}
}
void ALobbyPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALobbyPawn, CurrentIndex);
	DOREPLIFETIME(ALobbyPawn, Name);
}

void ALobbyPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	PlayerController = PlayerController == nullptr ? Cast<ACharacterController>(Controller) : PlayerController;
	if (PlayerController)
	{
		PlayerController->SetViewTarget(LobbyPawnCamera);
	}

	if (Name != "" && !namechanged)
	{
		ULobbyPawnNameWidget* NameTags = Cast<ULobbyPawnNameWidget>(NameWidget->GetUserWidgetObject());
		if (NameTags && NameTags->NameText)
		{
			namechanged = true;
			NameTags->NameText->SetText(FText::FromString(Name));
		}
	}
}

void ALobbyPawn::Server_SetName_Implementation(const FString& NameText)
{
	Name = NameText;
}


void ALobbyPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ALobbyPawn::LeftClick);
	PlayerInputComponent->BindAction("Aimming", IE_Pressed, this, &ALobbyPawn::RightClick);
}

void ALobbyPawn::LeftClick()
{
	PlayerController = PlayerController == nullptr ? Cast<ACharacterController>(Controller) : PlayerController;
	if (PlayerController)
	{
		PlayerController->LeftClick();
	}
}
void ALobbyPawn::RightClick()
{
	PlayerController = PlayerController == nullptr ? Cast<ACharacterController>(Controller) : PlayerController;
	if (PlayerController)
	{
		PlayerController->LeftClick();
	}
}


// 캐릭터 스킨 바꾸기
void ALobbyPawn::ChangeRightMesh()
{
	ServerChangeRight();
}
void ALobbyPawn::ChangeLeftMesh()
{
	ServerChangeLeft();
}
void ALobbyPawn::ServerChangeRight_Implementation()
{
	CurrentIndex = (CurrentIndex + 1) % 4;
	if (CurrentIndex == 0)
	{
		StaticMeshArray[3]->SetVisibility(false);
	}
	else
	{
		StaticMeshArray[CurrentIndex - 1]->SetVisibility(false);
	}

	StaticMeshArray[CurrentIndex]->SetVisibility(true);
}
void ALobbyPawn::ServerChangeLeft_Implementation()
{
	if (CurrentIndex - 1 == -1)
	{
		StaticMeshArray[0]->SetVisibility(false);
		CurrentIndex = 3;
	}
	else
	{
		StaticMeshArray[CurrentIndex]->SetVisibility(false);
		CurrentIndex--;
	}

	StaticMeshArray[CurrentIndex]->SetVisibility(true);
}
// CurrentIndex가 변하면 호출되는 함수
void ALobbyPawn::OnRep_ChangeMesh()
{
	if (CurrentIndex - 1 == -1)
	{
		StaticMeshArray[3]->SetVisibility(false);
	}
	else
	{
		StaticMeshArray[CurrentIndex - 1]->SetVisibility(false);
	}

	StaticMeshArray[CurrentIndex]->SetVisibility(true);

	if (CurrentIndex + 1 == 4)
	{
		StaticMeshArray[0]->SetVisibility(false);
	}
	else
	{
		StaticMeshArray[CurrentIndex + 1]->SetVisibility(false);
	}

	if (IsLocallyControlled())
	{
		HumanGameInstance = HumanGameInstance == nullptr ? Cast<UHumanGameInstance>(GetGameInstance()) : HumanGameInstance;
		if (HumanGameInstance)
		{
			HumanGameInstance->SetCharacterSkin(CurrentIndex);
		}
	}
}


void ALobbyPawn::ServerLeftGame_Implementation()
{
	ClientLeftGame();
	//OnLeftGame.Broadcast();
}
void ALobbyPawn::ClientLeftGame_Implementation()
{
	OnLeftGame.Broadcast();
}