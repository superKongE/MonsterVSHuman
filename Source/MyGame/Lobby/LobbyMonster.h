// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "LobbyMonster.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLeftGameLobbyMonster);

UCLASS()
class MYGAME_API ALobbyMonster : public APawn
{
	GENERATED_BODY()

public:
	ALobbyMonster();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void LeftClick();
	void RightClick();

	UFUNCTION(Server, Reliable)
	void ServerLeftGame();

	FOnLeftGameLobbyMonster OnLeftGame;

private:
	UPROPERTY(EditAnywhere)
	class UCameraComponent* Camera = nullptr;

	UPROPERTY()
	class ACharacterController* PlayerController = nullptr;

	UPROPERTY()
	class UHumanGameInstance* HumanGameInstance = nullptr;
};
