// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ChatBox.generated.h"

/**
 * 
 */
UCLASS()
class MYGAME_API UChatBox : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta = (BindWidget))
	class UEditableText* WriteMessage = nullptr;

	UPROPERTY(meta = (BindWidget))
	class UScrollBox* ScrollBox = nullptr;

	UPROPERTY(meta = (BindWidget))
	class UButton* TeamButton = nullptr;
};
