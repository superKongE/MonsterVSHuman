

#include "RocketWeapon.h"
#include "MyGame/Bullet/Rocket.h"
#include "Net/UnrealNetwork.h"
#include "Engine/SkeletalMeshSocket.h"

// 멀티케스트를 통해 호출되지만 서버에서만 실행되게 한다
//                             충돌 지점 정보
void ARocketWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	// 서버일때만
	if (!HasAuthority()) return;

	APawn* InstigatorPawn = Cast<APawn>(GetOwner());

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));

	if (MuzzleFlashSocket)
	{
		// 총구의 벡터와 방향 정보
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());

		// 충돌지점과 총구간의 벡터 정보
		FVector ToTarget = HitTarget - SocketTransform.GetLocation();
		// 충돌지점 방향
		FRotator TargetRotation = ToTarget.Rotation();

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
				World->SpawnActor<ARocket>(
					ProjectileClass,
					SocketTransform.GetLocation(),
					TargetRotation,
					SpawnParams
					);
			}
		}
	}
}