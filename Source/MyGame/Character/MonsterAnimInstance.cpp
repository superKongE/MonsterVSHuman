// Fill out your copyright notice in the Description page of Project Settings.


#include "MonsterAnimInstance.h"
#include "MyGame/Character/FPSMonster.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

// Anim Instance의 초기화 시점에 호출되는 함수
void UMonsterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Monster = Cast<AFPSMonster>(TryGetPawnOwner());
}

// 매 프레임마다 호출되는 함수
void UMonsterAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	Monster = Monster == nullptr ? Cast<AFPSMonster>(TryGetPawnOwner()) : Monster;
	if (Monster)
	{
		FVector Velocity = Monster->GetVelocity();
		Velocity.Z = 0.f;
		Speed = Velocity.Size();

		IsInAir = Monster->GetIsInAir();
		IsDash = Monster->GetIsDash();
		AttackButtonPressed = Monster->GetAttackButtonPressed();
		IsAccelerating = Monster->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0 ? true : false;
		IsDead = Monster->GetIsDead();
		AO_Pitch = Monster->GetAO_Pitch();

		Direction = CalculateDirection(Monster->GetVelocity(), Monster->GetActorRotation());
	}
}