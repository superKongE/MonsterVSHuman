
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MyGame/Weapon/WeaponType.h"
#include "MyGame/GameMode/MainGameMode.h"
#include "CharacterController.generated.h"

UCLASS()
class MYGAME_API ACharacterController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaTime);
	virtual void OnPossess(APawn* InPawn) override;

protected:
	virtual void BeginPlay() override;
	virtual float GetServerTime();
	virtual void ReceivedPlayer() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void SetupInputComponent() override;
	virtual void GetSeamlessTravelActorList(bool bToEntry, TArray<class AActor*>& ActorList);

	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);

	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);
	
public:
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDStemina(float Stemina, float MaxStemina);
	void SetHUDDash(float Dash, float MaxDash);
	void SetHUDScore(float ScoreAmount);
	void SetHUDDefeat(int32 DefeatAmount);
	void SetHUDAmmo(int32 AmmoAmount);
	void SetHUDKnife();
	void SetHUDCarriedAmmo(int32 CarriedAmmoAmount);
	void SetHUDWeaponType(EWeaponType WeaponType);
	void SetHUDMatchCountDown(float Time);
	void SetHUDTime();
	//void SetHUDReadyTime();
	void SetHUDVisible();
	void SetLobbyHUD();
	void DestroyLobbyHUD();
	void SetThrowCoolTimeBarHUD(float CoolTime);
	void ShowMap(bool bShow);


	void HumanWin();
	void MonsterWin();
	UFUNCTION()
	void GameFinished(bool bWasSuccessful);

	
	void HighPingWarning();
	void StopHighPingWarning();
	void CheckPing(float DeltaTime);

	void BroadcastElim(ACharacterState* Attacker, ACharacterState* Victim);
	UFUNCTION(Client, Reliable)
	void ClientElimAnnouncement(ACharacterState* Attacker, ACharacterState* Victim);


	// 채팅 관련 함수들
	bool LeftClick();
	void ShowChat();
	void ShowChatBoxHUD();
	void CloseChatBoxHUD();
	void SetChatBoxHUDMessage(const FString& Message);
	UFUNCTION(BlueprintCallable)
	void SendMessage();
	UFUNCTION(BlueprintCallable)
	void MakeMessage();
	UFUNCTION(Server, Reliable)
	void Server_SendMessage(const FString& Message, bool IsTeam);
	UFUNCTION(Client, Reliable)
	void Client_SendMessage(const FString& Message);


	// 로비 채팅
	void LobbyShowChatBoxHUD();
	void LobbyCloseChatBoxHUD();
	void LobbySetChatBoxHUDMessage(const FString& Message);
	UFUNCTION(BlueprintCallable)
	void LobbySendMessage();
	UFUNCTION(BlueprintCallable)
	void LobbyMakeMessage();
	UFUNCTION(Server, Reliable)
	void LobbyServer_SendMessage(const FString& Message);
	UFUNCTION(Client, Reliable)
	void LobbyClient_SendMessage(const FString& Message);


	UFUNCTION(Client, Reliable)
	void ClientDestroyLobbyHUD();

	UFUNCTION(BlueprintCallable)
	void SetReadyPlayerCount();
	UFUNCTION(BlueprintCallable)
	void StartGame();
	UFUNCTION(Server, Reliable)
	void Server_SetReadyPlayerCount(bool bReady);
	UFUNCTION(Server, Reliable)
	void Server_StartGame();


	UFUNCTION(Client, Reliable)
	void Client_SetUserName();
	UFUNCTION(Server, Reliable)
	void Server_SetUserName(const FString& name);


	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool GetLobbyReady() const { return LobbyReady; }
	FORCEINLINE void SetLobbyReady(bool bReady) { LobbyReady = bReady; }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool GetIsLobby() const { return IsLobby; }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool GetIsTeamChat() const { return IsTeamChat; }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetTeamChat(bool IsTeam) { IsTeamChat = IsTeam; }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool GetAmIMonster() const { return AmIMonster; }
	FORCEINLINE FName GetCurrentMatchState() const { return MatchState; }
	UFUNCTION(Server, Reliable)
	void Server_SetIsLobby(bool bLobby);


	UFUNCTION(Client, Reliable)
	void SetClientMonster();
	UFUNCTION(Client, Reliable)
	void SetClientHuman();

	void SetIsLobby(bool bLobby);
	UFUNCTION(Client, Reliable)
	void Client_SetIsLobby(bool bLobby);

	void SetHumanInputMode();


	UFUNCTION(Server, Reliable)
	void Server_DestroyLobbyHUDComplete();


	void SpawnPlayerIsMonster();
	void SetAmIMonster(bool bMonster);

	void LeftGame();

	//void HandleReady();

	void ShowReturnToMainMenu();

	void CheckTimeSync(float DeltaTime);

	void OnMatchStateSet(FName State);
	UFUNCTION()
	void OnRep_MatchState();

	UPROPERTY()
	class AActor* OwnerCharacter = nullptr;

private:
	UPROPERTY()
	class UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem = nullptr;
	UPROPERTY()
	class AFPSCharacter* HumanPlayer = nullptr;
	UPROPERTY()
	class AFPSMonster* MonsterPlayer = nullptr;
	UPROPERTY()
	class ACharacterHUD* CharacterHUD = nullptr;
	UPROPERTY()
	class ACharacterHUD* MonsterHUD = nullptr;
	UPROPERTY()
	class AMainGameMode* MainGameMode = nullptr;
	UPROPERTY()
	class UHumanGameInstance* HumanGameInstance = nullptr;
	UPROPERTY()
	class ALobbyHUD* LobbyHUD = nullptr;
	UPROPERTY()
	class UChatBox* ChatBoxHUD = nullptr;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UUserWidget> ReturnToMainMenuWidget;

	UPROPERTY()
	class UReturnToMenu* ReturnToMenu = nullptr;

	float ClientServerDelta = 0.f; // 서버와 클라이언트간의 시간 차이
	UPROPERTY(EditAnywhere, Category = GameTime)
	float TimeSyncFrequency = 5.f; // 5초마다 서버랑 시간 동기화 수행
	UPROPERTY(EditAnywhere, Category = GameTime)
	float TimeSyncRunningTime = 0.f; // 5초인지 체크

	// 경기시간 (초)
	UPROPERTY(EditAnywhere, Category = GameTime)
	float NightTime = 10.f;
	UPROPERTY(EditAnywhere, Category = GameTime)
	float MorningTime = 10.f;
	float LastServerTime = 1.f;
	UPROPERTY(Replicated)
	float ServerTime;


	UPROPERTY(EditAnywhere, Category = GameTime)
	int32 CooldownTime = 10; // 몇초동안 Cooldown 상태할지

	//UPROPERTY(EditAnywhere, Category = GameTime)
	//float ReadyCountTime = 10.f; // 몇초동안 Ready 상태할지
	//float CurrentReadyCountTime;

	float GameStartTime = 0; // 게임이 몇초후에 시작했는지
	UPROPERTY(Replicated)
	float LevelStartingTime = 0.f;
	uint32 CountdownInt = 1;

	bool AmIMonster = false;
	bool bWarmingUp = false;
	bool GameStart = false;
	bool bReturnToMenuOpen = false;
	UPROPERTY(Replicated)
	bool LobbyReady = false;
	UPROPERTY(Replicated)
	bool IsLobby = false;
	bool IsTeamChat = false;

	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState = MatchState::WaitingToStart;

	FTimerHandle ReadyTimer;
	FTimerHandle HumanWinTimer;
	FTimerHandle MonsterWinTimer;


	// 채팅 관련 변수들
	//UPROPERTY(Replicated)
	//FString UserName;
	FString TempMessage;
	int32 PreviousMessageLength = 0;
	UPROPERTY(EditAnywhere)
	int32 MessageLengthLimit = 30;
	bool IsOpenChatBox = false;
	bool LobbyIsOpenChatBox = false;

	// 핑 관련 변수들
	float HighPingRunningTime = 0.f;  // 핑 경고 표시가 끝난 후 부터 경과된 시간
	UPROPERTY(EditAnywhere)
	float HighPingDuration = 5.f; // 핑 경고 지속 시간
	UPROPERTY(EditAnywhere)
	float CheckPingFrequncy = 20.f;  // 몇초 마다 핑이 높은지 체크할지
	UPROPERTY(EditAnywhere)
	float HighPingThreshold = 50.f;  // 핑 임계값
	float PingAnimationRunningTime = 0.f;
};
