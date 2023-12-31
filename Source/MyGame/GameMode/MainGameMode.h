// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "MainGameMode.generated.h"

namespace MatchState
{
	extern MYGAME_API const FName GameStart;
	extern MYGAME_API const FName MonsterWin;
	extern MYGAME_API const FName HumanWin;
	extern MYGAME_API const FName Night;
	extern MYGAME_API const FName Morning;
}

UENUM(BlueprintType)
enum class EMatchState : uint8 {
	EMS_Morning UMETA(DisplayName = "Morning"),
	EMS_Night UMETA(DisplayName = "Night"),
};

UCLASS()
class MYGAME_API AMainGameMode : public AGameMode
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void GenericPlayerInitialization(AController* Controller) override;
	virtual void Logout(AController* Exiting) override;

public:
	AMainGameMode();
	virtual void Tick(float DeltaTime) override;
	virtual void OnMatchStateSet() override;
	void PlayerEliminated(class AFPSCharacter* ElimedCharacter, class ACharacterController* VictimController, class ACharacterController* AttackerController);
	void PlayerEliminated(class AFPSMonster* ElimedMonster, class ACharacterController* VictimController, class ACharacterController* AttackerController);

	void RequestRespone(class ACharacter* RequestCharacter, class AController* RequestController, bool IsMonster);

	void AddUser();
	void PlayerLeftGame(class AFPSCharacter* Character);
	void PlayerLeftGame(class AFPSMonster* Monster);

	void CompleteCharge();

	void SpawnMonsterPlayer(class ACharacterController* PlayerController);
	void SpawnHumanPlayer(class ACharacterController* PlayerController);

	void GameFinished();

	bool IsAlreadyGameFinished();

	/*UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.f;

	UPROPERTY(EditDefaultsOnly, Category = Time)
	float CooldownTime = 10.f;*/

	UPROPERTY(EditAnywhere, Category = Time)
	float NightChangeTime = 10.f;
	float LevelStartingTime = 0.f;

private:
	float ServerTime = 0.f;
	UPROPERTY(EditAnywhere)
	float Count = 1.f;
	UPROPERTY(EditAnywhere, Category = Generator)
	int32 ChargedGeneratorCount = 4;

	UPROPERTY(EditAnywhere)
	FVector MonsterSpawnLocation = FVector(-19672.8f, 33473.5f, 1506.8f);
	UPROPERTY(EditAnywhere)
	FRotator MonsterSpawnRotation = FRotator(0.f, 0.f, 0.f);

	//int32 cnt = 0;
	int32 UserCount = 0;
	int32 HumanCount = 0;
	int32 MonsterCount = 0;
	int32 ChargedGenerator = 0;

	EMatchState CurrentMatchState;
	
	UPROPERTY()
	class AMyGameState* MyGameState = nullptr;

	UPROPERTY()
	class ADirectionalLightActor* DirectionalLight = nullptr;

	UPROPERTY();
	class UHumanGameInstance* ServerGameInstance = nullptr;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<ACharacter> SpawnCharacter;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<ACharacter> SpawnMonster;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AActor> DirectionalLightClass;

	UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<APawn> SpectatorHumanClass;

public:
	FORCEINLINE EMatchState GetCurrentMatchState() const { return CurrentMatchState; }
	FORCEINLINE float GetLevelStartingTime() const { return LevelStartingTime; }
	FORCEINLINE float GetServerTime() const { return ServerTime; }
	FORCEINLINE int32 GetHumanCount() const { return HumanCount; }
	FORCEINLINE int32 GetMonsterCount() const { return MonsterCount; }
};
