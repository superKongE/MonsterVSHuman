// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/TimelineComponent.h"
#include "MyGame/Weapon/WeaponType.h"
#include "UObject/ConstructorHelpers.h"
#include "FPSCharacter.generated.h"

UENUM(BlueprintType)
enum class MoveStateHuman : uint8 {
	EMS_Walk UMETA(DisplayName = "Walk"),
	EMS_Crouch UMETA(DisplayName = "Crouch"),
	EMS_Run UMETA(DisplayName = "Run"),
	EMS_Aim UMETA(DisplayName = "Aim"),
	EMS_Slow UMETA(DisplayName = "Slow")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLeftGame);

UCLASS()
class MYGAME_API AFPSCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AFPSCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	virtual void Jump() override;
	virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;

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
	void Aimming();
	void StopAimming();
	void EquipWeapon();
	void Drop();
	void Fire();
	void FireEnd();
	void PlayFireAnimation();
	void Reload();
	void PlayReloadAnimation();
	void PollInit();
	void UpdateHUDHealth();
	void SetOverlappingWeapon(AWeapon* OverlappingWeapon);
	void OverlappingGenerator(class AGenerator* Generator);
	void ManageStemina(float DeltaTime);
	void StartSpotlight();
	void OpenMap();
	void CloseMap();

	UFUNCTION(Server, Reliable)
	void Server_SpeedChange(MoveStateHuman EMS);

	AWeapon* GetEquippedWeapon();

	void SlowSpeedTimerEnd();

	void Charge();
	void StopCharge();

	void PlayIdleSound();
	void PlayChasingSound();
	UFUNCTION(Client, Reliable)
	void Client_PlayIdleSound();
	UFUNCTION(Client, Reliable)
	void Client_ChasingSound();

	void RecoveryHealth(float RecoverHelath);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Elim(bool bPlayerLeftGame);
	void Elim(bool bPlayerLeftGame);
	void EndElime();

	UFUNCTION(Client, Reliable)
	void Client_GetSlowed(bool bSlow);

	FORCEINLINE bool GetIsRun() const  { return IsRun; }
	FORCEINLINE bool GetIsAimming() const { return IsAimming; }
	FORCEINLINE bool GetIsInAir() const { return IsInAir; }
	FORCEINLINE bool GetIsDead() const { return IsDead; }
	FORCEINLINE float GetRunSpeed() const { return RunSpeed; }
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	FORCEINLINE float GetHealth() const { return Health; }
	FORCEINLINE float GetStemina() const { return Stemina; }
	FORCEINLINE float GetMaxStemina() const { return MaxStemina; }
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }
	FORCEINLINE MoveStateHuman GetMoveStateHuman() { return CurrentMoveState; }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE float GetWalkSpeed() const { return WalkSpeed; }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE float GetSpeed() const { return GetVelocity().Size(); }
	FORCEINLINE class UCameraComponent* GetCamera() const { return Camera; }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE class UCombatComponent* GetCombatComponent() const { return CombatComponent; }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE class AGenerator* GetOverlappedGenerator() const { return OverlappedGenerator; }
	UFUNCTION(BlueprintCallable)
	FORCEINLINE class UChatComponent* GetChatComponent() const { return ChatComponent; }
	void SetCurrentWeaponType(EWeaponType WeaponType) { CurrentWeaponType = WeaponType;}
	FORCEINLINE EWeaponType GetCurrentWeaponType() const { return CurrentWeaponType; }
	bool GetIsEquippedWeapon();
	FORCEINLINE UAnimMontage* GetReloadMontage() const { return FireMontage; }
	FORCEINLINE class ACharacterController* GetPlayerController() const { return PlayerController; }


	UFUNCTION(Server, Reliable)
	void ServerEquipped();
	UFUNCTION(Server, Reliable)
	void ServerDropped();
	UFUNCTION(Server, Reliable)
	void ServerLeftGame();
	UFUNCTION(Server, Reliable)
	void ServerSpotlight(bool bLight);


	UFUNCTION(NetMulticast, Reliable)
	void MulticastSpotlight(bool bLight);


	UFUNCTION(Server, Reliable)
	void Server_SetCanCharge(bool bCanCharge);

	UFUNCTION(Server, Reliable)
	void Server_SetSkeletalMesh(int num);

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	EPhysicalSurface GetSurfaceType();

private:
	UPROPERTY()
	class UHumanGameInstance* HumanGameInstance = nullptr;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	class UCameraComponent* Camera = nullptr;

	UPROPERTY(VisibleAnywhere, Category = Camera)
	class USpringArmComponent* CameraBoom = nullptr;

	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* FireMontage = nullptr;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon* OverlappedWeapon = nullptr;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingGenerator)
	class AGenerator* OverlappedGenerator = nullptr;

	UPROPERTY(VisibleAnywhere)
	class UCombatComponent* CombatComponent = nullptr;

	UPROPERTY(VisibleAnywhere)
	class UChatComponent* ChatComponent = nullptr;

	UPROPERTY(EditAnywhere)
	class USpotLightComponent* Spotlight = nullptr;

	UPROPERTY(EditAnywhere, Category = Sound)
	class USoundCue* IdleSoundCue = nullptr;
	UPROPERTY(EditAnywhere, Category = Sound)
	class USoundCue* HumanChasingCue = nullptr;
	UPROPERTY()
	class UAudioComponent* IdleAudio = nullptr;
	UPROPERTY()
	class UAudioComponent* ChasingAudio = nullptr;

	UPROPERTY()
	class ACharacterController* PlayerController = nullptr;
	UPROPERTY()
	class ACharacterState* CharacterPlayerState = nullptr;

	EWeaponType CurrentWeaponType = EWeaponType::ET_Knife;
	UPROPERTY(ReplicatedUsing = OnRep_MovementState)
	MoveStateHuman CurrentMoveState = MoveStateHuman::EMS_Walk;
	MoveStateHuman LastMoveState = MoveStateHuman::EMS_Walk;


	UPROPERTY()
	class USkeletalMesh* SkeletalMesh01 = nullptr;
	UPROPERTY()
	class USkeletalMesh* SkeletalMesh02 = nullptr;
	UPROPERTY()
	class USkeletalMesh* SkeletalMesh03 = nullptr;
	UPROPERTY()
	class USkeletalMesh* SkeletalMesh04 = nullptr;

private:
	UPROPERTY(Replicated)
	bool IsAimming = false;
	bool CanCharge = false;
	bool IsInAir = false;
	UPROPERTY(Replicated)
	bool IsRun = false;
	bool FireButtonPressed = false;
	bool IsDead = false;
	bool bLeftGame = false;
	UPROPERTY(Replicated)
	bool ChargeButtonPressed = false;
	bool IsMonster = false;
	bool OnSpotlight = false;
	bool IsFalling = false;
	bool IsSlow = false;

	UPROPERTY(ReplicatedUsing = OnRep_SkinIndex)
	int32 SkinIndex = -1;

	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_Health, Category = Combat)
	float Health = 100.f;
	UPROPERTY(EditAnywhere, Category = Combat)
	float MaxHealth = 100.f;

	UPROPERTY(EditAnywhere, Category = Combat)
	float Stemina = 100.f;
	UPROPERTY(EditAnywhere, Category = Combat)
	float MaxStemina = 100.f;

	UPROPERTY(EditAnywhere, Category = Combat)
	float AimWalkSpeed = 150.f;

	UPROPERTY(EditAnywhere, Category = Combat)
	float SlowSpeedTimerDelay = 10.f; // 괴물의 칼에 맞았을때 느려지는 시간

	float InitalCapsuleHalfHeight;
	
	UPROPERTY(EditAnywhere, Category = Combat)
	float WalkSpeed = 350.f; 
	UPROPERTY(EditAnywhere, Category = Combat)
	float RunSpeed = 600.f; 
	UPROPERTY(EditAnywhere, Category = Combat)
	float CrouchSpeed = 150.f; 
	float SlowSpeed = 0.f;
	UPROPERTY(EditAnywhere, Category = Combat)
	float SlowSpeedVariable = 200.f;
	float FallingSpeed;
	float FallingStartHeight;
	float FallingEndHeight;

	// 낙하 높이
	UPROPERTY(EditAnywhere)
	float FallingLowDistance = 500.f;
	UPROPERTY(EditAnywhere)
	float FallingMediumDistance = 900.f;
	UPROPERTY(EditAnywhere)
	float FallingHighDistance = 1200;

	float AO_Pitch;

	FRotator StartingAimRotation;

	FTimerHandle FireTimer;
	FTimerHandle ElimTimer;
	FTimerHandle SlowSpeedTimer;

public:	
	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);
	UFUNCTION()
	void OnRep_OverlappingGenerator(AGenerator* Generator);
	UFUNCTION()
	void OnRep_MovementState();

	UFUNCTION()
	void OnRep_Health();

	UFUNCTION()
	void OnRep_SkinIndex();

	FOnLeftGame OnLeftGame;
};

