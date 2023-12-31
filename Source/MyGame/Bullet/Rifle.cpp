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

// �߻�ü�� �浹������ ȣ��Ǵ� �Լ�
void ARifle::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!Cast<AFPSMonster>(OtherActor))
	{
		Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
		return;
	}

	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()); // ���� Ŭ������ �����ڸ� ������

	if (OwnerCharacter)
	{
		AController* OwnerController = OwnerCharacter->Controller; // ���� �������� ��Ʈ�ѷ� ������

		if (OwnerController && OwnerController->HasAuthority())
		{
			//                         ���� ���� ���, ������,  ������ ����� ��Ʈ�ѷ�, ������ ���, ������ ����
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

	// �Ѿ� ���� ����Ʈ ����
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}