// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyHUD.h"
#include "MyGame/PlayerController/CharacterController.h"
#include "MyGame/HUD/LobbyPawnNameWidget.h"
#include "Mygame/HUD/LobbyMenu.h"
#include "MyGame/Lobby/LobbyPawn.h"
#include "Components/TextBlock.h"

void ALobbyHUD::BeginPlay()
{
	Super::BeginPlay();

	AddLobbyMenu();
}

void ALobbyHUD::AddLobbyMenu()
{
	OwningPlayerController = OwningPlayerController == nullptr ? GetOwningPlayerController() : OwningPlayerController;
	if (OwningPlayerController && LobbyMenuClass)
	{
		// �κ� UI ������ ȭ�鿡 �߰�
		LobbyMenu = CreateWidget<ULobbyMenu>(OwningPlayerController, LobbyMenuClass);
		LobbyMenu->AddToViewport();

		ACharacterController* PlayerController = Cast<ACharacterController>(OwningPlayerController);
		PlayerController->SetLobbyHUD();
		PlayerController->SetShowMouseCursor(true);
	}
}