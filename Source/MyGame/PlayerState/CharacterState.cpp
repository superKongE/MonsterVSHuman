// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterState.h"
#include "MyGame/Character/FPSCharacter.h"
#include "MyGame/Character/FPSMonster.h"
#include "MyGame/PlayerController/CharacterController.h"
#include "MyGame/HumanGameInstance.h"
#include "Net/UnrealNetwork.h"

ACharacterState::ACharacterState()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}
void ACharacterState::BeginPlay()
{
	Super::BeginPlay();
}
void ACharacterState::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
void ACharacterState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACharacterState, Defeat);
	DOREPLIFETIME(ACharacterState, CharacterSkin);
	DOREPLIFETIME(ACharacterState, UserName);
}
void ACharacterState::Server_SetUserName_Implementation(const FString& name)
{
	UserName = name;
}

// 서버에서만 불림
// 점수(킬,데스) 관리
void ACharacterState::AddToScore(float ScoreAmount)
{
	SetScore(GetScore() + ScoreAmount);

	if (GetLocalRole() != ENetRole::ROLE_Authority) return;

	Character = Character == nullptr ? Cast<AFPSCharacter>(GetPawn()) : Character;

	if (Character)
	{
		PlayerController = PlayerController == nullptr ? Cast<ACharacterController>(Character->Controller) : PlayerController;

		if (PlayerController) 
		{
			PlayerController->SetHUDScore(GetScore());
		}
	}
	else
	{
		Character = Cast<AFPSMonster>(GetPawn());
		if (Character == nullptr) return;

		PlayerController = PlayerController == nullptr ? Cast<ACharacterController>(Character->Controller) : PlayerController;

		if (PlayerController)
		{
			PlayerController->SetHUDScore(GetScore());
		}
	}
}
void ACharacterState::AddToDefeat(int32 DefeatAmount)
{
	Defeat += DefeatAmount;

	if (GetLocalRole() != ENetRole::ROLE_Authority) return;

	Character = Character == nullptr ? Cast<AFPSCharacter>(GetPawn()) : Character;

	if (Character)
	{
		PlayerController = PlayerController == nullptr ? Cast<ACharacterController>(Character->Controller) : PlayerController;

		if (PlayerController)
		{
			PlayerController->SetHUDDefeat(Defeat);
		}
	}
	else
	{
		Character = Cast<AFPSMonster>(GetPawn());
		if (Character == nullptr) return;

		PlayerController = PlayerController == nullptr ? Cast<ACharacterController>(Character->Controller) : PlayerController;

		if (PlayerController)
		{
			PlayerController->SetHUDScore(GetScore());
		}
	}
}


void ACharacterState::OnRep_Score()
{
	Super::OnRep_Score();

	Character = Character == nullptr ? Cast<AFPSCharacter>(GetPawn()) : Character;

	if (Character)
	{
		PlayerController = PlayerController == nullptr ? Cast<ACharacterController>(Character->Controller) : PlayerController;

		if (PlayerController)
		{
			PlayerController->SetHUDScore(GetScore());
		}
	}
	else
	{
		Character = Cast<AFPSMonster>(GetPawn());
		if (Character == nullptr) return;

		PlayerController = PlayerController == nullptr ? Cast<ACharacterController>(Character->Controller) : PlayerController;

		if (PlayerController)
		{
			PlayerController->SetHUDScore(GetScore());
		}
	}
}
void ACharacterState::OnRep_Defeat()
{
	Character = Character == nullptr ? Cast<AFPSCharacter>(GetPawn()) : Character;

	if (Character)
	{
		PlayerController = PlayerController == nullptr ? Cast<ACharacterController>(Character->Controller) : PlayerController;

		if (PlayerController)
		{
			PlayerController->SetHUDDefeat(Defeat);
		}
	}
	else
	{
		Character = Cast<AFPSMonster>(GetPawn());
		if (Character == nullptr) return;

		PlayerController = PlayerController == nullptr ? Cast<ACharacterController>(Character->Controller) : PlayerController;

		if (PlayerController)
		{
			PlayerController->SetHUDScore(GetScore());
		}
	}
}