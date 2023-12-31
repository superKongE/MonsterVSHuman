// Fill out your copyright notice in the Description page of Project Settings.


#include "Rocket.h"
#include "Kismet/GameplayStatics.h"
#include "MyGame/Weapon/RocketMovementComponent.h"

ARocket::ARocket()
{
	RocketMovementComponent = CreateDefaultSubobject<URocketMovementComponent>(TEXT("RocketMovementComponent"));
	RocketMovementComponent->InitialSpeed = 4500.f;
	RocketMovementComponent->MaxSpeed = 4500.f;
	RocketMovementComponent->bRotationFollowsVelocity = true;
	RocketMovementComponent->SetIsReplicated(true);
}

// 발사체가 충돌했을때 호출되는 함수
void ARocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor == GetOwner()) return;

	APawn* FirePawn = GetInstigator();
	if (FirePawn)
	{
		AController* FireController = FirePawn->GetController();
		if (FireController)
		{
			UGameplayStatics::ApplyRadialDamageWithFalloff(
				this,
				Damage, // 최대 데미지
				50, // 최소 데미지
				GetActorLocation(), // 원의 중심 위치
				DamageInnerRadius, // 내부 반경 반지름
				DamageOuterRadius, // 외부 반경 반지름
				DamageFallOff, // 내부에서 거리가 멀어질 수록 데미지 감소량
				UDamageType::StaticClass(), // 데미지 타입
				TArray<AActor*>(), // 이 데미지를 받지 않을 액터의 배열 (예를들어 발사한 자신 또는 팀원들), 빈 배열을 줌으로써 자신 포함 모두 데미지를 받는다
				this, // 피해를 유발하는 액터
				FireController // 피해를 유발하는 액터의 컨트롤러
			);
		}
	}

	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}