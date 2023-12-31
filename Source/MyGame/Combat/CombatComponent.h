// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MyGame/WeaponState.h"
#include "MyGame/HUD/CharacterHUD.h"
#include "MyGame/Weapon/WeaponType.h"
#include "CombatState.h"
#include "CombatComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MYGAME_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();
	friend class AFPSCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

public:	
	void EquipWeapon(class AWeapon* Weapon);
	UFUNCTION()
	void OnRep_EquippedWeapon();


	void FireShotgun();
	void FireHitScanWeapon();
	void FireProjectileHitWeapon();
	void Fire(bool bFire);
	void FireStart();
	void FireEnd();


	void SetAiming(bool bIsAiming);
	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);
	void TraceUnderCrosshair(FHitResult & TraceHitResult);
	void SetCrosshairHUD(float DeltaTime);
	void InterpCameraFOV(float DeltaTime);


	UFUNCTION(BlueprintCallable)
	void shotgunShellReload();
	UFUNCTION(BlueprintCallable)
	void shotgunShellReloadEnd();
	void Reload();
	int32 AmountToReload();
	void UpdateAmmoValue();
	void UpdateShotgunAmmoValue();
	UFUNCTION(BlueprintCallable)
	void EndReload();
	UFUNCTION(BlueprintCallable)
	void ShotgunEndReload();
	UFUNCTION(Server, Reliable)
	void ServerReload();


	bool CanFire();
	void LocalFire(const FVector_NetQuantize& TraceHitTarget);
	void LocalShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets);
	UFUNCTION(Server, Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);
	UFUNCTION(NetMulticast, Reliable)
	void MultiCastFire(const FVector_NetQuantize& TraceHitTarget);
	UFUNCTION(Server, Reliable)
	void ServerShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets);
	UFUNCTION(NetMulticast, Reliable)
	void MulticastShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets);
	void JumpToShotgfunEnd();
		
private:
	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	class AWeapon* EquippedWeapon = nullptr;

	class AFPSCharacter* Character = nullptr;

	UPROPERTY()
	class ACharacterController* Controller = nullptr;

	UPROPERTY()
	class ACharacterHUD* HUD = nullptr;

	UPROPERTY(EditAnywhere)
	float ZoomedFOV = 30.f;

	UPROPERTY(EditAnywhere)
	float ZoomInterpSpeed = 20.f;

	float VelocityFactor;
	float AimFactor;
	float AirFactor;
	float DefaultFOV;
	float CurrentFOV;

	UPROPERTY(EditAnywhere, Category = Combat)
	float AimWalkSpeed = 150.f;


	bool FireButtonPressed = false;
	bool IsEnemyAimed = false;
	bool CanShoot = true;
	bool IsReloading = false;
	bool bAimButtonPressed = false;
	bool bLocallyReloading = false;
	UPROPERTY(Replicated)
	bool bAiming = false;

	TMap<EWeaponType, int32> CarriedAmmo;

	UPROPERTY(EditAnywhere)
	int32 CurrentCarriedWeaponAmmo = 90;

	UPROPERTY(EditAnywhere)
	int32 CurrentCarriedRocketAmmo = 0;

	UPROPERTY(EditAnywhere)
	int32 CurrentCarriedGunAmmo = 50;

	UPROPERTY(EditAnywhere)
	int32 CurrentCarriedShotGunAmmo = 16;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentCarriedAmmo)
	int32 CurrentCarriedAmmo;

	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState;

public:
	UFUNCTION()
	void OnRep_CombatState();

	UFUNCTION()
	void OnRep_CurrentCarriedAmmo();

public:
	FTimerHandle FireTimer;
	FTimerHandle ReloadTimer;
	FHitResult HitResult;
	FVector HitTarget = FVector::ZeroVector;
	FHUDPackage HUDPackage;

	FORCEINLINE void SetFireButtonPressed(bool bPressed) { FireButtonPressed = bPressed; }
	FORCEINLINE AWeapon* GetEquippedWeapon() const { return EquippedWeapon; }
	FORCEINLINE ECombatState GetCurrentCombatState() const { return CombatState; }
	FORCEINLINE bool GetIsReloading() const { return IsReloading; }
	FORCEINLINE bool GetbAiming() const { return bAiming; }
};
