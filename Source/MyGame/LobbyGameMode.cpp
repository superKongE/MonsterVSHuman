// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"
#include "Math/UnrealMathUtility.h"
#include "MyGame/Lobby/LobbyPawn.h"
#include "MyGame/Lobby/LobbyMonster.h"
#include "MyGame/MyGameState.h"
#include "MyGame/PlayerController/CharacterController.h"
#include "MyGame/HumanGameInstance.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/Actor.h"
#include "GameFramework/GameSession.h"
#include "MultiplayerSessions/Public/MultiplayerSessionsSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"

void ALobbyGameMode::BeginPlay()
{
	Super::BeginPlay();

	ServerGameInstance = ServerGameInstance == nullptr ? Cast<UHumanGameInstance>(GetGameInstance()) : ServerGameInstance;

	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem)
	{
		// Online Subsystem으로 부터 Session 인터페이스를 얻어온다
		// Session 인터페이스를 통해 세션 생성, 검색, 참가, 파괴 등을 처리
		SessionInterface = Subsystem->GetSessionInterface();
	}
}
void ALobbyGameMode::GetSeamlessTravelActorList(bool bToTransition, TArray<class AActor*>& ActorList)
{
	//ActorList.Add(this);

	Super::GetSeamlessTravelActorList(bToTransition, ActorList);
}

// 플레이어가 서버에 접근이 완료되면 호출되는 함수
// 접근하려는 플레이어를 처리할려면 PreLogin
void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	ACharacterController* PlayerController = Cast<ACharacterController>(NewPlayer);

	UWorld* World = GetWorld();
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;

	TArray<AActor*> PlayerStart;
	UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStart);
	int32 NumberOfPlayer = GameState.Get()->PlayerArray.Num();

	ServerGameInstance = ServerGameInstance == nullptr ? Cast<UHumanGameInstance>(GetGameInstance()) : ServerGameInstance;

	if (SpawnCharacter && SpawnMosnter && PlayerController && ServerGameInstance)
	{
		if (PlayerCount != 0)
		{
			ALobbyPawn* LobbyCharacter = World->SpawnActor<ALobbyPawn>(SpawnCharacter, PlayerStart[PlayerCount - 1]->GetActorLocation(), PlayerStart[PlayerCount - 1]->GetActorRotation(), SpawnParams);
			if (LobbyCharacter)
			{
				PlayerController->SetIsLobby(true);
				PlayerController->OnPossess(LobbyCharacter);

				PlayerCount++;
				PlayerController->SetAmIMonster(false);
				PlayerController->Client_SetUserName();

				ServerGameInstance->SetMaxUserCount(1);

				if (!SessionInterface.IsValid())
				{
					IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
					if (Subsystem)
					{
						// Online Subsystem으로 부터 Session 인터페이스를 얻어온다
						// Session 인터페이스를 통해 세션 생성, 검색, 참가, 파괴 등을 처리
						SessionInterface = Subsystem->GetSessionInterface();
					}
				}

				FNamedOnlineSession* Session = SessionInterface->GetNamedSession(NAME_GameSession);
				if (Session != nullptr)
				{
					UpdateSession();
				}
			}
		}
		// 방장이 괴물 
		else if (PlayerCount == 0)
		{
			ALobbyMonster* LobbyMonster = World->SpawnActor<ALobbyMonster>(SpawnMosnter, FVector(-12555.f, 22130.f, -2195.f), FRotator(-15.f, -90.f, 0.f), SpawnParams);
			if (LobbyMonster)
			{
				PlayerController->SetIsLobby(true);
				PlayerController->OnPossess(LobbyMonster);

				PlayerCount++;
				PlayerController->SetAmIMonster(true);
				PlayerController->Client_SetUserName();

				ServerGameInstance->SetMaxUserCount(1);
			}
		}
	}
}
void ALobbyGameMode::UpdateSession()
{
	UWorld* const World = GetWorld();
	if (World)
	{
		AGameModeBase* const Game = World->GetAuthGameMode();
		if (Game)
		{
			if (Game->GameSession)
			{
				FJoinabilitySettings OutSettings;

				Game->GameSession->GetSessionJoinability(NAME_GameSession, OutSettings);

				Game->GameSession->UpdateSessionJoinability(NAME_GameSession,
					OutSettings.bPublicSearchable, OutSettings.bAllowInvites,
					OutSettings.bJoinViaPresence, OutSettings.bJoinViaPresenceFriendsOnly);
			}
		}
	}
}

// 플레이어가 서버에서 나가거나 소멸되었을때 호출되는 함수
void ALobbyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	FNamedOnlineSession* Session = SessionInterface->GetNamedSession(NAME_GameSession);
	if (Session != nullptr)
	{
		Session->NumOpenPublicConnections++;

		UpdateSession();
	}

	PlayerCount--;
	if (ACharacterController* CharacterController = Cast<ACharacterController>(Exiting))
	{
		if (CharacterController->GetLobbyReady())
		{
			CharacterController->SetLobbyReady(false);
			ReadyPlayerCount--;
		}
	}

	ServerGameInstance = ServerGameInstance == nullptr ? Cast<UHumanGameInstance>(GetGameInstance()) : ServerGameInstance;
	if (ServerGameInstance)
	{
		ServerGameInstance->SetMaxUserCount(-1);
	}
}

// 게임으로 이동하기 전에 모든 플레이어들의 HUD를 파괴
void ALobbyGameMode::StartGameBeforeDestoryLobbyHUD()
{
	if (ReadyPlayerCount == PlayerCount)
	{
		ServerGameInstance = ServerGameInstance == nullptr ? Cast<UHumanGameInstance>(GetGameInstance()) : ServerGameInstance;
		AMyGameState* MyGameState = Cast<AMyGameState>(GameState);
		if (MyGameState && ServerGameInstance)
		{
			MyGameState->SetMaxUserCount(ServerGameInstance->GetMaxUserCount());
		}

		UWorld* World = GetWorld();
		if (World)
		{
			for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; It++)
			{
				ACharacterController* CharacterController = Cast<ACharacterController>(*It);
				if (CharacterController)
				{
					CharacterController->DestroyLobbyHUD();
				}
			}
		}
		else
		{
			CanStartGame = false;
		}
	}
	else
	{
		CanStartGame = false;
	}
}
// 모두 파괴가 완료되면 게임으로 이동
void ALobbyGameMode::DestroyLobbyHUDCompleted()
{
	DestroyLobyHUDCount++;
	ServerGameInstance = ServerGameInstance == nullptr ? Cast<UHumanGameInstance>(GetGameInstance()) : ServerGameInstance;

	// 모든 유저가 LobbyHUD를 파괴했으면
	if (ServerGameInstance && DestroyLobyHUDCount == ServerGameInstance->GetMaxUserCount())
	{
		UWorld* World = GetWorld();

		if (World)
		{
			AGameModeBase* const Game = World->GetAuthGameMode();
			if (Game)
			{
				if (Game->GameSession)
				{
					FJoinabilitySettings OutSettings;

					Game->GameSession->GetSessionJoinability(NAME_GameSession, OutSettings);

					Game->GameSession->UpdateSessionJoinability(NAME_GameSession,
						false, false,
						false, OutSettings.bJoinViaPresenceFriendsOnly);
				}
			}

			CanStartGame = true;
			bUseSeamlessTravel = true;

			// 서버가 새 Level로 이동한 다음 연결된 모든 클라이언트가 서버가 이동하는 Level로 이동
			// 서버가 모든 클라이언트의 APlyaerController::ClientTravel() 을 호출하여 클라이언트들을 이동시킨다
			// 만약 APlyaerController::ClientTravel()를 Client에서 호출하면 이동할 주소(Level)을 전달해야 한다
			World->ServerTravel(FString("/Game/Maps/Mountains_Map?listen"));
		}
	}
	else
	{
		CanStartGame = false;
	}
}


void ALobbyGameMode::SetReadyPlayerCount(bool bReady)
{
	if (bReady)
	{
		ReadyPlayerCount++;
	}
	else
	{
		ReadyPlayerCount--;
	}
}