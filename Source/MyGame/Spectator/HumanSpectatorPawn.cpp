
#include "HumanSpectatorPawn.h"
#include "MyGame/MyGameState.h"
#include "MyGame/Character/FPSCharacter.h"
#include "MyGame/Character/FPSMonster.h"
#include "MyGame/PlayerController/CharacterController.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

AHumanSpectatorPawn::AHumanSpectatorPawn()
{
	PrimaryActorTick.bCanEverTick = false;

}
void AHumanSpectatorPawn::BeginPlay()
{
	Super::BeginPlay();
	
	PlayerController = PlayerController == nullptr ? Cast<ACharacterController>(Controller) : PlayerController;
	if (PlayerController)
	{
		Server_GetNextSpectator();
	}
}
void AHumanSpectatorPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
void AHumanSpectatorPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AHumanSpectatorPawn::LeftClick);
}


// 다음에 관전할 플레이어 가져오기
void AHumanSpectatorPawn::Server_GetNextSpectator_Implementation()
{
	MyGameState = MyGameState == nullptr ? GetWorld()->GetGameState<AMyGameState>() : MyGameState;
	PlayerController = PlayerController == nullptr ? Cast<ACharacterController>(Controller) : PlayerController;
	if (MyGameState && PlayerController)
	{
		// 살아있는 인간이 있으면 해당 인간을 관전
		if (MyGameState->GetSurvivalHumanCount() > 0)
		{
			SpectatorCharacter = MyGameState->GetSpectatorNextHumanPlayer();
			if (SpectatorCharacter)
			{
				PlayerController->SetViewTarget(SpectatorCharacter->Controller);
			}
		}
		// 없으면 괴물 관전
		else
		{
			SpectatorMonster = MyGameState->GetSpectatorMonsterPlayer();
			if (SpectatorMonster)
			{
				PlayerController->SetViewTarget(SpectatorMonster->Controller);
			}
		}
	}
}
void AHumanSpectatorPawn::LeftClick()
{
	PlayerController = PlayerController == nullptr ? Cast<ACharacterController>(Controller) : PlayerController;
	if (PlayerController)
	{
		Server_GetNextSpectator();
	}
}


void AHumanSpectatorPawn::LeftGame()
{
	OnLeftGame.Broadcast();
}