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

		// ������ ��� �ѹ��� �������� ��Ƿ�
		// �ѹ߸��� ��� ������� ������ �����ϱ� ���� Map �ڷᱸ��
		TMap<ACharacter*, uint32> HitMap;
		for (FVector_NetQuantize HitTarget : HitTargets)
		{
			FHitResult FireHit;
			// LineTrace�� ����� Out �Ű������� ��������
			WeaponTraceHit(Start, HitTarget, FireHit);
			
			AFPSMonster* Monster = Cast<AFPSMonster>(FireHit.GetActor());
			if (Monster)
			{
				// �Ӹ��� ������
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


// ������ ���� ���� �ش� �������� ������ ��ǥ�� ���� �ش� �������� LineTrace�� ������ ����� Out �Ű������� ����
void AShotgunWeapon::ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets)
{
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket == nullptr) return;

	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();

	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal(); // Ÿ�� ���� (���� ����)
	const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;  // ���� �߽�

	for (int32 i = 0; i < 8; i++)
	{
		const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius); // �� ���� ������ ���� ��ġ(���� ����)
		const FVector EndLoc = SphereCenter + RandVec; // �� ������ ��� ��ġ
		FVector ToEndLoc = (EndLoc - TraceStart); // �� ������ ��� ��ġ�� ���� ����
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