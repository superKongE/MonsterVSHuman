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

// �߻�ü�� �浹������ ȣ��Ǵ� �Լ�
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
				Damage, // �ִ� ������
				50, // �ּ� ������
				GetActorLocation(), // ���� �߽� ��ġ
				DamageInnerRadius, // ���� �ݰ� ������
				DamageOuterRadius, // �ܺ� �ݰ� ������
				DamageFallOff, // ���ο��� �Ÿ��� �־��� ���� ������ ���ҷ�
				UDamageType::StaticClass(), // ������ Ÿ��
				TArray<AActor*>(), // �� �������� ���� ���� ������ �迭 (������� �߻��� �ڽ� �Ǵ� ������), �� �迭�� �����ν� �ڽ� ���� ��� �������� �޴´�
				this, // ���ظ� �����ϴ� ����
				FireController // ���ظ� �����ϴ� ������ ��Ʈ�ѷ�
			);
		}
	}

	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}