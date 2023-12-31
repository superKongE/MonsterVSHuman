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
 
// 생성시 호출되는 함수
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
	
	// 충돌시 호출될 함수 바인딩
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

// 충돌시 호출되는 함수
void ADagger::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	AFPSCharacter* Character = Cast<AFPSCharacter>(OtherActor);
	AController* OwnerController = OwnerCharacter->Controller;

	// 맞은 대상이 인간이면 데미지 주기
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
// 물체에 맞은 이펙트 소리 재생
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