// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "CharacterState.generated.h"

/**
 * 
 */
UCLASS()
class MYGAME_API ACharacterState : public APlayerState
{
	GENERATED_BODY()

protected:
	ACharacterState();
	virtual void Tick(float DeltaTime) override;
	virtual void BeginPlay() override;
	virtual void OnRep_Score() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	// 내장된 Score 변수가 float 타입이으로 매개변수 타입도 float
	void AddToScore(float ScoreAmount);
	void AddToDefeat(int32 DefeatAmount);
	
	FORCEINLINE const int32 GetCharacterSkin() const { return CharacterSkin; }
	FORCEINLINE void SetCharacterSkin(int32 Skin) { CharacterSkin = Skin; UE_LOG(LogTemp, Warning, TEXT("%d"), CharacterSkin); }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE FString GetUserName() const { return UserName; }
	FORCEINLINE void SetUserName(const FString& name) { UserName = name; }
	FORCEINLINE bool GetAmIMonster() { return AmIMonster; }
	FORCEINLINE void SetAmIMonster(bool IsMonster) { AmIMonster = IsMonster; }

	UFUNCTION(Server, Reliable)
	void Server_SetUserName(const FString& name);

	UFUNCTION()
	void OnRep_Defeat();

private:
	UPROPERTY()
	class ACharacter* Character = nullptr;
	UPROPERTY()
	class ACharacterController* PlayerController = nullptr;
	UPROPERTY()
	class UHumanGameInstance* HumanGameInstance = nullptr;

	UPROPERTY(ReplicatedUsing = OnRep_Defeat)
	int32 Defeat = 0;

	UPROPERTY(Replicated)
	int32 CharacterSkin = 0;

	UPROPERTY(Replicated)
	FString UserName = "";

	bool AmIMonster;
};
