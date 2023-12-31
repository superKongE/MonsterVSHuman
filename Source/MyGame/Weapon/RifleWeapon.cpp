// Fill out your copyright notice in the Description page of Project Settings.


#include "RifleWeapon.h"
#include "MyGame/Bullet/Rifle.h"
#include "Net/UnrealNetwork.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/KismetMathLibrary.h"

// 멀티케스트를 통해 호출되지만 서버에서만 실행되게 한다
//                             충돌 지점 정보
void ARifleWeapon::Fire(const FVector& HitTarget)
{
	// 총 애니메이션 및 총알 감소
	Super::Fire(HitTarget);

	// 서버일때만 총알 발사
	if (!HasAuthority()) return;

	APawn* InstigatorPawn = Cast<APawn>(GetOwner());

	FVector Velocity = InstigatorPawn->GetVelocity();
	Velocity.Z = 0.f;
	float Speed = Velocity.Size();

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	if (MuzzleFlashSocket)
	{
		// 총구의 벡터와 방향 정보
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		// 충돌지점과 총구간의 벡터 정보
		FVector ToTarget = HitTarget - SocketTransform.GetLocation();
		// 충돌지점 방향
		FRotator TargetRotation = ToTarget.Rotation();

		// 구 반경안에서 랜덤한 벡터 생성
		FVector TraceStart = SocketTransform.GetLocation();
		FVector ToTargetNormalized = (HitTarget - TraceStart); // 타겟 방향 (유닛 벡터)
		FVector SphereCenter = TraceStart + ToTargetNormalized;  // 원의 중심
		FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius); // 원 범위 내에서 랜덤 위치(유닛 벡터)
		FVector EndLoc = SphereCenter + RandVec; // 원 내부의 어느 위치
		FVector ToEndLoc = (EndLoc - TraceStart); // 원 내부의 어느 위치의 방향 벡터

		if (ProjectileClass && InstigatorPawn)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = GetOwner();
			SpawnParams.Instigator = InstigatorPawn;
			UWorld* World = GetWorld();		

			if (World)
			{
				// 이 함수는 서버에서만 실행되지만
				// ABullet 클래스는 Replicate 설정되어 있기 때문에 
				// 다른 클라이언트에서도 총알이 날라가는것이 보인다
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