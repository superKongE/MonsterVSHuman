// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyGame/Weapon.h"
#include "RifleWeapon.generated.h"

/**
 * 
 */
UCLASS()
class MYGAME_API ARifleWeapon : public AWeapon
{
	GENERATED_BODY()
	
public:
	virtual void Fire(const FVector& HitTarget) override;
	 
private:
	UPROPERTY(EditAnywhere)
	TSubclassOf<class AActor> ProjectileClass;
};
