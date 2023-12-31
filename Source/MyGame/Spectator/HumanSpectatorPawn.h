// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "HumanSpectatorPawn.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLeftGameSpectaor);

UCLASS()
class MYGAME_API AHumanSpectatorPawn : public APawn
{
	GENERATED_BODY()

public:
	AHumanSpectatorPawn();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	//virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void LeftClick();

	UFUNCTION(Server, Reliable)
	void Server_GetNextSpectator();

	void LeftGame();

	FOnLeftGameSpectaor OnLeftGame;

private:
	UPROPERTY()
	class AMyGameState* MyGameState = nullptr;

	UPROPERTY()
	class ACharacterController* PlayerController = nullptr;

	UPROPERTY()
	class AFPSCharacter* SpectatorCharacter = nullptr;

	UPROPERTY()
	class AFPSMonster* SpectatorMonster = nullptr;

	int32 CurrentUserIndex = 0;
};
