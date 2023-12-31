// Fill out your copyright notice in the Description page of Project Settings.


#include "ShotgunWeapon.h"
#include "MyGame/Character/FPSCharacter.h"
#include "MyGame/Character/FPSMonster.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SkeletalMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Sound/SoundCue.h"

void AShotgunWeapon::FireShotgun(const TArray<FVector_NetQuantize>& HitTargets)
{
	AWeapon::Fire(FVector());

	UWorld* World = GetWorld();
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	AController* InstigatorController = OwnerPawn->GetController();

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (World && MuzzleFlashSocket)
	{
		const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		const FVector Start = SocketTransform.GetLocation();

		// 샷건의 경우 한번에 여러발을 쏘므로
		// 한발마다 어디를 맞췄는지 정보를 저장하기 위한 Map 자료구조
		TMap<ACharacter*, uint32> HitMap;
		for (FVector_NetQuantize HitTarget : HitTargets)
		{
			FHitResult FireHit;
			// LineTrace한 결과를 Out 매개변수로 제공받음
			WeaponTraceHit(Start, HitTarget, FireHit);
			
			AFPSMonster* Monster = Cast<AFPSMonster>(FireHit.GetActor());
			if (Monster)
			{
				// 머리를 맞춘경우
				if (FireHit.BoneName.ToString() == FString("head"))
				{
					if (HitMap.Contains(Monster)) HitMap[Monster] += HeadShotDamage;
					else HitMap.Emplace(Monster, Damage);
				}
				else
				{
					if (HitMap.Contains(Monster)) HitMap[Monster] += Damage;
					else HitMap.Emplace(Monster, Damage);
				}

				if (Cast<AFPSCharacter>(FireHit.GetActor()))
				{
					if (ImpactHumanParticles)
					{
						UGameplayStatics::SpawnEmitterAtLocation(World, ImpactHumanParticles, FireHit.ImpactPoint, FireHit.ImpactNormal.Rotation());
					}
				}
				else if (Cast<AFPSMonster>(FireHit.GetActor()))
				{
					if (ImpactMonsterParticles)
					{
						UGameplayStatics::SpawnEmitterAtLocation(World, ImpactMonsterParticles, FireHit.ImpactPoint, FireHit.ImpactNormal.Rotation());
					}
				}
				else
				{
					if (ImpactSomethingParticles)
					{
						UGameplayStatics::SpawnEmitterAtLocation(World, ImpactSomethingParticles, FireHit.ImpactPoint, FireHit.ImpactNormal.Rotation());
					}
				}

				if (BeamParticles)
				{
					UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticles, Start, FRotator::ZeroRotator, false);
					if (Beam)
					{
						Beam->SetVectorParameter(FName("Target"), HitTarget);
					}
				}

				if (ImpactSound)
				{
					UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, FireHit.ImpactPoint);
				}
			}

			for (const auto &HitPair : HitMap)
			{
				if (HitPair.Key && HasAuthority() && InstigatorController)
				{
					UGameplayStatics::ApplyDamage(HitPair.Key, HitPair.Value, InstigatorController, this, UDamageType::StaticClass());
				}
			}
		}
	}
}


// 임의의 원을 만들어서 해당 원내에서 랜덤한 좌표를 구해 해당 방향으로 LineTrace를 수행한 결과를 Out 매개변수로 전달
void AShotgunWeapon::ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets)
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket == nullptr) return;

	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();

	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal(); // 타겟 방향 (유닛 벡터)
	const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;  // 원의 중심

	for (int32 i = 0; i < 8; i++)
	{
		const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius); // 원 범위 내에서 랜덤 위치(유닛 벡터)
		const FVector EndLoc = SphereCenter + RandVec; // 원 내부의 어느 위치
		FVector ToEndLoc = (EndLoc - TraceStart); // 원 내부의 어느 위치의 방향 벡터
		ToEndLoc = TraceStart + ToEndLoc * 8000.f / ToEndLoc.Size();

		HitTargets.Add(ToEndLoc);
	}
}
void AShotgunWeapon::ShotgunTraceEndWithScatter_Zoomed(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets)
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket == nullptr) return;

	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();

	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal(); 
	const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere; 

	for (int32 i = 0; i < 8; i++)
	{
		const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, ZoomedSphereRadius);
		const FVector EndLoc = SphereCenter + RandVec; 
		FVector ToEndLoc = (EndLoc - TraceStart);
		ToEndLoc = TraceStart + ToEndLoc * 8000.f / ToEndLoc.Size();

		HitTargets.Add(ToEndLoc);
	}
}