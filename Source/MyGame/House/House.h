// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "House.generated.h"

UCLASS()
class MYGAME_API AHouse : public AActor
{
	GENERATED_BODY()
	
public:	
	AHouse();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	virtual void SphereAreaBeginOverlap(class UPrimitiveComponent* OverlappedComponent, class AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void SphereAreaEndOverlap(class UPrimitiveComponent* OverlappedComponent, class AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void Recovery();

private:
	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* HouseMesh = nullptr;

	UPROPERTY()
	class AMainGameMode* MainGameMode = nullptr;

	UPROPERTY(EditAnywhere)
	class USphereComponent* SphereArea = nullptr;

	FTimerHandle RecoveryHealthTimer;
	TDoubleLinkedList<class AFPSCharacter*> RecoveryHumanList;

private:
	UPROPERTY(EditAnywhere, Category = Recovery)
	float RecoveryHealth = 5.f;

	UPROPERTY(EditAnywhere, Category = Recovery)
	float RecoveryHealthDelay = 5.f;
};
