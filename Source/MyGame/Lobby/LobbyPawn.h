// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "LobbyPawn.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLeftGameLobbyHuman);

UCLASS()
class MYGAME_API ALobbyPawn : public APawn
{
	GENERATED_BODY()

public:
	ALobbyPawn();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void LeftClick();
	void RightClick();

	UFUNCTION(BlueprintCallable)
	void ChangeRightMesh();
	UFUNCTION(BlueprintCallable)
	void ChangeLeftMesh();
	UFUNCTION(Server, Reliable)
	void ServerChangeRight();
	UFUNCTION(Server, Reliable)
	void ServerChangeLeft();
	UFUNCTION(BlueprintCallable)
	FString GetHumanName() { return Name; }

	UFUNCTION(Server, Reliable)
	void ServerLeftGame();

	UFUNCTION(Client, Reliable)
	void ClientLeftGame();


	UFUNCTION()
	void OnRep_ChangeMesh();

	UFUNCTION(Server, Reliable)
	void Server_SetName(const FString& NameText);


	void newHumanLogined();

	FORCEINLINE bool GetIsMonster() const { return IsMonster; }


	FOnLeftGameLobbyHuman OnLeftGame;

private:
	UPROPERTY(EditAnywhere)
	TSubclassOf<AActor> LobyActorClass;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UUserWidget> NameTagClass;

	UPROPERTY()
	class AMyGameState* MyGameState = nullptr;

	UPROPERTY()
	class ULobbyPawnNameWidget* NameTag = nullptr;

	UPROPERTY()
	class UHumanGameInstance* HumanGameInstance = nullptr;

	UPROPERTY()
	class ACharacterController* PlayerController = nullptr;

	UPROPERTY()
	class ACharacterState* CharacterState = nullptr;

	UPROPERTY(EditAnywhere)
	class UCameraComponent* Camera = nullptr;

	UPROPERTY()
	class ALobbyActor* LobbyPawnCamera = nullptr;

	UPROPERTY()
	class ALobbyHUD* LobbyHUD = nullptr;

	UPROPERTY(VisibleAnywhere)
	class UWidgetComponent* NameWidget = nullptr;

	UPROPERTY(EditAnywhere)
	class USkeletalMeshComponent* StaticMesh1 = nullptr;

	UPROPERTY(EditAnywhere)
	class USkeletalMeshComponent* StaticMesh2 = nullptr;

	UPROPERTY(EditAnywhere)
	class USkeletalMeshComponent* StaticMesh3 = nullptr;

	UPROPERTY(EditAnywhere)
	class USkeletalMeshComponent* StaticMesh4 = nullptr;

	TArray<class USkeletalMeshComponent*> StaticMeshArray;

	bool IsMonster = false;
	bool namechanged = false;

	UPROPERTY(ReplicatedUsing = OnRep_ChangeMesh)
	int32 CurrentIndex = 0;

	UPROPERTY(Replicated)
	FString Name = "";
};
