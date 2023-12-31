// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyGame/Weapon.h"
#include "HitScanWeapon.generated.h"

/**
 * 
 */
UCLASS()
class MYGAME_API AHitScanWeapon : public AWeapon
{
	GENERATED_BODY()

protected:
	void WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit);

	UPROPERTY(EditAnywhere)
	class UParticleSystem* ImpactHumanParticles = nullptr;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* ImpactMonsterParticles = nullptr;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* ImpactSomethingParticles = nullptr;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* BeamParticles = nullptr;

	
public:
	virtual void Fire(const FVector& HitTarget);
};
