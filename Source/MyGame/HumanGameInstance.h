// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "HumanGameInstance.generated.h"

UCLASS()
class MYGAME_API UHumanGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	FORCEINLINE int32 GetCharacterSkin() const { return CharacterSkin; }
	FORCEINLINE bool GetAmIMonster() const { return AmIMonster; }
	FORCEINLINE void SetCharacterSkin(int32 Skin) { CharacterSkin = Skin; }
	FORCEINLINE void SetAmIMonster(bool IsMonster) { AmIMonster = IsMonster; }
	FORCEINLINE void SetPlayerNumber(int num) { PlayerNumber = num; }
	FORCEINLINE int32 GetPlayerNumber() const { return PlayerNumber; }
	FORCEINLINE void SetMaxUserCount(int32 n) { MaxUserCount += n; }
	FORCEINLINE int32 GetMaxUserCount() const { return MaxUserCount; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE FString GetUserName() const { return UserName; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetUserName(FString Name) { UserName = Name; }

private:
	int32 CharacterSkin = 0;
	int32 PlayerNumber = 0;
	int32 MaxUserCount = 0;

	bool AmIMonster = false;

	FString UserName = "nAmE";
};
