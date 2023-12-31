// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyGame/Weapon.h"
#include "RocketWeapon.generated.h"

UCLASS()
class MYGAME_API ARocketWeapon : public AWeapon
{
	GENERATED_BODY()
	
public:	
	virtual void Fire(const FVector& HitTarget) override;

private:
	UPROPERTY(EditAnywhere)
	TSubclassOf<class AActor> ProjectileClass;
};
