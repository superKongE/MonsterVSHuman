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

	//                     콜백 함수가 바인딩 되어 있지 않으면
	if (ReturnButton && !ReturnButton->OnClicked.IsBound())
	{
		ReturnButton->OnClicked.AddDynamic(this, &UReturnToMenu::ReturnButtonClicked);
	}

	return true;
}

// 메뉴 보이기
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
			// MultiplayerSessionsSubsystem 의 커스템 델리게이트에 세션 파괴 요청완료시 호출될 콜백함수 바인딩
			MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &UReturnToMenu::OnDestroySession);
		}
	}
}
 // 메뉴 없애기
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
		// 바인딩된 델리게이트 제거
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
		// 서버일경우
		if (GameMode)
		{
			// 메뉴로 돌아가기
			GameMode->ReturnToMainMenuHost();
		}
		// 클라이언트일 경우
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

// ReturnButton 클릭시 호출되는 콜백함수
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
				// OnPlayerLeftGame() 함수를 델리게이트에 바인딩
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

// 유저가 나갈려고할때
void UReturnToMenu::OnPlayerLeftGame()
{
	if (MultiplayerSessionsSubsystem)
	{
		// 세션 파괴
		MultiplayerSessionsSubsystem->DestroySession();
	}
}


// 해당 키를 이미 다른 액션이 사용하고 있으면 true 반환, 키 변경이 불가능함
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
// 유효하지 않은 키 삭제
void UReturnToMenu::RemoveInvalidKey()
{
	InputSettings = InputSettings == nullptr ? UInputSettings::GetInputSettings() : InputSettings;
	if (InputSettings)
	{
		InputSettings->RemoveInvalidKeys();
	}
}