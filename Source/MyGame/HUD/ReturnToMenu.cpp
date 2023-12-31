// Fill out your copyright notice in the Description page of Project Settings.


#include "ReturnToMenu.h"
#include "GameFramework/PlayerController.h"
#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"
#include "GameFramework/GameModeBase.h"
#include "GameFramework/InputSettings.h"

#include "MyGame/Character/FPSCharacter.h"
#include "MyGame/Character/FPSMonster.h"
#include "MyGame/Lobby/LobbyPawn.h"
#include "MyGame/Lobby/LobbyMonster.h"
#include "MyGame/Spectator/HumanSpectatorPawn.h"

bool UReturnToMenu::Initialize()
{
	if (!Super::Initialize()) return false;

	//                     �ݹ� �Լ��� ���ε� �Ǿ� ���� ������
	if (ReturnButton && !ReturnButton->OnClicked.IsBound())
	{
		ReturnButton->OnClicked.AddDynamic(this, &UReturnToMenu::ReturnButtonClicked);
	}

	return true;
}

// �޴� ���̱�
void UReturnToMenu::MenuSetup(bool IsLobby)
{
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;

	UWorld* World = GetWorld();
	if (World)
	{
		PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController;
		if (PlayerController)
		{
			if (!IsLobby)
			{
				FInputModeGameAndUI InputModeData;
				InputModeData.SetWidgetToFocus(TakeWidget());
				PlayerController->SetInputMode(InputModeData);
			}
			PlayerController->SetShowMouseCursor(true);
		}
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
		if (MultiplayerSessionsSubsystem)
		{
			// MultiplayerSessionsSubsystem �� Ŀ���� ��������Ʈ�� ���� �ı� ��û�Ϸ�� ȣ��� �ݹ��Լ� ���ε�
			MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &UReturnToMenu::OnDestroySession);
		}
	}
}
 // �޴� ���ֱ�
void UReturnToMenu::MenuTearDown()
{
	RemoveFromParent();

	UWorld* World = GetWorld();
	/*if (World)
	{
		PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController;
		if (PlayerController)
		{
			FInputModeGameOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
		}
	}*/

	if (ReturnButton && ReturnButton->OnClicked.IsBound())
	{
		// ���ε��� ��������Ʈ ����
		ReturnButton->OnClicked.RemoveDynamic(this, &UReturnToMenu::ReturnButtonClicked);
	}
	if (MultiplayerSessionsSubsystem && MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.IsBound())
	{
		MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.RemoveDynamic(this, &UReturnToMenu::OnDestroySession);
	}
}

void UReturnToMenu::OnDestroySession(bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		ReturnButton->SetIsEnabled(true);
		return;
	}

	UWorld* World = GetWorld();
	if (World)
	{
		AGameModeBase* GameMode = World->GetAuthGameMode<AGameModeBase>();
		// �����ϰ��
		if (GameMode)
		{
			// �޴��� ���ư���
			GameMode->ReturnToMainMenuHost();
		}
		// Ŭ���̾�Ʈ�� ���
		else
		{
			PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController;
			if (PlayerController)
			{
				PlayerController->ClientReturnToMainMenuWithTextReason(FText());
			}
		}
	}
}

// ReturnButton Ŭ���� ȣ��Ǵ� �ݹ��Լ�
void UReturnToMenu::ReturnButtonClicked()
{
	ReturnButton->SetIsEnabled(false);

	UWorld* World = GetWorld();
	if (World)
	{
		//APlayerController* FirstPlayerController = World->GetFirstPlayerController();
		APlayerController* FirstPlayerController = GetOwningPlayer();
		if (FirstPlayerController)
		{
			AFPSCharacter* Character = Cast<AFPSCharacter>(FirstPlayerController->GetPawn());
			if (Character)
			{
				// OnPlayerLeftGame() �Լ��� ��������Ʈ�� ���ε�
				Character->OnLeftGame.AddDynamic(this, &UReturnToMenu::OnPlayerLeftGame);
				Character->ServerLeftGame();
			}
			else if (AFPSMonster* Mosnter = Cast<AFPSMonster>(FirstPlayerController->GetPawn()))
			{
				Mosnter->OnLeftGames.AddDynamic(this, &UReturnToMenu::OnPlayerLeftGame);
				Mosnter->ServerLeftGame();
			}
			else if (ALobbyPawn* LobbyPawn = Cast<ALobbyPawn>(FirstPlayerController->GetPawn()))
			{
				LobbyPawn->OnLeftGame.AddDynamic(this, &UReturnToMenu::OnPlayerLeftGame);
				LobbyPawn->ServerLeftGame();
			}
			else if (ALobbyMonster* LobbyMonster = Cast<ALobbyMonster>(FirstPlayerController->GetPawn()))
			{
				LobbyMonster->OnLeftGame.AddDynamic(this, &UReturnToMenu::OnPlayerLeftGame);
				LobbyMonster->ServerLeftGame();
			}
			else if (AHumanSpectatorPawn* SpectatorPawn = Cast<AHumanSpectatorPawn>(FirstPlayerController->GetPawn()))
			{
				SpectatorPawn->OnLeftGame.AddDynamic(this, &UReturnToMenu::OnPlayerLeftGame);
				SpectatorPawn->LeftGame();
			}
			else
			{
				ReturnButton->SetIsEnabled(true);
			}
		}
	}
}

// ������ ���������Ҷ�
void UReturnToMenu::OnPlayerLeftGame()
{
	if (MultiplayerSessionsSubsystem)
	{
		// ���� �ı�
		MultiplayerSessionsSubsystem->DestroySession();
	}
}


// �ش� Ű�� �̹� �ٸ� �׼��� ����ϰ� ������ true ��ȯ, Ű ������ �Ұ�����
bool UReturnToMenu::IsThatKeyWhoUsing(FKey InputActionKey)
{
	InputSettings = InputSettings == nullptr ? UInputSettings::GetInputSettings() : InputSettings;
	if (InputSettings == nullptr) return false;

	const TArray<FInputActionKeyMapping> &InputActionKeyMap = InputSettings->GetActionMappings();
	for (const FInputActionKeyMapping& ActionMapping : InputActionKeyMap)
	{
		if (ActionMapping.Key == InputActionKey)
		{
			return true;
		}
	}

	const TArray<FInputAxisKeyMapping>& InputAxisKeyMap = InputSettings->GetAxisMappings();
	for (const FInputAxisKeyMapping& ActionMapping : InputAxisKeyMap)
	{
		if (ActionMapping.Key == InputActionKey)
		{
			return true;
		}
	}

	return false;
}
// ��ȿ���� ���� Ű ����
void UReturnToMenu::RemoveInvalidKey()
{
	InputSettings = InputSettings == nullptr ? UInputSettings::GetInputSettings() : InputSettings;
	if (InputSettings)
	{
		InputSettings->RemoveInvalidKeys();
	}
}