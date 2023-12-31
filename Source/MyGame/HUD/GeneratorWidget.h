// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GeneratorWidget.generated.h"

/**
 * 
 */
UCLASS()
class MYGAME_API UGeneratorWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* GeneratorBar = nullptr;
};
