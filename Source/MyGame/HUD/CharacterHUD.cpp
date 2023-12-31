// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterHUD.h"
#include "GameFramework/PlayerController.h"
#include "CharacterOverlay.h"
#include "ElimAnnouncement.h"
#include "Components/HorizontalBox.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Blueprint/WidgetLayoutLibrary.h"

#include "MyGame/HUD/GeneratorWidget.h"
#include "MyGame/Announcement.h"
#include "MyGame/HUD/ChatBox.h"
#include "MyGame/HUD/MapWidget.h"

// Tick �Լ�
// ũ�ν���� ������ ����, ���� ���� ������ ũ�ν���� �׸��� �Լ� ȣ��
void ACharacterHUD::DrawHUD()
{
	Super::DrawHUD();

	FVector2D ViewportSize;

	if (GEngine)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);

		const FVector2D ViewportCenter(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

		float SpreadScale = CrosshairSpreadMax * HUDPackage.CrosshairSpread;

		if (HUDPackage.CrosshairCenter)
		{
			FVector2D Spread(0.f, 0.f);
			DrawCrosshair(HUDPackage.CrosshairCenter, ViewportCenter, Spread, HUDPackage.CrosshairColor);
		}
		if (HUDPackage.CrosshairRight)
		{
			FVector2D Spread(SpreadScale, 0.f);
			DrawCrosshair(HUDPackage.CrosshairRight, ViewportCenter, Spread, HUDPackage.CrosshairColor);
		}
		if (HUDPackage.CrosshairLeft)
		{
			FVector2D Spread(-SpreadScale, 0.f);
			DrawCrosshair(HUDPackage.CrosshairLeft, ViewportCenter, Spread, HUDPackage.CrosshairColor);
		}
		if (HUDPackage.CrosshairBottom)
		{
			FVector2D Spread(0.f, SpreadScale);
			DrawCrosshair(HUDPackage.CrosshairBottom, ViewportCenter, Spread, HUDPackage.CrosshairColor);
		}
		if (HUDPackage.CrosshairTop)
		{
			FVector2D Spread(0.f, -SpreadScale);
			DrawCrosshair(HUDPackage.CrosshairTop, ViewportCenter, Spread, HUDPackage.CrosshairColor);
		}
	}
}

void ACharacterHUD::BeginPlay()
{
	Super::BeginPlay();
	
	AddCharacterOverlay();
	AddAnnouncement();
	AddChatBox();
	AddMap();
}

void ACharacterHUD::AddCharacterOverlay()
{
	OwningPlayerController = OwningPlayerController == nullptr ? GetOwningPlayerController() : OwningPlayerController;
	if (OwningPlayerController && CharacterOverlayClass)
	{
		CharacterOverlay = CreateWidget<UCharacterOverlay>(OwningPlayerController, CharacterOverlayClass);
		CharacterOverlay->AddToViewport();
	}
}
void ACharacterHUD::AddAnnouncement()
{
	OwningPlayerController = OwningPlayerController == nullptr ? GetOwningPlayerController() : OwningPlayerController;
	if (OwningPlayerController && AnnouncementClass)
	{
		Announcment = CreateWidget<UAnnouncement>(OwningPlayerController, AnnouncementClass);
		Announcment->AddToViewport();
	}
}
void ACharacterHUD::AddChatBox()
{
	OwningPlayerController = OwningPlayerController == nullptr ? GetOwningPlayerController() : OwningPlayerController;
	if (OwningPlayerController && ChatBoxClass)
	{
		ChatBox = CreateWidget<UChatBox>(OwningPlayerController, ChatBoxClass);
		ChatBox->AddToViewport();
	}
}
void ACharacterHUD::AddMap()
{
	OwningPlayerController = OwningPlayerController == nullptr ? GetOwningPlayerController() : OwningPlayerController;
	if (OwningPlayerController && MapClass)
	{
		MapWidget = CreateWidget<UMapWidget>(OwningPlayerController, MapClass);
		MapWidget->AddToViewport();
		MapWidget->MapImage->SetVisibility(ESlateVisibility::Hidden);
	}
}


// ĳ���� ������ �˸��� �޽���
void ACharacterHUD::AddElimAnnouncement(FString Attacker, FString Victim)
{
	OwningPlayerController = OwningPlayerController == nullptr ? GetOwningPlayerController() : OwningPlayerController;
	if (OwningPlayerController && ElimAnnouncementClass)
	{
		UElimAnnouncement* ElimAnnouncementWidget = CreateWidget<UElimAnnouncement>(OwningPlayerController, ElimAnnouncementClass);
		ElimAnnouncementWidget->SetElimAnnouncementText(Attacker, Victim);
		ElimAnnouncementWidget->AddToViewport();

		// ������ �޼������� ���� �ø���
		for (UElimAnnouncement* Msg : ElimMessages)
		{
			if (Msg && Msg->AnnouncementBox)
			{
				UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(Msg->AnnouncementBox);
				if (CanvasSlot)
				{
					FVector2D Position = CanvasSlot->GetPosition();
					FVector2D Newposition(CanvasSlot->GetPosition().X, Position.Y - CanvasSlot->GetSize().Y);
					CanvasSlot->SetPosition(Newposition);
				}
			}
		}

		ElimMessages.Add(ElimAnnouncementWidget);

		FTimerHandle ElimMsgTimer;
		FTimerDelegate ElimMsgDelegate;
		//                                       ȣ���� �Լ� �̸�,                    ȣ���� �Լ��� ������ �Ű�����
		ElimMsgDelegate.BindUFunction(this, FName("ElimAnnouncementTimerFinished"), ElimAnnouncementWidget);
		// ElimAnnouncementTime �� �Ŀ� ��������Ʈ�� ���ε��� �Լ��� ȣ��ȴ�
		GetWorldTimerManager().SetTimer(ElimMsgTimer, ElimMsgDelegate, ElimAnnouncementTime, false);
	}
}
// ���� �˸� �޽��� ����
void ACharacterHUD::ElimAnnouncementTimerFinished(UElimAnnouncement* MsgToRemove)
{
	if (MsgToRemove)
	{
		MsgToRemove->RemoveFromParent();
	}
}


// ũ�ν���� �׸�
void ACharacterHUD::DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor)
{
	const float TextureWidth = Texture->GetSizeX();
	const float TextureHeight = Texture->GetSizeY();
	const FVector2D DrawPoint(
		ViewportCenter.X - (TextureWidth / 2.f) + Spread.X,
		ViewportCenter.Y - (TextureHeight / 2.f) + Spread.Y
	);

	DrawTexture(
		Texture,
		DrawPoint.X,
		DrawPoint.Y,
		TextureWidth,
		TextureHeight,
		0.f,
		0.f,
		1.f,
		1.f,
		CrosshairColor
	);
}
