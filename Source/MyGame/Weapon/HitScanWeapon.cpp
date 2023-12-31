
#include "HitScanWeapon.h"
#include "MyGame/Character/FPSCharacter.h"
#include "MyGame/Character/FPSMonster.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SkeletalMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetMathLibrary.h"

// �� �߻�� ȣ��Ǵ� �Լ�
void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;

	AController* InstigatorController = OwnerPawn->GetController();

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector Start = SocketTransform.GetLocation();

		FHitResult FireHit;
		WeaponTraceHit(Start, HitTarget, FireHit);

		UWorld* World = GetWorld();
		if (World)
		{		
			if (FireHit.bBlockingHit)
			{
				if (HasAuthority() && Cast<AFPSMonster>(FireHit.GetActor()))
				{
					// �Ӹ��� �����
					if (FireHit.BoneName.ToString() == FString("head"))
					{
						AActor* DamagedActor = FireHit.GetActor();
						if (DamagedActor)
						{
							UGameplayStatics::ApplyDamage(DamagedActor, HeadShotDamage, InstigatorController, this, UDamageType::StaticClass());
						}
					}
					// ������ ���� �����
					else
					{
						AActor* DamagedActor = FireHit.GetActor();
						if (DamagedActor)
						{
							UGameplayStatics::ApplyDamage(DamagedActor, Damage, InstigatorController, this, UDamageType::StaticClass());
						}
					}
				}
			}
		}
	}
}


// TraceEndWidthScatter() �� ���� ���� ���� ������ ������� LineTrace �� FHitResult �� OutHit �Ű������� ����
void AHitScanWeapon::WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit)
{
	UWorld* World = GetWorld();
	if (World)
	{            
		FVector End = TraceStart + (HitTarget - TraceStart) * 1.25f;

		World->LineTraceSingleByChannel(OutHit, TraceStart, End, ECollisionChannel::ECC_WorldDynamic);

		FVector BeamEnd = End;

		if (OutHit.bBlockingHit)
		{
			BeamEnd = OutHit.ImpactPoint;
		}
		else
		{
			OutHit.ImpactPoint = End;
		}

		if (Cast<AFPSCharacter>(OutHit.GetActor()))
		{
			if (ImpactHumanParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(World, ImpactHumanParticles, OutHit.ImpactPoint, OutHit.ImpactNormal.Rotation());
			}
		}
		else if (Cast<AFPSMonster>(OutHit.GetActor()))
		{
			if (ImpactMonsterParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(World, ImpactMonsterParticles, OutHit.ImpactPoint, OutHit.ImpactNormal.Rotation());
			}
		}
		else
		{
			if (ImpactSomethingParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(World, ImpactSomethingParticles, OutHit.ImpactPoint, OutHit.ImpactNormal.Rotation());
			}
		}

		if (BeamParticles)
		{
			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(World, BeamParticles, TraceStart, FRotator::ZeroRotator, false);
			if (Beam)
			{
				Beam->SetVectorParameter(FName("Target"), BeamEnd);
			}
		}
	}
}
