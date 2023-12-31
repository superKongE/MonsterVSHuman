// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu.h"
#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"
#include "StartMapHUD.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "Components/EditableTextBox.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/TextBlock.h"
#include "ServerRoomSlot.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "FindSessionsCallbackProxy.h"

UMultiplayerSessionsSubsystem* UMenu::GetUMultiPlayerSessionSubsystem()
{
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		UMultiplayerSessionsSubsystem* MultiplayerSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
		return MultiplayerSubsystem;
	}

	return nullptr;
}

// �޴� ������ ȣ���
void UMenu::MenuSetup(int32 NumberOfPublicConnections, FString TypeOfMatch, FString LobbyPath)
{
	PathToLobby = FString::Printf(TEXT("%s?listen"), *LobbyPath);
	NumPublicConnections = NumberOfPublicConnections;
	MatchType = TypeOfMatch;
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;

	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeUIOnly InputModeData;
			InputModeData.SetWidgetToFocus(TakeWidget());
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true);
		}
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
	}

	// ��������Ʈ�� �Լ� ���
	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic(this, &ThisClass::OnCreateSession);
		MultiplayerSessionsSubsystem->MultiplayerOnFindSessionsComplete.AddUObject(this, &ThisClass::OnFindSessions);
		MultiplayerSessionsSubsystem->MultiplayerOnJoinSessionComplete.AddUObject(this, &ThisClass::OnJoinSession);
		MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &ThisClass::OnDestroySession);
		MultiplayerSessionsSubsystem->MultiplayerOnStartSessionComplete.AddDynamic(this, &ThisClass::OnStartSession);
	}
}
// �޴� ���Ž� ȣ���
void UMenu::MenuTearDown()
{
	RemoveFromParent();
	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeGameOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false);
		}
	}
}

// ������
bool UMenu::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}
	if (HostButton)
	{
		HostButton->OnClicked.AddDynamic(this, &ThisClass::HostButtonClicked);
	}
	if (RefreshButton)
	{
		RefreshButton->OnClicked.AddDynamic(this, &ThisClass::RefreshButtonClicked);
	}

	if(GetWorld() && GetWorld()->GetFirstPlayerController() && GetWorld()->GetFirstPlayerController()->GetHUD())
		StartMapHUD = Cast<AStartMapHUD>(GetWorld()->GetFirstPlayerController()->GetHUD());

	return true;
}

void UMenu::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
	MenuTearDown();
	Super::OnLevelRemovedFromWorld(InLevel, InWorld);
}

// �� ���� ��û�� ó�� �Ϸ�Ǹ� ��������Ʈ�� ���� ȣ��Ǵ� �Լ���
void UMenu::OnCreateSession(bool bWasSuccessful)
{
	HostButton->SetIsEnabled(true);

	if (bWasSuccessful)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			World->ServerTravel(PathToLobby);
		}
	}
}
void UMenu::OnFindSessions(TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful)
{
	RefreshButton->SetIsEnabled(true);

	if (!bWasSuccessful) return;

	if (MultiplayerSessionsSubsystem == nullptr) return;

	int32 ArrayIndex = 0;

	for (FOnlineSessionSearchResult& Result : SessionResults)
	{
		if (!Result.IsValid()) continue;

		FString ServerName = "NoName";
		FString SettingsValue;

		Result.Session.SessionSettings.Get(FName("MatchType"), SettingsValue);
		Result.Session.SessionSettings.Get(FName("SERVER_NAME_KEY"), ServerName);
	
		if (SettingsValue == MatchType)
		{
			FServerInfo Info;
			Info.ServerName = ServerName;
			Info.MaxPlayers = Result.Session.SessionSettings.NumPublicConnections;
			Info.CurrentPlayers = Info.MaxPlayers - Result.Session.NumOpenPublicConnections;

			//// ServerList�� ���� ����߰�
			if (ServerList)
			{
				if (StartMapHUD && StartMapHUD->ServerRoomSlotClass && StartMapHUD->ServerRoomSlot)
				{
					StartMapHUD->ServerRoomSlot = CreateWidget<UServerRoomSlot>(GetWorld(), StartMapHUD->ServerRoomSlotClass);
					StartMapHUD->ServerRoomSlot->ServerName->SetText(FText::FromString(ServerName));
					StartMapHUD->ServerRoomSlot->CurrentPlayers->SetText(FText::FromString(FString::FromInt(Info.CurrentPlayers)));
					StartMapHUD->ServerRoomSlot->MaxPlayers->SetText(FText::FromString(FString::FromInt(Info.MaxPlayers)));
					StartMapHUD->ServerRoomSlot->ArrayIndex = ArrayIndex;
					StartMapHUD->ServerRoomSlot->JoinButton->OnClicked.AddDynamic(StartMapHUD->ServerRoomSlot, &UServerRoomSlot::ServerRoomSlot_JoinButtonClicked);
					ServerList->AddChild(StartMapHUD->ServerRoomSlot);
				}
			}
		}
		ArrayIndex++;
	}
}
void UMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem)
	{
		IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			FString Address;
			SessionInterface->GetResolvedConnectString(NAME_GameSession, Address);
			APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
			if (PlayerController)
			{
				PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
			}
		}
	}
}
void UMenu::OnDestroySession(bool bWasSuccessful)
{
	HostButton->SetIsEnabled(true);
	RefreshButton->SetIsEnabled(true);
}
void UMenu::OnStartSession(bool bWasSuccessful)
{
}

void UMenu::ClearServerList()
{
	if (ServerList)
	{
		ServerList->ClearChildren();
	}
}

// �� �����ϱ� ��ư Ŭ����
void UMenu::HostButtonClicked() // �� �����ϱ�
{
	if (MultiplayerSessionsSubsystem)
	{
		HostButton->SetIsEnabled(false);
		MultiplayerSessionsSubsystem->CreateSession(NumPublicConnections, MatchType, ServerNameTextBox->GetText().ToString());
	}
}
// �� ���ΰ�ħ ��ư Ŭ����
void UMenu::RefreshButtonClicked() // ���ΰ�ħ Ŭ����
{
	// ���ΰ�ħ ��ư ��Ȱ��ȭ �� ��� ����� ����� ���� �˻�
	// ���� �˻��� �Ϸ�Ǹ� OnFindSession�� ȣ��ȴ�
	if (MultiplayerSessionsSubsystem)
	{
		// ���ΰ�ħ ��ư ��Ȱ��ȭ
		RefreshButton->SetIsEnabled(false);
		// ServerList�� ����� ��� ������
		ServerList->ClearChildren();
		// Session �˻� ��û
		MultiplayerSessionsSubsystem->FindSessions(10000);
	}
}
