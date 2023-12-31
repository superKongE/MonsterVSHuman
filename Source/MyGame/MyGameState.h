// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "MyGameState.generated.h"

/**
 * 
 */
UCLASS()
class MYGAME_API AMyGameState : public AGameState
{
	GENERATED_BODY()

public:
	void AddHumanArray(class AFPSCharacter* Human);
	void AddMonsterArray(class AFPSMonster* Monster);

	class AFPSCharacter* GetSpectatorNextHumanPlayer();
	class AFPSMonster* GetSpectatorMonsterPlayer();

	FORCEINLINE int32 GetMaxUserCount() const { return MaxUserCount; }
	FORCEINLINE void SetMaxUserCount(int n) { MaxUserCount += n; }
	FORCEINLINE int32 GetSurvivalHumanCount() const { return SurvivalHumanCount; }
	FORCEINLINE void SetSurvivalHumanCount(int32 Count) { SurvivalHumanCount += Count; }
	FORCEINLINE int32 GetSurvivalMonsterCount() const { return SurvivalMonsterCount; }
	FORCEINLINE void SetSurvivalMonstercount(int32 Count) { SurvivalMonsterCount += Count; }
	FORCEINLINE TArray<class AFPSCharacter*> GetHumanArray() const { return HumanArray; }
	FORCEINLINE TArray<class AFPSMonster*> GetMonsterArray() const { return MonsterArray; }

private:
	TArray<class AFPSCharacter*> HumanArray;
	TArray<class AFPSMonster*> MonsterArray;

	int32 Humanindex = 0;
	int32 MonsterIndex = 0;
	int32 SurvivalHumanCount = 0;
	int32 SurvivalMonsterCount = 0;
	int32 MaxUserCount = 0;
};
