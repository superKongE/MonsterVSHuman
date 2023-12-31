// Fill out your copyright notice in the Description page of Project Settings.


#include "RifleWeapon.h"
#include "MyGame/Bullet/Rifle.h"
#include "Net/UnrealNetwork.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/KismetMathLibrary.h"

// ��Ƽ�ɽ�Ʈ�� ���� ȣ������� ���������� ����ǰ� �Ѵ�
//                             �浹 ���� ����
void ARifleWeapon::Fire(const FVector& HitTarget)
{
	// �� �ִϸ��̼� �� �Ѿ� ����
	Super::Fire(HitTarget);

	// �����϶��� �Ѿ� �߻�
	if (!HasAuthority()) return;

	APawn* InstigatorPawn = Cast<APawn>(GetOwner());

	FVector Velocity = InstigatorPawn->GetVelocity();
	Velocity.Z = 0.f;
	float Speed = Velocity.Size();

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	if (MuzzleFlashSocket)
	{
		// �ѱ��� ���Ϳ� ���� ����
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		// �浹������ �ѱ����� ���� ����
		FVector ToTarget = HitTarget - SocketTransform.GetLocation();
		// �浹���� ����
		FRotator TargetRotation = ToTarget.Rotation();

		// �� �ݰ�ȿ��� ������ ���� ����
		FVector TraceStart = SocketTransform.GetLocation();
		FVector ToTargetNormalized = (HitTarget - TraceStart); // Ÿ�� ���� (���� ����)
		FVector SphereCenter = TraceStart + ToTargetNormalized;  // ���� �߽�
		FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius); // �� ���� ������ ���� ��ġ(���� ����)
		FVector EndLoc = SphereCenter + RandVec; // �� ������ ��� ��ġ
		FVector ToEndLoc = (EndLoc - TraceStart); // �� ������ ��� ��ġ�� ���� ����

		if (ProjectileClass && InstigatorPawn)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = GetOwner();
			SpawnParams.Instigator = InstigatorPawn;
			UWorld* World = GetWorld();		

			if (World)
			{
				// �� �Լ��� ���������� ���������
				// ABullet Ŭ������ Replicate �����Ǿ� �ֱ� ������ 
				// �ٸ� Ŭ���̾�Ʈ������ �Ѿ��� ���󰡴°��� ���δ�
				if (Speed < 400.f)
				{
					World->SpawnActor<ARifle>(
						ProjectileClass,
						TraceStart,
						TargetRotation,
						SpawnParams
						);
				}
				else
				{
					World->SpawnActor<ARifle>(
						ProjectileClass,
						TraceStart,
						ToEndLoc.Rotation(),
						SpawnParams
						);
				}
			}
		}
	}
}