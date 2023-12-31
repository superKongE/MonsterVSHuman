// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponStore.generated.h"

UCLASS()
class MYGAME_API AWeaponStore : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeaponStore();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	void Open();
};
