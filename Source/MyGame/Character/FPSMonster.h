// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/TimelineComponent.h"
#include "FPSMonster.generated.h"

UENUM(BlueprintType)
enum class MoveStateMonster : uint8 {
	EMS_Walk UMETA(DisplayName = "Walk"),
	EMS_Crouch UMETA(DisplayName = "Crouch"),
	EMS_Run UMETA(DisplayName = "Run")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLeftGames);

UCLASS()
class MYGAME_API AFPSMonster : public ACharacter
{
	GENERATED_BODY()

public:
	AFPSMonster();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	virtual void Jump() override;
	virtual void LaunchCharacter(FVector LaunchVelocity, bool bXYOverride, bool bZOverride) override;

	UFUNCTION()
	virtual void OnSphereOverlap(class UPrimitiveComponent* OverlappedComponent, class AActor* OtherActor,
			UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
			bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	virtual void OnSphereEndOverlap(class UPrimitiveComponent* OverlappedComponent, class AActor* OtherActor,
			UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCursor);

public:
	void MoveForward(float Value);
	void MoveRight(float Value);
	void LookUp(float Value);
	void LookRight(float Value);
	void DoCrouch();
	void Run();
	void StopRun();

	void Throw();
	UFUNCTION(BlueprintCallable)
	void Loop();
	UFUNCTION(BlueprintCallable)
	void ThrowDagger();
	void PlayThrowMontage();
	void ThrowCoolTimeEnd();
	void ChargeThrowEnd();
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Throw();
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ChargeThrowEnd();
	UFUNCTION(Server, Reliable)
	void Server_ThrowKeyPressedDuringCoolTime(bool bPress);
	FTimerHandle ThrowTimer;

	void ChangeToMorning();
	void ChangeToNight();


	void NormalAttack();
	void PlayNormalAttackMontage();
	void DissolveButtonPressed();
	void PollInit();
	void UpdateHUDHealth();

	void ManageStemina(float DeltaTime);
	void ManageDash(float DeltaTime);

	void ChargeDash();
	void StartDash();

	void ThermalVision();

	void EndDash();
	UFUNCTION(NetMulticast, Reliable)
	void MulticastDash();

	UFUNCTION(Server, Reliable)
	void Server_SpeedChange(MoveStateMonster EMS);


	UFUNCTION(BlueprintCallable)
	void NormalAttackEnd();
	UFUNCTION(BlueprintCallable)
	void NormalAttackFinish();
	UFUNCTION(Server, Reliable)
	void Server_Attack();


	UFUNCTION(Server, Reliable)
	void Server_DissolveButtonPressed(bool b);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_DissolveButtonPressed(bool b);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Elim(bool bPlayerLeftGame);
	void Elim(bool bPlayerLeftGame);
	//Dissolve 관련 함수들
	void EndDissolve();
	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);


	//FORCEINLINE bool GetIsRun() const { return IsRun; }
	FORCEINLINE bool GetIsInAir() const { return IsInAir; }
	FORCEINLINE bool GetIsDead() const { return IsDead; }
	FORCEINLINE bool GetIsDash() const { return IsDash; }
	FORCEINLINE bool GetIsDissovle() const { return IsDissolve; }
	FORCEINLINE bool GetAttackButtonPressed() const { return AttackButtonPressed; }
	//FORCEINLINE float GetRunSpeed() const { return RunSpeed; }
	//FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	//FORCEINLINE float GetHealth() const { return Health; }
	//FORCEINLINE float GetStemina() const { return Stemina; }
	//FORCEINLINE float GetMaxStemina() const { return MaxStemina; }
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE float GetWalkSpeed() const { return WalkSpeed; }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE float GetSpeed() const { return GetVelocity().Size(); }
	FORCEINLINE class UCameraComponent* GetCamera() const { return Camera; }

	UFUNCTION(Server, Reliable)
	void ServerLeftGame();

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	EPhysicalSurface GetSurfaceType();


private:
	UPROPERTY(EditAnywhere)
	TSubclassOf<class AActor> ThrowDaggerClass;

	UPROPERTY()
	const class USkeletalMeshSocket* LeftWeaponStart = nullptr;

	UPROPERTY()
	const class USkeletalMeshSocket* LeftWeaponEnd = nullptr;

	UPROPERTY()
	const class USkeletalMeshSocket* RightWeaponStart = nullptr;

	UPROPERTY()
	const class USkeletalMeshSocket* RightWeaponEnd = nullptr;

	UPROPERTY(EditAnywhere)
	class UPostProcessComponent* PostProcessComponent = nullptr;

	UPROPERTY(EditAnywhere, Category = Combat)
	class USoundCue* IdleSoundCue = nullptr;
	UPROPERTY(EditAnywhere, Category = Combat)
	class USoundCue* NormalAttackSuccessSound = nullptr;
	UPROPERTY(EditAnywhere, Category = Combat)
	class USoundCue* NormalAttackFailSound = nullptr;
	UPROPERTY(EditAnywhere, Category = Combat)
	class USoundCue* StartSound = nullptr;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	class UCameraComponent* Camera = nullptr;
	UPROPERTY(VisibleAnywhere, Category = Camera)
	class USpringArmComponent* CameraBoom = nullptr;

	UPROPERTY(EditAnywhere)
	class UMonsterCombatComponent* MonsterCombat = nullptr;

	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* AttackMontage = nullptr;

	UPROPERTY(EditAnywhere, Category = Combat)
	class USphereComponent* AreaSphere = nullptr;

	UPROPERTY()
	class ACharacterController* PlayerController = nullptr;

	UPROPERTY()
	class ACharacterState* CharacterPlayerState = nullptr;

private:
	UPROPERTY(Replicated)
	bool IsDissolve = false;
	bool IsInAir = false;
	bool IsRun = false;
	bool IsDash = false;
	bool FireButtonPressed = false;
	bool IsDead = false;
	bool bLeftGame = false;
	bool CanAttack = true;
	UPROPERTY(ReplicatedUsing = OnRep_AttackButtonPressed)
	bool AttackButtonPressed = false;
	bool AttackFinished = false;
	UPROPERTY(Replicated)
	bool bChargeDash = false;
	bool bThermalVision = false;
	UPROPERTY(EditAnywhere)
	bool ActiveFootIK = false;
	//UPROPERTY(Replicated)
	bool IsThrowCoolTime = false;
	UPROPERTY(Replicated)
	bool ThrowKeyPressedDuringCoolTime = false;
	bool ThrowButtonPressed = false;

	UPROPERTY(EditAnywhere, Category = Combat)
	float Health = 100.f;
	UPROPERTY(EditAnywhere, Category = Combat)
	float MaxHealth = 100.f;


	UPROPERTY(EditAnywhere, Category = Combat)
	float Stemina = 100.f;
	UPROPERTY(EditAnywhere, Replicated, Category = Combat)
	float MaxStemina = 100.f;


	UPROPERTY(EditAnywhere, Replicated, Category = Combat)
	float Dash = 100.f;
	UPROPERTY(EditAnywhere, Category = Combat)
	float MaxDash = 100.f;
	UPROPERTY(Replicated)
	float ChargedDash = 0.f;
	UPROPERTY(EditAnywhere)
	float IncreaseDash = 20.f;
	UPROPERTY(EditAnywhere)
	float DecreaseDash = 70.f;

	int32 AirDashCount = 0;

	UPROPERTY(EditAnywhere, Category = CoolTime)
	float ThrowCoolTime = 5.f;
	float ThrowCoolTimeCopy = 5.f;
	float ThrowCoolTimePercentage = 100.f;


	UPROPERTY(EditAnywhere, Category = Combat)
	float WalkSpeed = 450.f;
	UPROPERTY(EditAnywhere, Category = Combat)
	float RunSpeed = 700.f;
	UPROPERTY(EditAnywhere, Category = Combat)
	float CrouchSpeed = 300.f;
	UPROPERTY(EditAnywhere, Category = Combat)
	float CurrentSpeed = 0.f;
	UPROPERTY(EditAnywhere, Category = Combat)
	float SlowSpeed = 300.f;
	UPROPERTY(EditAnywhere, Category = Combat)
	float SlowSpeedReduce = 13.f;
	UPROPERTY(EditAnywhere, Category = Combat)
	float MorningSlowSpeed = 100.f;
	float CurrentMorningSlowSpeed = 0.f;
	float CurrentSlowSpeed = 0.f;


	UPROPERTY(EditAnywhere, Category = Combat)
	float NormalDamage = 30.f;
	UPROPERTY(EditAnywhere, Category = Combat)
	float MorningNormalDamage = 20.f;
	float CurrentMorningNormalDamge = 0.f;

	float AO_Pitch;

	FRotator StartingAimRotation;

	FTimerHandle NormalAttackTimer;
	FTimerHandle ElimTimer;
	FTimerHandle DashTimer;

	FHitResult HitResult;
	FHitResult DashResult;

	MoveStateMonster CurrentMoveState;

	// Dissolve 관련 변수들
	UPROPERTY(EditAnywhere, Category = Dissolve)
	class UTimelineComponent* DissolveTimeline = nullptr;

	FOnTimelineFloat DissolveTrack;

	UPROPERTY(EditAnywhere)
	UCurveFloat* DissolveCurve = nullptr;

	UPROPERTY()
	class UMaterialInstanceDynamic* DynamicDissolveMaterialInstance1 = nullptr;
	UPROPERTY(EditAnywhere)
	class UMaterialInstance* DissolveMaterialInstance1 = nullptr;

	UPROPERTY()
	class UMaterialInstanceDynamic* DynamicDissolveMaterialInstance2 = nullptr;
	UPROPERTY(EditAnywhere)
	class UMaterialInstance* DissolveMaterialInstance2 = nullptr;

	UPROPERTY()
	class UMaterialInstanceDynamic* DynamicDissolveMaterialInstance3 = nullptr;
	UPROPERTY(EditAnywhere)
	class UMaterialInstance* DissolveMaterialInstance3 = nullptr;

	UPROPERTY()
	class UMaterialInstanceDynamic* DynamicDissolveMaterialInstance4 = nullptr;
	UPROPERTY(EditAnywhere)
	class UMaterialInstance* DissolveMaterialInstance4 = nullptr;

	UPROPERTY()
	class UMaterialInstanceDynamic* DynamicDissolveMaterialInstance5 = nullptr;
	UPROPERTY(EditAnywhere)
	class UMaterialInstance* DissolveMaterialInstance5 = nullptr;

	UPROPERTY()
	class UMaterialInstanceDynamic* DynamicDissolveMaterialInstance6 = nullptr;
	UPROPERTY(EditAnywhere)
	class UMaterialInstance* DissolveMaterialInstance6 = nullptr;

	UPROPERTY()
	class UMaterialInstanceDynamic* DynamicDissolveMaterialInstance7 = nullptr;
	UPROPERTY(EditAnywhere)
	class UMaterialInstance* DissolveMaterialInstance7 = nullptr;

	UPROPERTY()
	class UMaterialInstanceDynamic* DynamicDissolveMaterialInstance8 = nullptr;
	UPROPERTY(EditAnywhere)
	class UMaterialInstance* DissolveMaterialInstance8 = nullptr;

	UPROPERTY()
	class UMaterialInstanceDynamic* DynamicDissolveMaterialInstance9 = nullptr;
	UPROPERTY(EditAnywhere)
	class UMaterialInstance* DissolveMaterialInstance9 = nullptr;

	UPROPERTY()
	class UMaterialInstanceDynamic* DynamicDissolveMaterialInstance10 = nullptr;
	UPROPERTY(EditAnywhere)
	class UMaterialInstance* DissolveMaterialInstance10 = nullptr;

	UPROPERTY()
	class UMaterialInstanceDynamic* DynamicDissolveMaterialInstance11 = nullptr;
	UPROPERTY(EditAnywhere)
	class UMaterialInstance* DissolveMaterialInstance11 = nullptr;

public:

	UFUNCTION()
	void OnRep_AttackButtonPressed();

	FOnLeftGames OnLeftGames;
};
