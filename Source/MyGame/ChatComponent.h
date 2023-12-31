// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ChatComponent.generated.h"

UCLASS( ClassGroup=(Blueprintable), meta=(BlueprintSpawnableComponent) )
class MYGAME_API UChatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UChatComponent();

protected:
	virtual void BeginPlay() override;

protected:
	TArray<FString> ChatMessages;

public:	
	UFUNCTION(BlueprintCallable)
	void SendMessage(const FString& Message, class ACharacterController* PlayerController);
	UFUNCTION(Server, Reliable)
	void Server_SendMessage(const FString& Message, class ACharacterController* PlayerController);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SendMessage(const FString& Message, class ACharacterController* PlayerController);

	UFUNCTION(BlueprintCallable)
	FORCEINLINE TArray<FString> GetChatMessage() const { return ChatMessages; };

	//void SetCharacter(ACharacter* Character) const;

private:
	UPROPERTY()
	class ACharacter* Character = nullptr;

	UPROPERTY()
	class AFPSCharacter* Human = nullptr;

	UPROPERTY()
	class AFPSMonster* Monster = nullptr;
};
