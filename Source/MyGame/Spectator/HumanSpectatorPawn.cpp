
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


// ������ ������ �÷��̾� ��������
void AHumanSpectatorPawn::Server_GetNextSpectator_Implementation()
{
	MyGameState = MyGameState == nullptr ? GetWorld()->GetGameState<AMyGameState>() : MyGameState;
	PlayerController = PlayerController == nullptr ? Cast<ACharacterController>(Controller) : PlayerController;
	if (MyGameState && PlayerController)
	{
		// ����ִ� �ΰ��� ������ �ش� �ΰ��� ����
		if (MyGameState->GetSurvivalHumanCount() > 0)
		{
			SpectatorCharacter = MyGameState->GetSpectatorNextHumanPlayer();
			if (SpectatorCharacter)
			{
				PlayerController->SetViewTarget(SpectatorCharacter->Controller);
			}
		}
		// ������ ���� ����
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