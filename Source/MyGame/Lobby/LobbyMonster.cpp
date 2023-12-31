// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyMonster.h"
#include "Camera/CameraComponent.h"
#include "MyGame/PlayerController/CharacterController.h"

ALobbyMonster::ALobbyMonster()
{
	PrimaryActorTick.bCanEverTick = false;
	NetUpdateFrequency = 2.f;
	bReplicates = false;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
}
void ALobbyMonster::BeginPlay()
{
	Super::BeginPlay();
}
void ALobbyMonster::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
void ALobbyMonster::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ALobbyMonster::LeftClick);
	PlayerInputComponent->BindAction("Aimming", IE_Pressed, this, &ALobbyMonster::RightClick);
}

void ALobbyMonster::LeftClick()
{
	PlayerController = PlayerController == nullptr ? Cast<ACharacterController>(Controller) : PlayerController;
	if (PlayerController)
	{
		PlayerController->LeftClick();
	}
}
void ALobbyMonster::RightClick()
{
	PlayerController = PlayerController == nullptr ? Cast<ACharacterController>(Controller) : PlayerController;
	if (PlayerController)
	{
		PlayerController->LeftClick();
	}
}

// 나가기 요청했을때 호출되는 함수
void ALobbyMonster::ServerLeftGame_Implementation()
{
	OnLeftGame.Broadcast();
}