// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DirectionalLightActor.generated.h"

UENUM(BlueprintType)
enum class EWeatherState : uint8
{
	EWS_Night UMETA(DisplayName = "Night"),
	EWS_Morning UMETA(DisplayName = "Morning"),
	EWS_Default UMETA(DisplayName = "Default")
};

UCLASS()
class MYGAME_API ADirectionalLightActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ADirectionalLightActor();

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:	
	virtual void Tick(float DeltaTime) override;

	void SetNight();
	void SetMorning();


private:
	UPROPERTY(EditAnywhere)
	class UDirectionalLightComponent* DirectionalLight = nullptr;

	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* Mesh = nullptr;

	UPROPERTY(EditAnywhere)
	float ChangeSpeed = 2.5f;

	UPROPERTY(Replicated)
	EWeatherState WeatherState;
	bool bDefault;

	UPROPERTY(Replicated)
	float Current = -20.f;
};
