// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGame/Combat/MonsterCombatComponent.h"
#include "Kismet/GameplayStatics.h"
#include "MyGame/Character/FPSMonster.h"
#include "MyGame/PlayerController/CharacterController.h"
#include "MyGame/CameraShake/ChargingCameraShake.h"
#include "Camera/CameraComponent.h"

UMonsterCombatComponent::UMonsterCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	ChargingCameraShake = CreateDefaultSubobject<UChargingCameraShake>(TEXT("ChargingCameraShake"));
}

void UMonsterCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	Controller = Controller == nullptr ? Cast<ACharacterController>(Character->Controller) : Controller;

	if (Character != nullptr)
	{
		if (Character->GetCamera())
		{
			DefaultFOV = Character->GetCamera()->FieldOfView; // FieldOfView 현재 카메라의 시야각
			CurrentFOV = DefaultFOV;
		}
	}
}

void UMonsterCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (Controller && Character && Character->IsLocallyControlled())
	{
		TraceUnderCrosshair(HitResult);
		HitTarget = HitResult.ImpactPoint; // 충돌 지점 저장

		// 조준 상태면 확대
		if (IsCharging)
		{
			//                              작을수록 더 확대			작을수록 천천히
			CurrentFOV = FMath::FInterpTo(CurrentFOV, 30, DeltaTime, 0.5);
		
			if (CurrentFOV < CameraShakePoint)
			{
				CameraShakeStart = true;
				ChargingCameraShake = Controller->PlayerCameraManager->StartCameraShake(CameraShakeClass, 10.f);
			}
		}
		else
		{
			Controller->PlayerCameraManager->StopCameraShake(ChargingCameraShake);
			CameraShakeStart = false;
			CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, 10);
		}

		if (Character)
		{
			// 카메라 시야각 설정(확대 및 축소)
			Character->GetCamera()->SetFieldOfView(CurrentFOV);
		}
	}
}


void UMonsterCombatComponent::TraceUnderCrosshair(FHitResult& TraceHitResult)
{
	// 뷰포트 크기 구하기
	FVector2D ViewportSize = FVector2D::ZeroVector;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	// 크로스헤어 위치 지정
	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	// 2d 공간 정보인 크로스헤어 위치를  3d 공간 정보로 변환
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection
	);

	if (bScreenToWorld)
	{
		// 크로스헤어가 3d 공간에서의 시작점
		FVector Start = CrosshairWorldPosition;

		if (Character)
		{
			// 크로스헤어와 캐릭터간의 거리
			float DistanceToCharacter = (Character->GetActorLocation() - Start).Size();
			Start += CrosshairWorldDirection * (DistanceToCharacter + 100.f);
		}

		// 크로스헤어가 3d 공간에서의 끝점
		FVector End = Start + CrosshairWorldDirection * 8000.f;

		// Start 부터 End 까지 충돌검사
		FCollisionQueryParams FQS;
		FQS.AddIgnoredActor(GetOwner());
		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility,
			FQS
		);

		// 충돌한 대상이 없을 경우
		if (!TraceHitResult.bBlockingHit)
		{
			TraceHitResult.ImpactPoint = End;
		}
	}
}

void UMonsterCombatComponent::SetAimming(bool bCharging)
{
	if (Character == nullptr) return;

	IsCharging = bCharging;
}