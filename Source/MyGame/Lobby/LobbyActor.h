// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LobbyActor.generated.h"

UCLASS()
class MYGAME_API ALobbyActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ALobbyActor();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(EditAnywhere)
	class UCameraComponent* Camera = nullptr;
};
