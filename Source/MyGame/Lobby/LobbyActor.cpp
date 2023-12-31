
#include "LobbyActor.h"
#include "Components/StaticMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "MyGame/PlayerController/CharacterController.h"

ALobbyActor::ALobbyActor()
{
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
}

void ALobbyActor::BeginPlay()
{
	Super::BeginPlay();
}

void ALobbyActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

