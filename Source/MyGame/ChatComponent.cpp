#include "ChatComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "MyGame/Character/FPSCharacter.h"
#include "MyGame/Character/FPSMonster.h"
#include "MyGame/PlayerController/CharacterController.h"

UChatComponent::UChatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UChatComponent::BeginPlay()
{
	Super::BeginPlay();
}


void UChatComponent::SendMessage(const FString& Message, class ACharacterController* PlayerController)
{
	Server_SendMessage(Message, PlayerController);
}
void UChatComponent::Server_SendMessage_Implementation(const FString& Message, class ACharacterController* PlayerController)
{
	Multicast_SendMessage(Message, PlayerController);
}
void UChatComponent::Multicast_SendMessage_Implementation(const FString& Message, class ACharacterController* PlayerController)
{
	if (PlayerController && PlayerController->IsLocalController())
	{
		PlayerController->SetChatBoxHUDMessage(Message);
	}
}