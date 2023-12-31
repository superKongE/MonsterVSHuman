// Fill out your copyright notice in the Description page of Project Settings.


#include "MainGameMode.h"
#include "Kismet/GamePlayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/GameStateBase.h"
#include "MyGame/Character/FPSCharacter.h"
#include "MyGame/Character/FPSMonster.h"
#include "MyGame/PlayerController/CharacterController.h"
#include "MyGame/SpawnPoint/HumanPlayerStart.h"
#include "MyGame/SpawnPoint/MonsterPlayerStart.h"
#include "MyGame/PlayerState/CharacterState.h"
#include "MyGame/HumanGameInstance.h"
#include "MyGame/Spectator/HumanSpectatorPawn.h"
#include "MyGame/MyGameState.h"
#include "MyGame/Weather/DirectionalLightActor.h"
#include "Engine/World.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

namespace MatchState
{
	const FName MonsterWin = FName("MonsterWin");
	const FName HumanWin = FName("HumanWin");
	const FName GameStart = FName("GameStart");
	const FName Night = FName("Night");
	const FName Morning = FName("Morning");
}

// servertravel이 완료되고 호출되는 함수
// postlogin은 호출되지 않는다
void AMainGameMode::GenericPlayerInitialization(AController* Controller)
{
	Super::GenericPlayerInitialization(Controller);

	ACharacterController* PlayerController = Cast<ACharacterController>(Controller);

	// 에디터에서 테스트하기 위한 용도
	/*if (UserCount == 0)
	{
		SpawnMonsterPlayer(PlayerController);
	}
	else
	{
		SpawnHumanPlayer(PlayerController);
	}*/

	PlayerController->SpawnPlayerIsMonster();
}
void AMainGameMode::Logout(AController* Exiting)
{
	ServerGameInstance = ServerGameInstance == nullptr ? Cast<UHumanGameInstance>(GetGameInstance()) : ServerGameInstance;
	if (ServerGameInstance)
	{
		ServerGameInstance->SetMaxUserCount(-1);
	}
	UserCount--;

	Super::Logout(Exiting);
}
AMainGameMode::AMainGameMode()
{
	bDelayedStart = true;
}
// Spawn되는 캐릭터의 BeginPlay에서 호출되는 함수
void AMainGameMode::AddUser()
{
	UserCount++;
	ServerGameInstance = ServerGameInstance == nullptr ? GetGameInstance<UHumanGameInstance>() : ServerGameInstance;
	//ServerGameInstance->GetMaxUserCount()
	if (UserCount == 2)
	{
		UWorld* World = GetWorld();
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		if (World)
		{
			DirectionalLight = World->SpawnActor<ADirectionalLightActor>(DirectionalLightClass, FVector(1930.f, 14470.f, 4620.f), FRotator(3.f, 0.f, 0.f), SpawnParams);
		}
		ServerTime = 0.f;
		// 유저들이 다 들어올때 까지 시간이 얼마나 걸린지
		LevelStartingTime = GetWorld()->GetTimeSeconds();
		/*FString str = FString::Printf(TEXT("GameMode LevelStartingTime : %f"), LevelStartingTime);
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Red,
				str
			);
		}*/

		StartMatch();
	}
}
void AMainGameMode::BeginPlay()
{
	Super::BeginPlay();
}

void AMainGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ServerTime += DeltaTime;
	if (MatchState == MatchState::InProgress && ServerTime >= 10.f)
	{
		ServerTime = 0.f;
		SetMatchState(MatchState::Morning);
	}
	else if (MatchState == MatchState::Night)
	{
		if (ServerTime >= NightChangeTime * Count)
		{
			Count += 1.f;
			SetMatchState(MatchState::Morning);
		}
	}
	else if (MatchState == MatchState::Morning)
	{
		if (ServerTime >= NightChangeTime * Count)
		{
			Count += 1.f;
			SetMatchState(MatchState::Night);
		}
	}
}

// MatchState가 변할때마다 호출되는 함수
void AMainGameMode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for (FConstPlayerControllerIterator Iter = GetWorld()->GetPlayerControllerIterator(); Iter; ++Iter)
	{
		ACharacterController* Controller = Cast<ACharacterController>(*Iter);
		if (Controller)
		{
			if (MatchState == MatchState::Night || MatchState == MatchState::Morning)
			{
				if (MatchState == MatchState::Night)
				{
					CurrentMatchState = EMatchState::EMS_Night;
					DirectionalLight->SetNight();
				}
				else if (MatchState == MatchState::Morning)
				{
					CurrentMatchState = EMatchState::EMS_Night;
					DirectionalLight->SetMorning();
				}
			}
			/*else if (MatchState == MatchState::HumanWin || MatchState == MatchState::MonsterWin)
			{
				ServerTime = 0.f;
			}*/
			Controller->OnMatchStateSet(MatchState);
		}
	}
}


// 플레이어가 죽었을 때 들어오는 요청
void AMainGameMode::PlayerEliminated(class AFPSCharacter* ElimedCharacter, class ACharacterController* VictimController, class ACharacterController* AttackerController)
{
	ACharacterState* AttackerPlayerState = AttackerController ? Cast<ACharacterState>(AttackerController->PlayerState) : nullptr;
	ACharacterState* VictimPlayerState = VictimController ? Cast<ACharacterState>(VictimController->PlayerState) : nullptr;

	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState)
	{
		AttackerPlayerState->AddToScore(1.f);
	}

	if (VictimPlayerState)
	{
		VictimPlayerState->AddToDefeat(1);
		MyGameState->SetSurvivalHumanCount(-1);

		if (MyGameState->GetSurvivalHumanCount() == 0 && !IsAlreadyGameFinished())
		{
			SetMatchState(MatchState::MonsterWin);
		}

		ElimedCharacter->Elim(false);
	}

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; It++)
	{
		ACharacterController* CharacterController = Cast<ACharacterController>(*It);
		if (CharacterController)
		{
			CharacterController->BroadcastElim(AttackerPlayerState, VictimPlayerState);
		}
	}
}
void AMainGameMode::PlayerEliminated(class AFPSMonster* ElimedMonster, class ACharacterController* VictimController, class ACharacterController* AttackerController)
{
	ACharacterState* AttackerPlayerState = AttackerController ? Cast<ACharacterState>(AttackerController->PlayerState) : nullptr;
	ACharacterState* VictimPlayerState = VictimController ? Cast<ACharacterState>(VictimController->PlayerState) : nullptr;

	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState)
	{
		AttackerPlayerState->AddToScore(1.f);
	}

	if (VictimPlayerState)
	{
		VictimPlayerState->AddToDefeat(1);
		MyGameState->SetSurvivalMonstercount(-1);
		MonsterCount--;

		if (MonsterCount == 0 && !IsAlreadyGameFinished())
		{
			SetMatchState(MatchState::HumanWin);
		}
		
		ElimedMonster->Elim(false);
	}

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; It++)
	{
		ACharacterController* CharacterController = Cast<ACharacterController>(*It);
		if (CharacterController)
		{
			CharacterController->BroadcastElim(AttackerPlayerState, VictimPlayerState);
		}
	}
}
void AMainGameMode::RequestRespone(class ACharacter* RequestCharacter, class AController* RequestController, bool IsMonster)
{
	if (RequestCharacter)
	{
		// PlayerController 빙의 해제
		RequestCharacter->Reset();
		RequestCharacter->Destroy();
	}

	// 괴물이 요청시 게임 끝
	if (IsMonster)
	{
		SetMatchState(MatchState::HumanWin);
	}
	// 사람이 요청시 관전모드로 전환
	else
	{
		HumanCount--;
		if (HumanCount == 0)
		{
			SetMatchState(MatchState::MonsterWin);
		}

		// 죽은사람 관전모드로
		UWorld* World = GetWorld();
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;

		MyGameState = MyGameState == nullptr ? GetWorld()->GetGameState<AMyGameState>() : MyGameState;
		if (MyGameState)
		{
			ACharacterController* PlayerController = Cast<ACharacterController>(RequestController);
			if (PlayerController)
			{
				AHumanSpectatorPawn* SpectatorPawn = World->SpawnActor<AHumanSpectatorPawn>(SpectatorHumanClass, RequestCharacter->GetActorLocation(), RequestCharacter->GetActorRotation(), SpawnParams);
				PlayerController->Possess(SpectatorPawn);
				PlayerController->SetOwner(SpectatorPawn);
			}
		}
	}
}

void AMainGameMode::PlayerLeftGame(AFPSCharacter* Character)
{
	// 플레이어가 나가는것을 요청했을 경우 들고 있는 아이템을 떨어뜨리게 한다
	AFPSCharacter* FPSCharacter = Cast<AFPSCharacter>(Character);
	if (FPSCharacter)
	{
		FPSCharacter->Elim(true);

		MyGameState->SetSurvivalHumanCount(-1);

		if (MyGameState->GetSurvivalHumanCount() == 0 && !IsAlreadyGameFinished())
		{
			SetMatchState(MatchState::MonsterWin);
		}
	}
}
void AMainGameMode::PlayerLeftGame(AFPSMonster* Monster)
{
	// 플레이어가 나가는것을 요청했을 경우 들고 있는 아이템을 떨어뜨리게 한다
	AFPSMonster* FPSMonster = Cast<AFPSMonster>(Monster);
	if (FPSMonster)
	{
		FPSMonster->Elim(true);
	}
}


void AMainGameMode::GameFinished()
{
	ReturnToMainMenuHost();
}


void AMainGameMode::CompleteCharge()
{
	ChargedGenerator++;

	// 4개가 충전 완료되면 인간 승리
	if (ChargedGenerator == ChargedGeneratorCount)
	{
		SetMatchState(MatchState::HumanWin);
	}
}

bool AMainGameMode::IsAlreadyGameFinished()
{
	if (MatchState != MatchState::HumanWin && MatchState != MatchState::MonsterWin)
	{
		return false;
	}

	return true;
}


void AMainGameMode::SpawnMonsterPlayer(ACharacterController* PlayerController)
{
	UWorld* World = GetWorld();
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;

	TArray<AActor*> PlayerStart;
	UGameplayStatics::GetAllActorsOfClass(this, AMonsterPlayerStart::StaticClass(), PlayerStart);
	int32 Selection = FMath::RandRange(0, PlayerStart.Num() - 1);

	MyGameState = MyGameState == nullptr ? GetGameState<AMyGameState>() : MyGameState;
	AFPSMonster* FPSMonster = World->SpawnActor<AFPSMonster>(SpawnMonster, PlayerStart[Selection]->GetActorLocation(), PlayerStart[Selection]->GetActorRotation(), SpawnParams);
	ACharacterState* CharacterState = Cast<ACharacterState>(FPSMonster->GetPlayerState());
	
	if (FPSMonster && MyGameState)
	{
		PlayerController->SetIsLobby(false);
		PlayerController->OnPossess(FPSMonster);
		PlayerController->Client_SetUserName();
		PlayerController->SetAmIMonster(true);

		MonsterCount++;
		MyGameState->AddMonsterArray(FPSMonster);
		MyGameState->SetSurvivalMonstercount(1);
	}
}
void AMainGameMode::SpawnHumanPlayer(ACharacterController* PlayerController)
{
	UWorld* World = GetWorld();
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;

	TArray<AActor*> PlayerStart;
	UGameplayStatics::GetAllActorsOfClass(this, AHumanPlayerStart::StaticClass(), PlayerStart);
	int32 Selection = FMath::RandRange(0, PlayerStart.Num() - 1);

	MyGameState = MyGameState == nullptr ? GetGameState<AMyGameState>() : MyGameState;
	AFPSCharacter* FPSCharacter = World->SpawnActor<AFPSCharacter>(SpawnCharacter, PlayerStart[Selection]->GetActorLocation(), PlayerStart[Selection]->GetActorRotation(), SpawnParams);
	if (FPSCharacter && MyGameState)
	{
		PlayerController->SetIsLobby(false);
		PlayerController->OnPossess(FPSCharacter);
		PlayerController->Client_SetUserName();
		PlayerController->SetAmIMonster(false);

		HumanCount++;
		MyGameState->AddHumanArray(FPSCharacter);
		MyGameState->SetSurvivalHumanCount(1);
	}
}