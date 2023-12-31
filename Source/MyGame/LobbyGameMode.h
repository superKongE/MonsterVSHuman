// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "LobbyGameMode.generated.h"

/**
 * 
 */
UCLASS()
class MYGAME_API ALobbyGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual void GetSeamlessTravelActorList(bool bToEntry, TArray<class AActor*>& ActorList);

	UFUNCTION(BlueprintCallable)
	void SetReadyPlayerCount(bool bReady);

	UFUNCTION(BlueprintCallable)
	void StartGameBeforeDestoryLobbyHUD();

	void DestroyLobbyHUDCompleted();

	void UpdateSession();

	UPROPERTY(BlueprintReadWrite)
	bool CanStartGame = false;

	UFUNCTION(BlueprintCallable)
	FORCEINLINE int32 GetReadyPlayerCount() {return ReadyPlayerCount;}

	IOnlineSessionPtr SessionInterface;

private:
	UPROPERTY()
	class UHumanGameInstance* ServerGameInstance = nullptr;

	int32 PlayerCount = 0;
	int32 ReadyPlayerCount = 1; // 방장은 준비된 상태기 때문에 1
	int32 DestroyLobyHUDCount = 0;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<APawn> SpawnCharacter;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<APawn> SpawnMosnter;

	TArray<APlayerController*> PlayerInLobby;
};
