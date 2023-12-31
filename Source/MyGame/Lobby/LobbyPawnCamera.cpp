
#include "LobbyPawnCamera.h"
#include "Camera/CameraComponent.h"

ULobbyPawnCamera::ULobbyPawnCamera()
{
	PrimaryComponentTick.bCanEverTick = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));

	Camera->SetWorldLocation(FVector(-12607.5f, 22225.4f, -2123.9f));
}

void ULobbyPawnCamera::BeginPlay()
{
	Super::BeginPlay();
}

void ULobbyPawnCamera::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

