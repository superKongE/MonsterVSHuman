// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

UCLASS()
class MYGAME_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AProjectile();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	float Damage = 20.f;
	UPROPERTY(EditAnywhere)
	float HeadShotDamage = 100.f;

public:	
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()//            때리는 것(총알, 칼 등),   맞은 액터,           맞은 컴포넌트
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION(NetMulticast, Reliable)
	void MultiCast_Hit(bool bHitCharacter, bool bHitMonster);

	UPROPERTY(EditAnywhere)
	class UProjectileMovementComponent* ProjectileMovementComponent = nullptr;

private:

	UPROPERTY(EditAnywhere)
	class UBoxComponent* CollisionArea = nullptr;

	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* ProjectileMesh = nullptr;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* Tracer = nullptr;

	UPROPERTY(EditAnywhere)
	class UParticleSystemComponent* TraceParticleComponent = nullptr;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* HitCharacterParticle = nullptr;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* HitMonsterParticle = nullptr;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* HitSomethingParticle = nullptr;

	UPROPERTY(EditAnywhere)
	class USoundCue* ImpactSound = nullptr;

	bool IsHitCharacter = false;
	bool IsHitMonster = false;
};
