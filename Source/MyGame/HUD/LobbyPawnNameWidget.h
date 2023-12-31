// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyPawnNameWidget.generated.h"

/**
 * 
 */
UCLASS()
class MYGAME_API ULobbyPawnNameWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta = (BindWidget))
	class UEditableText* NameText = nullptr;
};
