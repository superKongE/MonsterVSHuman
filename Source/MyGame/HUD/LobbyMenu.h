// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyMenu.generated.h"

/**
 * 
 */
UCLASS()
class MYGAME_API ULobbyMenu : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	class UButton* LeftButton = nullptr;

	UPROPERTY(meta = (BindWidget))
	class UButton* RightButton = nullptr;

	UPROPERTY(meta = (BindWidget))
	class UButton* ReadyButton = nullptr;

	UPROPERTY(meta = (BindWidget))
	class UButton* StartButton = nullptr;

	UPROPERTY(meta = (BindWidget))
	class UScrollBox* ScrollBox = nullptr;

	UPROPERTY(meta = (BindWidget))
	class UEditableText* WriteMessage = nullptr;
};
