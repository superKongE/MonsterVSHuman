

#include "DirectionalLightActor.h"
#include "Components/DirectionalLightComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Math/UnrealMathUtility.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SceneCapture2D.h"

ADirectionalLightActor::ADirectionalLightActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	DirectionalLight = CreateDefaultSubobject<UDirectionalLightComponent>(TEXT("DirectionalLight"));

	WeatherState = EWeatherState::EWS_Default;
	Current = -20.f;
}

void ADirectionalLightActor::BeginPlay()
{
	Super::BeginPlay();

	SetActorRotation(FRotator(3.f, 0.f, 0.f));
}
void ADirectionalLightActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ADirectionalLightActor, WeatherState);
	DOREPLIFETIME(ADirectionalLightActor, Current);
}

void ADirectionalLightActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 상태에 따라 값을 바꿈으로써 낮과 밤을 전환
	if (HasAuthority())
	{
		if (WeatherState == EWeatherState::EWS_Night)
		{
			Current += ChangeSpeed * DeltaTime;

			if (Current >= 3.f)
			{
				Current = 3.f;
				WeatherState = EWeatherState::EWS_Default;
			}

			FRotator temp = UKismetMathLibrary::MakeRotator(0.f, Current, 0.f);
			SetActorRotation(temp);
		}
		else if (WeatherState == EWeatherState::EWS_Morning)
		{
			Current -= ChangeSpeed * DeltaTime;

			if (Current <= -20.f)
			{
				Current = -20.f;
				WeatherState = EWeatherState::EWS_Default;
			}

			FRotator temp = UKismetMathLibrary::MakeRotator(0.f, Current, 0.f);
			SetActorRotation(temp);
		}
	}
}


void ADirectionalLightActor::SetNight()
{
	WeatherState = EWeatherState::EWS_Night;
}
void ADirectionalLightActor::SetMorning()
{
	WeatherState = EWeatherState::EWS_Morning;
}

