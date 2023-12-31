 // Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "CharacterHUD.generated.h"

USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()

public:
	class UTexture2D* CrosshairCenter;
	class UTexture2D* CrosshairLeft;
	class UTexture2D* CrosshairRight;
	class UTexture2D* CrosshairBottom;
	class UTexture2D* CrosshairTop;

	float CrosshairSpread;
	FLinearColor CrosshairColor;
};

UCLASS()
class MYGAME_API ACharacterHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void DrawHUD() override;
	virtual void BeginPlay() override;

	void AddCharacterOverlay();
	void AddAnnouncement();
	void AddElimAnnouncement(FString Attacker, FString Victim);
	void AddChatBox();
	void AddMap();

public:
	void DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor);
	void DrawCrosshair(UMaterialInstance* Material, FVector2D ViewportCenter);
	void SetHUDPackage(FHUDPackage& HUD) { HUDPackage = HUD; }

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UUserWidget> CharacterOverlayClass;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UUserWidget> AnnouncementClass;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UUserWidget> ChatBoxClass;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UUserWidget> ChatMessageClass;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UUserWidget> MapClass;

	UPROPERTY()
	class APlayerController* OwningPlayerController = nullptr;

	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay = nullptr;

	UPROPERTY()
	class UAnnouncement* Announcment = nullptr;

	UPROPERTY()
	class UChatBox* ChatBox = nullptr;

	UPROPERTY()
	class UMapWidget* MapWidget = nullptr;
	
private:
	FHUDPackage HUDPackage;
	float CrosshairSpreadMax = 16.f;
	float Countdown = 0.f;
	UPROPERTY(EditAnywhere)
	float ElimAnnouncementTime = 5.f;

	UFUNCTION()
	void ElimAnnouncementTimerFinished(UElimAnnouncement* MsgToRemove);

	UPROPERTY()
	TArray<UElimAnnouncement*> ElimMessages;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UElimAnnouncement> ElimAnnouncementClass;
};
