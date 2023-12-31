
#include "BlockWall.h"
#include "MyGame/GameMode/MainGameMode.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

ABlockWall::ABlockWall()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
}

void ABlockWall::BeginPlay()
{
	Super::BeginPlay();
	
	if(HasAuthority())
		MainGameMode = Cast<AMainGameMode>(UGameplayStatics::GetGameMode(this));
}

void ABlockWall::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// ������ ���۵Ǹ� �ı�
	if (HasAuthority() && MainGameMode)
	{
		if (MainGameMode->GetMatchState() == MatchState::Morning)
		{
			Destroy();
		}
	}
}

