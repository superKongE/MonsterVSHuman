#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MonsterCombatComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MYGAME_API UMonsterCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UMonsterCombatComponent();
	friend class AFPSMonster;

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:

	void TraceUnderCrosshair(FHitResult& TraceHitResult);

	void SetAimming(bool bCharging);

private:
	UPROPERTY()
	class AFPSMonster* Character;

	UPROPERTY()
	class ACharacterController* Controller = nullptr;

public:
	FVector HitTarget = FVector::ZeroVector;
	FHitResult HitResult;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UCameraShakeBase> CameraShakeClass;

	UPROPERTY(EditAnywhere)
	class UCameraShakeBase* ChargingCameraShake = nullptr;

private:
	float CurrentFOV;
	float DefaultFOV;
	float CameraShakePoint = 50.f;
	UPROPERTY(EditAnywhere)
	float CameraShakeScale = 10.f;
	bool IsCharging;
	bool CameraShakeStart = false;
};
