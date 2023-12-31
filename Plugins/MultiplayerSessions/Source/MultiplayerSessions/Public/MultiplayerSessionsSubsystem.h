// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MultiplayerSessionsSubsystem.generated.h"


USTRUCT(BlueprintType)
struct FServerInfo
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintType)
	FString ServerName;
	UPROPERTY(BlueprintType)
	int32 CurrentPlayers;
	UPROPERTY(BlueprintType)
	int32 MaxPlayers;
	UPROPERTY(BlueprintType)
	FString MatchType;
};
// ����� ���� ��������Ʈ ����( Session ���� ���࿡ ���� ó���� �Ϸ�Ǹ� Menu Ŭ������ �ݹ� �Լ��� ȣ���ϱ� ����)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnCreateSessionComplete, bool, bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_TwoParams(FMultiplayerOnFindSessionsComplete, TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_OneParam(FMultiplayerOnJoinSessionComplete, EOnJoinSessionCompleteResult::Type Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnDestroySessionComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnStartSessionComplete, bool, bWasSuccessful);



UCLASS()
class MULTIPLAYERSESSIONS_API UMultiplayerSessionsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	UMultiplayerSessionsSubsystem();

	// Menu Ŭ�������� ȣ��Ǵ� �Լ���
	void CreateSession(int32 NumPublicConnections, FString MatchType, FString ServerName);
	void FindSessions(int32 MaxSearchResults);
	void JoinSession(int32 ArrayIndex);
	void DestroySession();
	void StartSession();

	// Menu Ŭ������ �ݹ� �Լ��� ���ε��� ����� ���� Delegate
	FMultiplayerOnCreateSessionComplete MultiplayerOnCreateSessionComplete;
	FMultiplayerOnFindSessionsComplete MultiplayerOnFindSessionsComplete;
	FMultiplayerOnJoinSessionComplete MultiplayerOnJoinSessionComplete;
	FMultiplayerOnDestroySessionComplete MultiplayerOnDestroySessionComplete;
	FMultiplayerOnStartSessionComplete MultiplayerOnStartSessionComplete;

protected:
	// Session Interface�� ��������Ʈ List�� �߰��� ��������Ʈ�� ���� �ݹ� �Լ���
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	void OnStartSessionComplete(FName SessionName, bool bWasSuccessful);

private:
	IOnlineSessionPtr SessionInterface;
	TSharedPtr<class FOnlineSessionSettings> LastSessionSettings; // �ֱٿ� ������ ������ ����
	TSharedPtr<class FOnlineSessionSearch> LastSessionSearch; // �ֱٿ� �˻��� ���� ���


	// Online Session ��������Ʈ List�� �߰��� ��������Ʈ
	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FDelegateHandle CreateSessionCompleteDelegateHandle;
	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
	FDelegateHandle FindSessionsCompleteDelegateHandle;
	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
	FDelegateHandle JoinSessionCompleteDelegateHandle;
	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
	FDelegateHandle DestroySessionCompleteDelegateHandle;
	FOnStartSessionCompleteDelegate StartSessionCompleteDelegate;
	FDelegateHandle StartSessionCompleteDelegateHandle;

	bool bCreateSessionOnDestroy{ false };
	int32 LastNumPublicConnections;
	FString LastMatchType;
	FString LastServerName;
};