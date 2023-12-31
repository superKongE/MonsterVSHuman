// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyGame/WeaponState.h"
#include "MyGame/Weapon/WeaponType.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	WS_Equipped UMETA(DisplayName = "Equipped"),
    WS_Dropped UMETA(DisplayName = "Dropped"),

	WS_MAX UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EFireType : uint8
{
	EFT_HitScan UMETA(DisplayName = "Hit Scan Weapon"),
	EFT_Projectile UMETA(DisplayName = "ProjectileHit Weapon"),
	EFT_Shotgun UMETA(DisplayName = "Shotgun Weapon"),

	EFT_MAX UMETA(DisplayName = "DefaultMAX")
};

UCLASS()
class MYGAME_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	AWeapon();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_Owner() override;

	UFUNCTION()
	virtual void OnSphereOverlap(class UPrimitiveComponent* OverlappedComponent, class AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void OnSphereEndOverlap(class UPrimitiveComponent* OverlappedComponent, class AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	FVector TraceEndWidthScatter(const FVector& HitTarget);

protected:
	// 랜덤 발사 관련 변수들
	UPROPERTY(EditAnywhere)
	float DistanceToSphere = 800.f;
	UPROPERTY(EditAnywhere)
	float SphereRadius = 75.f;
	UPROPERTY(EditAnywhere)
	bool bUseScatter = false;
	UPROPERTY(EditAnywhere)
	float Damage = 20.f;
	UPROPERTY(EditAnywhere)
	float HeadShotDamage = 100.f;

public:	
	FORCEINLINE float GetFireDelay() const { return FireDelay; }
	FORCEINLINE float GetZoomedFOV() const  { return ZoomedFOV; }
	FORCEINLINE float GetZoomInterpSpeed() const { return ZoomInterpSpeed; }
	FORCEINLINE float GetReloadDelay() const { return ReloadDelay; }
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	FORCEINLINE int32 GetMagCapcity() const { return MagCapacity; }
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	FORCEINLINE EFireType GetFireType() const { return FireType; }
	FORCEINLINE class USoundCue* GetEquippedSoundCue() const { return EquippedSoundCue; }
	FORCEINLINE bool IsAutomaticShoot() const { return AutomaticShoot; }

	FORCEINLINE void SetbUseScatter(bool bScatter) { bUseScatter = bScatter; }
	FORCEINLINE bool GetUserScatter() const { return bUseScatter; }

	void ShowPickUpWidget(bool show);
	void SetWeaponState(EWeaponState State);
	void DropWeapon();
	void SetHUDAmmo();
	void AddAmmo(int32 ReloadAmount);
	void SpendAmmo();
	bool IsFull();
	virtual void Fire(const FVector& HitTarget);

	UFUNCTION(Client, Reliable)
	void ClientUpdateAmmo(int32 ServerAmmo);
	UFUNCTION(Client, Reliable)
	void ClientAddAmmo(int32 AmmoToAdd);

private:
	class AFPSCharacter* Character = nullptr;
	class ACharacterController* CharacterController = nullptr;

	UPROPERTY(EditAnywhere)
	class USoundCue* EquippedSoundCue = nullptr;

	UPROPERTY(EditAnywhere, Category = Mesh)
	class USkeletalMeshComponent* WeaponMesh = nullptr;

	UPROPERTY(VisibleAnywhere, Category = Capsule)
	class USphereComponent* AreaSphere = nullptr;

	UPROPERTY(VisibleAnywhere, Category = Widget)
	class UWidgetComponent* PickUpWidget = nullptr;

	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_SetWeaponState)
	EWeaponState WeaponState;

	UPROPERTY(EditAnywhere)
	class UAnimationAsset* FireAnimation = nullptr;

	UPROPERTY(EditAnywhere, Category = Combat)
	float FireDelay = 0.23f;

	UPROPERTY(EditAnywhere, Category = Combat)
	float ReloadDelay = 2.f;

	UPROPERTY(EditAnywhere)
	float ZoomedFOV = 50.f;

	UPROPERTY(EditAnywhere)
	float ZoomInterpSpeed = 20.f;

	UPROPERTY(EditAnywhere)
	bool AutomaticShoot = true;

	//UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_Ammo)
	//int32 Ammo = 45;
	UPROPERTY(EditAnywhere)
	int32 Ammo = 45;	    // 현재 총알
	UPROPERTY(EditAnywhere)
	int32 MagCapacity = 45; // 탄창에 총알

	// Ammo 에 대해 처리되지 않은 서버 요청 수
	int32 Sequence = 0;

	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType;
	UPROPERTY(EditAnywhere)
	EFireType FireType;

public:
	UPROPERTY(EditAnywhere, Category = Crosshair)
	class UTexture2D* CrosshairCeneter = nullptr;

	UPROPERTY(EditAnywhere, Category = Crosshair)
	class UTexture2D* CrosshairTop = nullptr;

	UPROPERTY(EditAnywhere, Category = Crosshair)
	class UTexture2D* CrosshairBottom = nullptr;

	UPROPERTY(EditAnywhere, Category = Crosshair)
	class UTexture2D* CrosshairRight = nullptr;

	UPROPERTY(EditAnywhere, Category = Crosshair)
	class UTexture2D* CrosshairLeft = nullptr;


public:
	UFUNCTION()
	void OnRep_SetWeaponState();

	UFUNCTION()
	void OnRep_Ammo();
};
