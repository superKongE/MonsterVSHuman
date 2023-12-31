

#include "RocketWeapon.h"
#include "MyGame/Bullet/Rocket.h"
#include "Net/UnrealNetwork.h"
#include "Engine/SkeletalMeshSocket.h"

// ��Ƽ�ɽ�Ʈ�� ���� ȣ������� ���������� ����ǰ� �Ѵ�
//                             �浹 ���� ����
void ARocketWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);

	// �����϶���
	if (!HasAuthority()) return;

	APawn* InstigatorPawn = Cast<APawn>(GetOwner());

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));

	if (MuzzleFlashSocket)
	{
		// �ѱ��� ���Ϳ� ���� ����
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());

		// �浹������ �ѱ����� ���� ����
		FVector ToTarget = HitTarget - SocketTransform.GetLocation();
		// �浹���� ����
		FRotator TargetRotation = ToTarget.Rotation();

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