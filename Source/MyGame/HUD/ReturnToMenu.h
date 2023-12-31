// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ReturnToMenu.generated.h"

/**
 * 
 */
UCLASS()
class MYGAME_API UReturnToMenu : public UUserWidget
{
	GENERATED_BODY()

protected:
	// BeginPlay ¿Í ºñ½Á
	virtual bool Initialize() override;

	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);

	UFUNCTION()
	void OnPlayerLeftGame();

	void RequestGameFinish();
	
public:
	void MenuSetup(bool IsLobby);
	void MenuTearDown();

	UFUNCTION(BlueprintCallable)
	bool IsThatKeyWhoUsing(FKey InputActionKey);

	UFUNCTION(BlueprintCallable)
	void RemoveInvalidKey();


private:
	UPROPERTY(meta = (BindWidget))
	class UButton* ReturnButton = nullptr;

	UFUNCTION()
	void ReturnButtonClicked();

	UPROPERTY()
	class UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem = nullptr;

	UPROPERTY()
	class APlayerController* PlayerController = nullptr;

	UPROPERTY()
	class UInputSettings* InputSettings = nullptr;
};
