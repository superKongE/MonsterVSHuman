// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "Rifle.generated.h"

/**
 * 
 */
UCLASS()
class MYGAME_API ARifle : public AProjectile
{
	GENERATED_BODY()
	
public:
	ARifle();

	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
};
