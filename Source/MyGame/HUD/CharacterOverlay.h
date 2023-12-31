// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterOverlay.generated.h"

/**
 * 
 */
UCLASS()
class MYGAME_API UCharacterOverlay : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* HealthText = nullptr;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* SteminaAmount = nullptr;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* DashAmount = nullptr;


	UPROPERTY(meta = (BindWidget))
	class UProgressBar* ThrowCoolTimeBar = nullptr;


	UPROPERTY(meta = (BindWidget))
	class UProgressBar* HealthBar = nullptr;
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* SteminaBar = nullptr;
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* DashBar = nullptr;


	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ScoreAmount = nullptr;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* DefeatAmount = nullptr;


	UPROPERTY(meta = (BindWidget))
	class UTextBlock* AmmoAmount = nullptr;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* CarriedAmmoAmount = nullptr;


	UPROPERTY(meta = (BindWidget))
	class UTextBlock* WeaponType = nullptr;


	UPROPERTY(meta = (BindWidget))
	class UTextBlock* MatchTime = nullptr;


	UPROPERTY(meta = (BindWidget))
	class UTextBlock* PingText = nullptr;
	UPROPERTY(meta = (BindWidget))
	class UImage* HighPingImage = nullptr;
	UPROPERTY(meta = (BindWidgetAnim), Transient) // Transient : 디스크에 직렬화 되지 않음
	class UWidgetAnimation* HighPingAnimation = nullptr;
};
