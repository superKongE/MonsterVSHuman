// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MapUserPoint.generated.h"

/**
 * 
 */
UCLASS()
class MYGAME_API UMapUserPoint : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta = (BindWidget))
	class UImage* UserPoint = nullptr;

	UPROPERTY(EditAnywhere)
	class UTexture2D* OwnerPoint = nullptr;

	UPROPERTY(EditAnywhere)
	class UTexture2D* OtherPoint = nullptr;
};
