// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGame/Weapon/Dagger.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/BoxComponent.h"
#include "MyGame/Character/FPSCharacter.h"
#include "MyGame/DamageType/SlowEffectDamage.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

ADagger::ADagger()
{
	PrimaryActorTick.bCanEverTick = true;

	CollisionArea = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionArea"));
	RootComponent = CollisionArea;

	ThorwKnifeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ThrowKnifeMesh"));
	ThorwKnifeMesh->SetupAttachment(RootComponent);
	ThorwKnifeMesh->SetVisibility(true);
	
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("DaggerMovementComponent"));
	ProjectileMovementComponent->InitialSpeed = 3500.f;
	ProjectileMovementComponent->MaxSpeed = 3500.f;
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->ProjectileGravityScale = 1.f;
	ProjectileMovementComponent->SetIsReplicated(true);
}
 
// ������ ȣ��Ǵ� �Լ�
void ADagger::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Error, TEXT("Begin"));
	if (Tracer)
	{
		TraceParticleComponent = UGameplayStatics::SpawnEmitterAttached(
			Tracer,
			ThorwKnifeMesh,
			FName(),
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition
		);
		TraceParticleComponent->SetVisibility(false);
	}
	
	// �浹�� ȣ��� �Լ� ���ε�
	if (HasAuthority())
	{
		CollisionArea->OnComponentHit.AddDynamic(this, &ADagger::OnHit);
	}
}

void ADagger::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (Tracer)
	{
		TraceParticleComponent = UGameplayStatics::SpawnEmitterAttached(
			Tracer,
			ThorwKnifeMesh,
			FName(),
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition
		);
	}
}

// �浹�� ȣ��Ǵ� �Լ�
void ADagger::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	AFPSCharacter* Character = Cast<AFPSCharacter>(OtherActor);
	AController* OwnerController = OwnerCharacter->Controller;

	// ���� ����� �ΰ��̸� ������ �ֱ�
	if (Character && OwnerCharacter && OwnerController)
	{
		IsHitCharacter = true;
		UGameplayStatics::ApplyDamage(OtherActor, DaggerDamage, OwnerController, this, SlowEffectDamageClass);
	}
	else
	{
		IsHitCharacter = false;
	}

	MultiCast_Hit(IsHitCharacter);

	Destroy();
}
// ��ü�� ���� ����Ʈ �Ҹ� ���
void ADagger::MultiCast_Hit_Implementation(bool bHitCharacter)
{
	if (IsHitCharacter && HitCharacterParticle)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitCharacterParticle, GetActorTransform());
	}
	else if(HitSomethingParticle)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitSomethingParticle, GetActorTransform());
	}

	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
}