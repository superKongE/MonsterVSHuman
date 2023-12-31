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

// servertravel�� �Ϸ�ǰ� ȣ��Ǵ� �Լ�
// postlogin�� ȣ����� �ʴ´�
void AMainGameMode::GenericPlayerInitialization(AController* Controller)
{
	Super::GenericPlayerInitialization(Controller);

	ACharacterController* PlayerController = Cast<ACharacterController>(Controller);

	// �����Ϳ��� �׽�Ʈ�ϱ� ���� �뵵
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
// Spawn�Ǵ� ĳ������ BeginPlay���� ȣ��Ǵ� �Լ�
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
		// �������� �� ���ö� ���� �ð��� �󸶳� �ɸ���
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

// MatchState�� ���Ҷ����� ȣ��Ǵ� �Լ�
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


// �÷��̾ �׾��� �� ������ ��û
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
		// PlayerController ���� ����
		RequestCharacter->Reset();
		RequestCharacter->Destroy();
	}

	// ������ ��û�� ���� ��
	if (IsMonster)
	{
		SetMatchState(MatchState::HumanWin);
	}
	// ����� ��û�� �������� ��ȯ
	else
	{
		HumanCount--;
		if (HumanCount == 0)
		{
			SetMatchState(MatchState::MonsterWin);
		}

		// ������� ��������
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
	// �÷��̾ �����°��� ��û���� ��� ��� �ִ� �������� ����߸��� �Ѵ�
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
	// �÷��̾ �����°��� ��û���� ��� ��� �ִ� �������� ����߸��� �Ѵ�
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

	// 4���� ���� �Ϸ�Ǹ� �ΰ� �¸�
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