// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Dagger.generated.h"

UCLASS()
class MYGAME_API ADagger : public AActor
{
	GENERATED_BODY()
	
public:	
	ADagger();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()//            ������ ��(�Ѿ�, Į ��),   ���� ����,           ���� ������Ʈ
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	UFUNCTION(NetMulticast, Reliable)
	void MultiCast_Hit(bool bHitCharacter);

	FVector BeginLocation;
	FVector CurrentLocation;

private:
	UPROPERTY(EditAnywhere)
	TSubclassOf<class UDamageType> SlowEffectDamageClass;

	UPROPERTY(EditAnywhere)
	class UProjectileMovementComponent* ProjectileMovementComponent = nullptr;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* HitCharacterParticle = nullptr;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* HitSomethingParticle = nullptr;

	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* ThorwKnifeMesh = nullptr;

	UPROPERTY(EditAnywhere)
	class USoundCue* ImpactSound = nullptr;

	UPROPERTY(EditAnywhere)
	class UBoxComponent* CollisionArea = nullptr;

	UPROPERTY(EditAnywhere)
	class UParticleSystemComponent* TraceParticleComponent = nullptr;

	UPROPERTY(EditAnywhere)
	class UParticleSystem* Tracer = nullptr;

	UPROPERTY(EditAnywhere)
	int32 DaggerDamage = 10;

	bool IsHitCharacter;
};
