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

// Tick 함수
// 크로스헤어 벌어짐 정도, 색깔 값을 설정후 크로스헤어 그리는 함수 호출
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


// 캐릭터 죽으면 알리는 메시지
void ACharacterHUD::AddElimAnnouncement(FString Attacker, FString Victim)
{
	OwningPlayerController = OwningPlayerController == nullptr ? GetOwningPlayerController() : OwningPlayerController;
	if (OwningPlayerController && ElimAnnouncementClass)
	{
		UElimAnnouncement* ElimAnnouncementWidget = CreateWidget<UElimAnnouncement>(OwningPlayerController, ElimAnnouncementClass);
		ElimAnnouncementWidget->SetElimAnnouncementText(Attacker, Victim);
		ElimAnnouncementWidget->AddToViewport();

		// 기존의 메세지들을 위로 올리기
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
		//                                       호출할 함수 이름,                    호출할 함수에 전달할 매개변수
		ElimMsgDelegate.BindUFunction(this, FName("ElimAnnouncementTimerFinished"), ElimAnnouncementWidget);
		// ElimAnnouncementTime 초 후에 델리게이트에 바인딩된 함수가 호출된다
		GetWorldTimerManager().SetTimer(ElimMsgTimer, ElimMsgDelegate, ElimAnnouncementTime, false);
	}
}
// 죽음 알림 메시지 삭제
void ACharacterHUD::ElimAnnouncementTimerFinished(UElimAnnouncement* MsgToRemove)
{
	if (MsgToRemove)
	{
		MsgToRemove->RemoveFromParent();
	}
}


// 크로스헤어 그림
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
