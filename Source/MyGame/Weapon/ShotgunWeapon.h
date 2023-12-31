// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HitScanWeapon.h"
#include "ShotgunWeapon.generated.h"

/**
 * 
 */
UCLASS()
class MYGAME_API AShotgunWeapon : public AHitScanWeapon
{
	GENERATED_BODY()
	
public:
	virtual void FireShotgun(const TArray<FVector_NetQuantize>& HitTargets);
	void ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets);
	void ShotgunTraceEndWithScatter_Zoomed(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets);

private:
	UPROPERTY(EditAnywhere)
	uint32 NumberOfPellets = 8;

	UPROPERTY(EditAnywhere)
	float ZoomedSphereRadius = 40.f;

	UPROPERTY(EditAnywhere)
	class USoundCue* ImpactSound = nullptr;
};
