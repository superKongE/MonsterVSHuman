// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "MyGame/Character/FPSCharacter.h"
#include "MyGame/Character/FPSMonster.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

AProjectile::AProjectile()
{
	bReplicates = true;
	PrimaryActorTick.bCanEverTick = false;
	
	CollisionArea = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionArea"));
	RootComponent = CollisionArea;

	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NulletMesh"));
	ProjectileMesh->SetupAttachment(RootComponent);
	ProjectileMesh->SetVisibility(false);
}

void AProjectile::BeginPlay()
{
	Super::BeginPlay();

	if (Tracer)
	{
		TraceParticleComponent = UGameplayStatics::SpawnEmitterAttached(
			Tracer,
			ProjectileMesh,
			FName(),
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition
		);
	}

	if (HasAuthority())
	{
		CollisionArea->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);
	}
}

void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (Tracer)
	{
		TraceParticleComponent = UGameplayStatics::SpawnEmitterAttached(
			Tracer,
			ProjectileMesh,
			FName(),
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition
		);
	}
}

// �߻�ü�� �浿������ ȣ��Ǵ� �Լ�
void AProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	AFPSCharacter* Character = Cast<AFPSCharacter>(OtherActor);

	if (Character)
	{
		IsHitCharacter = true;
		IsHitMonster = false;
	}
	else if (Cast<AFPSMonster>(OtherActor))
	{
		IsHitCharacter = false;
		IsHitMonster = true;
	}

	MultiCast_Hit(IsHitCharacter, IsHitMonster);
	
	// �ı� ��û
	Destroy();
}

// ���� ��ü�� ���� ����Ʈ�� �Ҹ� ���
void AProjectile::MultiCast_Hit_Implementation(bool bHitCharacter, bool bHitMonster)
{
	if (bHitCharacter)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitCharacterParticle, GetActorTransform());
	}
	else if (!bHitCharacter && bHitMonster)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitMonsterParticle, GetActorTransform());
	}
	else if(!bHitCharacter && !bHitMonster)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitSomethingParticle, GetActorTransform());
	}

	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
}