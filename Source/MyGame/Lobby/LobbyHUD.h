// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "LobbyHUD.generated.h"

/**
 * 
 */
UCLASS()
class MYGAME_API ALobbyHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	FORCEINLINE class ULobbyMenu* GetLobbyMenu() const { return LobbyMenu; }

	//void ShowNameWidget(bool bShow);
	void AddLobbyMenu();
	
public:
	UPROPERTY()
	APlayerController* OwningPlayerController = nullptr;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UUserWidget> LobbyMenuClass;

	//UPROPERTY(EditAnywhere)
	//TSubclassOf<class UUserWidget> NameWidgetClass;

	UPROPERTY()
	class ULobbyMenu* LobbyMenu = nullptr;

	/*UPROPERTY()
	class ULobbyPawnNameWidget* NameWidget = nullptr;*/
};
