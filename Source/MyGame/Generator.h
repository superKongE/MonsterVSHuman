// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyGame/Weapon/WeaponType.h"
#include "Generator.generated.h"

UCLASS()
class MYGAME_API AGenerator : public AActor
{
	GENERATED_BODY()
	
public:	
	AGenerator();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	virtual void OnSphereOverlap(class UPrimitiveComponent* OverlappedComponent, class AActor* OtherActor,
			UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
			bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void OnSphereEndOverlap(class UPrimitiveComponent* OverlappedComponent, class AActor* OtherActor,
			UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

public:	
	virtual void Tick(float DeltaTime) override;

public:
	FORCEINLINE float GetGenerateAmount() const { return CurrentGernerateAmount; }

	void ShowGenerateWidget(bool bShow);

	void FullCharge();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFullCharge();

	bool AddChargingUser();

	UFUNCTION(BlueprintCallable)
	FORCEINLINE float GetCurrentGenerateAmount() const { return CurrentGernerateAmount; }

	//UFUNCTION()
	//void OnRep_CurrentGenerateAmount();

private:
	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* GeneratorMesh = nullptr;

	UPROPERTY(EditAnywhere)
	class USphereComponent* SphereArea = nullptr;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UUserWidget> GeneratorClass;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AWeapon> WeaponClass;


	UPROPERTY(VisibleAnywhere)
	class UWidgetComponent* GeneratorWidget = nullptr;

	UPROPERTY(EditAnywhere)
	float GenerateMaxAmount = 1.f;
	UPROPERTY(EditAnywhere)
	float ChargeAmount = 0.005f;

	UPROPERTY(Replicated)
	float CurrentGernerateAmount = 0.f;

	UPROPERTY(Replicated)
	bool ChargeFinish = false;

	UPROPERTY()
	class UMaterialInstanceDynamic* DynamicDissolveMaterialInstance1 = nullptr;
	UPROPERTY(EditAnywhere)
	class UMaterialInstance* DissolveMaterialInstance1 = nullptr;

	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType;
};
