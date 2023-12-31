// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "Rocket.generated.h"

/**
 * 
 */
UCLASS()
class MYGAME_API ARocket : public AProjectile
{
	GENERATED_BODY()

public:
	ARocket();

	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

private:
	UPROPERTY(EditAnywhere)
	float DamageInnerRadius = 200.f;

	UPROPERTY(EditAnywhere)
	float DamageOuterRadius = 500.f;

	UPROPERTY(EditAnywhere)
	float DamageFallOff = 100.f;

	UPROPERTY(EditAnywhere)
	class URocketMovementComponent* RocketMovementComponent = nullptr;
};
