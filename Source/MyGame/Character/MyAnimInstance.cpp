// Fill out your copyright notice in the Description page of Project Settings.


#include "MyAnimInstance.h"
#include "FPSCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "MyGame/Weapon.h"
#include "MyGame/Combat/CombatComponent.h"

void UMyAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	Character = Cast<AFPSCharacter>(TryGetPawnOwner());
}

void UMyAnimInstance::NativeUpdateAnimation(float DeltaTime) 
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (Character == nullptr) return;

	FVector Velocity = Character->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	IsInAir = Character->GetIsInAir();
	EquippedWeapon = Character->GetEquippedWeapon();
	IsEquippedWeapon = Character->GetIsEquippedWeapon();
	IsAimming = Character->GetCombatComponent()->GetbAiming();
	IsAccelerating = Character->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0 ? true : false;
	IsDead = Character->GetIsDead();
	IsReloading = Character->GetCombatComponent()->GetIsReloading();
	AO_Pitch = Character->GetAO_Pitch();

	Direction = CalculateDirection(Character->GetVelocity(), Character->GetActorRotation());

	if (IsEquippedWeapon && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && Character->GetMesh())
	{
		// LeftHandSocket의 위치를 world 공간의 정보로 가져오기
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
		FVector OutPosition;
		FRotator OutRotation;

		// world 공간에서 hand_r 뼈에서 상대공간으로 변환
		Character->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));
	}
}
