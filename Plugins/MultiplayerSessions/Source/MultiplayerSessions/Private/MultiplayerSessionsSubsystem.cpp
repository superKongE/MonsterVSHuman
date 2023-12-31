// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"

UMultiplayerSessionsSubsystem::UMultiplayerSessionsSubsystem():
	CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete)),
	FindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsComplete)),
	JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionComplete)),
	DestroySessionCompleteDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionComplete)),
	StartSessionCompleteDelegate(FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnStartSessionComplete))
{
	// static�Լ��� Get()�� ���� OnlineSubsystem �����͸� ���´�
	// �پ��� �������̽��� ���� ����
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem)
	{
		// Online Subsystem���� ���� Session �������̽��� ���´�
		// Session �������̽��� ���� ���� ����, �˻�, ����, �ı� ���� ó��
		SessionInterface = Subsystem->GetSessionInterface();
	}
}


// ���� ��û �Լ���(����, �˻�, ����, ������)
void UMultiplayerSessionsSubsystem::CreateSession(int32 NumPublicConnections, FString MatchType, FString ServerName)
{
	if (!SessionInterface.IsValid())
	{
		return;
	}

	// ���� ������ �����ִٸ� ����
	FNamedOnlineSession* ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);
	if (ExistingSession != nullptr)
	{
		DestroySession();

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Red,
				FString(TEXT("Retry"))
			);
		}
		return;
	}

	// ��������Ʈ List�� ��������Ʈ �߰�
	CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

	// LastSessionSettings�� ���������ͷ� �������ش�.
	LastSessionSettings = MakeShareable(new FOnlineSessionSettings());

	// Steam�� ����� ���̱� ������ LanMatch = false
	//LastSessionSettings->bIsLANMatch = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : false;
	LastSessionSettings->bIsLANMatch  = false;
	// ���ǿ� ���� ������ �ο�
	LastSessionSettings->NumPublicConnections = NumPublicConnections;
	// ������ ���� ��(���� ��) �������� ����
	LastSessionSettings->bAllowJoinInProgress = true;
	// ��Ȯ���� ���� - Steam ����ڿ� ���� �������� ���� ���� ����
	LastSessionSettings->bAllowJoinViaPresence = true;
	// Steam�� ���� ������ �˸���.(�ٸ� �÷��̾ �ش� ������ ã�� ���� ����)
	LastSessionSettings->bShouldAdvertise = true;
	// ��Ȯ���� ���� - Steam ����ڿ� ���� �������� ������ ã�� �� �ִ� ����
	LastSessionSettings->bUsesPresence = true;
	LastSessionSettings->Set(FName("MatchType"), MatchType, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	LastSessionSettings->Set(FName("SERVER_NAME_KEY"), ServerName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	// ���� �ٸ� ����(����?)�� �˻����� �ʰ� �����ϱ� ����
	LastSessionSettings->BuildUniqueId = 2;
	// Lobby API�� ������ ��� Lobby ��� ����
	LastSessionSettings->bUseLobbiesIfAvailable = true;

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	
	if (!SessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *LastSessionSettings))
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);

		MultiplayerOnCreateSessionComplete.Broadcast(false);
	}
	// ���� ������ ������ ��� ������ ������ ������ ����
	else
	{
		bCreateSessionOnDestroy = true;
		LastNumPublicConnections = NumPublicConnections;
		LastMatchType = MatchType;
		LastServerName = ServerName;
	}
}
void UMultiplayerSessionsSubsystem::FindSessions(int32 MaxSearchResults)
{
	if (!SessionInterface.IsValid()) return;

	// ���� ������ �����ִٸ� ����
	auto ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);
	if (ExistingSession != nullptr)
	{
		bCreateSessionOnDestroy = true;

		DestroySession();
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Red,
				FString(TEXT("Retry"))
			);
		}
		return;
	}

	FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);

	LastSessionSearch = MakeShareable(new FOnlineSessionSearch());

	LastSessionSearch->MaxSearchResults = MaxSearchResults;
	//LastSessionSearch->bIsLanQuery = IOnlineSubsystem::Get()->GetSubsystemName() == "NULL" ? true : false; // Lan ��������
	LastSessionSearch->bIsLanQuery = false;
	LastSessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	TSharedRef<FOnlineSessionSearch> SearchSettingsRef = LastSessionSearch.ToSharedRef();

	if (!SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), SearchSettingsRef))
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
		
		TArray<FOnlineSessionSearchResult> TempArray;
		MultiplayerOnFindSessionsComplete.Broadcast(TempArray, false);
	}
}
void UMultiplayerSessionsSubsystem::JoinSession(int32 ArrayIndex)
{
	if (!SessionInterface.IsValid())
	{
		MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
		return;
	}

	JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);

	if (LastSessionSearch->SearchResults[ArrayIndex].IsValid())
	{
		
		const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
		if (!SessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, LastSessionSearch->SearchResults[ArrayIndex]))
		{
			SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);

			MultiplayerOnJoinSessionComplete.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
		}
		else
		{
			SessionInterface->RegisterPlayer(NAME_GameSession, *LocalPlayer->GetPreferredUniqueNetId(), false);
		}
	}
}
void UMultiplayerSessionsSubsystem::DestroySession()
{
	if (!SessionInterface.IsValid())
	{
		MultiplayerOnDestroySessionComplete.Broadcast(false);
		return;
	}

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	SessionInterface->UnregisterPlayer(NAME_GameSession, *LocalPlayer->GetPreferredUniqueNetId());

	DestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);
	auto DestroyedSession = SessionInterface->GetNamedSession(NAME_GameSession);

	if (!SessionInterface->DestroySession(NAME_GameSession))
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Red,
				FString(TEXT("DestroySession fail"))
			);
		}
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
		MultiplayerOnDestroySessionComplete.Broadcast(false);
	}
}
void UMultiplayerSessionsSubsystem::StartSession()
{
}


// Delegate�� ���� ȣ��Ǵ� �Լ���(�� ���ǿ� ���� ó�� ��û�� �Ϸ�Ǹ� ȣ��ȴ�)
void UMultiplayerSessionsSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{ 
	if (SessionInterface)
	{
		// ��������Ʈ List ��� ����
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	}

	MultiplayerOnCreateSessionComplete.Broadcast(bWasSuccessful);
}
void UMultiplayerSessionsSubsystem::OnFindSessionsComplete(bool bWasSuccessful)
{
	if (SessionInterface)
	{
		// ��������Ʈ List ��� ����
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
	}

	if (LastSessionSearch->SearchResults.Num() <= 0)
	{
		TArray<FOnlineSessionSearchResult> TempArray;
		MultiplayerOnFindSessionsComplete.Broadcast(TempArray, false);
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Red,
				FString(TEXT("Not Find"))
			);
		}
		return;
	}

	MultiplayerOnFindSessionsComplete.Broadcast(LastSessionSearch->SearchResults, bWasSuccessful);
}
void UMultiplayerSessionsSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (SessionInterface)
	{
		// ��������Ʈ List ��� ����
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
	}
	MultiplayerOnJoinSessionComplete.Broadcast(Result);
}
void UMultiplayerSessionsSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (SessionInterface)
	{
		// ��������Ʈ List ��� ����
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
	}
	if (bWasSuccessful && bCreateSessionOnDestroy)
	{
		bCreateSessionOnDestroy = false;
	}
	MultiplayerOnDestroySessionComplete.Broadcast(bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnStartSessionComplete(FName SessionName, bool bWasSuccessful)
{
}
