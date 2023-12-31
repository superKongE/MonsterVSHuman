// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BlockWall.generated.h"

UCLASS()
class MYGAME_API ABlockWall : public AActor
{
	GENERATED_BODY()
	
public:	
	ABlockWall();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(EditAnywhere)
	class UStaticMeshComponent* Mesh = nullptr;

	UPROPERTY()
	class AMainGameMode* MainGameMode = nullptr;
};
