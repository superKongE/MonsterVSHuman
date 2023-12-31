// Fill out your copyright notice in the Description page of Project Settings.


#include "Rifle.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "MyGame/Character/FPSMonster.h"

ARifle::ARifle()
{
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->InitialSpeed = 15000.f;
	ProjectileMovementComponent->MaxSpeed = 15000.f;
	ProjectileMovementComponent->ProjectileGravityScale = 0.f; 
	ProjectileMovementComponent->SetIsReplicated(true);
}

// 발사체가 충돌했을때 호출되는 함수
void ARifle::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!Cast<AFPSMonster>(OtherActor))
	{
		Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
		return;
	}

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()); // 현재 클래스의 소유자를 가져옴

	if (OwnerCharacter)
	{
		AController* OwnerController = OwnerCharacter->Controller; // 현재 소유자의 컨트롤러 가져옴

		if (OwnerController && OwnerController->HasAuthority())
		{
			//                         공격 받은 대상, 데미지,  공격한 대상의 컨트롤러, 공격한 대상, 데미지 종류
			if (Cast<AFPSMonster>(Hit.GetActor()) && Hit.BoneName.ToString() == FString("head"))
			{
				UGameplayStatics::ApplyDamage(OtherActor, HeadShotDamage, OwnerController, this, UDamageType::StaticClass());
			}
			else if(Cast<AFPSMonster>(Hit.GetActor()) && Hit.BoneName.ToString() != FString("head"))
			{
				UGameplayStatics::ApplyDamage(OtherActor, Damage, OwnerController, this, UDamageType::StaticClass());
			}
		}
	}

	// 총알 맞은 이펙트 생성
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}