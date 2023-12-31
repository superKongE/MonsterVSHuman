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
			DefaultFOV = Character->GetCamera()->FieldOfView; // FieldOfView ���� ī�޶��� �þ߰�
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
		HitTarget = HitResult.ImpactPoint; // �浹 ���� ����

		// ���� ���¸� Ȯ��
		if (IsCharging)
		{
			//                              �������� �� Ȯ��			�������� õõ��
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
			// ī�޶� �þ߰� ����(Ȯ�� �� ���)
			Character->GetCamera()->SetFieldOfView(CurrentFOV);
		}
	}
}


void UMonsterCombatComponent::TraceUnderCrosshair(FHitResult& TraceHitResult)
{
	// ����Ʈ ũ�� ���ϱ�
	FVector2D ViewportSize = FVector2D::ZeroVector;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	// ũ�ν���� ��ġ ����
	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	// 2d ���� ������ ũ�ν���� ��ġ��  3d ���� ������ ��ȯ
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection
	);

	if (bScreenToWorld)
	{
		// ũ�ν��� 3d ���������� ������
		FVector Start = CrosshairWorldPosition;

		if (Character)
		{
			// ũ�ν����� ĳ���Ͱ��� �Ÿ�
			float DistanceToCharacter = (Character->GetActorLocation() - Start).Size();
			Start += CrosshairWorldDirection * (DistanceToCharacter + 100.f);
		}

		// ũ�ν��� 3d ���������� ����
		FVector End = Start + CrosshairWorldDirection * 8000.f;

		// Start ���� End ���� �浹�˻�
		FCollisionQueryParams FQS;
		FQS.AddIgnoredActor(GetOwner());
		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility,
			FQS
		);

		// �浹�� ����� ���� ���
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