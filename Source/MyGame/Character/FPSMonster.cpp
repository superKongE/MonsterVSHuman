 // Fill out your copyright notice in the Description page of Project Settings.


#include "FPSMonster.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SkeletalMeshComponent.h"
#include "MyGame/Weapon.h"
#include "MyGame/Combat/MonsterCombatComponent.h"
#include "Animation/AnimInstance.h"
#include "MyGame/PlayerController/CharacterController.h"
#include "MyGame/GameMode/MainGameMode.h"
#include "MyGame/Weapon/Dagger.h"
#include "MyGame/PlayerState/CharacterState.h"
#include "MyGame/HumanGameInstance.h"
#include "Components/TimelineComponent.h"
#include "Components/SphereComponent.h"
#include "Components/PostProcessComponent.h"
#include "Curves/CurveFloat.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInstance.h"
#include "MyGame/Weapon/WeaponType.h"
#include "MyGame/Character/FPSCharacter.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "PhysicalMaterials/PhysicalMaterial.h"

AFPSMonster::AFPSMonster()
{
	PrimaryActorTick.bCanEverTick = true;
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn; // ĳ���� ��ȯ ������ �浹�� ��� ��� ó���� ������ ����, �������Ʈ������ ����

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 420;
	CameraBoom->bUsePawnControlRotation = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	Camera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false; // Camera �� CameraBoom�� �پ������Ƿ� Camera�� ȸ���ϸ� ���� ȸ���Ѵ�

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(GetMesh());

	PostProcessComponent = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcessComponent"));
	PostProcessComponent->SetupAttachment(RootComponent);
	
	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveMaterialline"));

	NormalAttackSuccessSound = CreateDefaultSubobject<USoundCue>(TEXT("NormalAttackSuccess"));
	NormalAttackFailSound = CreateDefaultSubobject<USoundCue>(TEXT("NormalAttackFail"));

	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 10000.f;
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true; // ĳ���� Ŭ������ �⺻���� ����� ��ũ���� ����� Ȱ��ȭ

	MonsterCombat = CreateDefaultSubobject<UMonsterCombatComponent>(TEXT("MonsterMonsterCombat"));
	MonsterCombat->SetIsReplicated(true);

	NetUpdateFrequency = 66.f; // �ʴ� 66�� ���� ���ø�����Ʈ(������Ʈ) �Ѵ� 
	MinNetUpdateFrequency = 33.f; // �ʴ� �ּ� 33�� �̻� ������Ʈ
}
void AFPSMonster::BeginPlay()
{
	Super::BeginPlay();

	UpdateHUDHealth();

	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &AFPSMonster::ReceiveDamage);
	}

	PlayerController = PlayerController == nullptr ? Cast<ACharacterController>(Controller) : PlayerController;
	if (PlayerController)
	{
		PlayerController->OwnerCharacter = this;
	}

	DynamicDissolveMaterialInstance1 = UMaterialInstanceDynamic::Create(DissolveMaterialInstance1, this);
	DynamicDissolveMaterialInstance2 = UMaterialInstanceDynamic::Create(DissolveMaterialInstance2, this);
	DynamicDissolveMaterialInstance3 = UMaterialInstanceDynamic::Create(DissolveMaterialInstance3, this);
	DynamicDissolveMaterialInstance4 = UMaterialInstanceDynamic::Create(DissolveMaterialInstance4, this);
	DynamicDissolveMaterialInstance5 = UMaterialInstanceDynamic::Create(DissolveMaterialInstance5, this);
	DynamicDissolveMaterialInstance6 = UMaterialInstanceDynamic::Create(DissolveMaterialInstance6, this);
	DynamicDissolveMaterialInstance7 = UMaterialInstanceDynamic::Create(DissolveMaterialInstance7, this);
	DynamicDissolveMaterialInstance8 = UMaterialInstanceDynamic::Create(DissolveMaterialInstance8, this);
	DynamicDissolveMaterialInstance9 = UMaterialInstanceDynamic::Create(DissolveMaterialInstance9, this);
	DynamicDissolveMaterialInstance10 = UMaterialInstanceDynamic::Create(DissolveMaterialInstance10, this);
	DynamicDissolveMaterialInstance11 = UMaterialInstanceDynamic::Create(DissolveMaterialInstance11, this);

	GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance1);
	GetMesh()->SetMaterial(1, DynamicDissolveMaterialInstance2);
	GetMesh()->SetMaterial(2, DynamicDissolveMaterialInstance3);
	GetMesh()->SetMaterial(3, DynamicDissolveMaterialInstance4);
	GetMesh()->SetMaterial(4, DynamicDissolveMaterialInstance5);
	GetMesh()->SetMaterial(5, DynamicDissolveMaterialInstance6);
	GetMesh()->SetMaterial(6, DynamicDissolveMaterialInstance7);
	GetMesh()->SetMaterial(7, DynamicDissolveMaterialInstance8);
	GetMesh()->SetMaterial(8, DynamicDissolveMaterialInstance9);
	GetMesh()->SetMaterial(9, DynamicDissolveMaterialInstance10);
	GetMesh()->SetMaterial(10, DynamicDissolveMaterialInstance11);

	LeftWeaponStart = GetMesh()->GetSocketByName(FName("LeftWeaponStart"));
	LeftWeaponEnd = GetMesh()->GetSocketByName(FName("LeftWeaponEnd"));
	RightWeaponStart = GetMesh()->GetSocketByName(FName("RightWeaponStart"));
	RightWeaponEnd = GetMesh()->GetSocketByName(FName("RightWeaponEnd"));

	if (GetLocalRole() == ENetRole::ROLE_Authority)
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AFPSMonster::OnSphereOverlap);
		AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AFPSMonster::OnSphereEndOverlap);
	}

	if (IdleSoundCue && IsLocallyControlled())
	{
		UGameplayStatics::PlaySound2D(this, IdleSoundCue);
	}
	if (StartSound)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), StartSound, GetActorLocation());
	}

	PostProcessComponent->bHiddenInGame = true;

	AMainGameMode* CharacterGameMode = GetWorld()->GetAuthGameMode<AMainGameMode>();
	if (CharacterGameMode)
	{
		// �������� �غ� �� �ƴٰ� �˸���
		CharacterGameMode->AddUser();
	}
}
// ������Ʈ���� �ʱ�ȭ���� BeginPlay()�� ȣ��Ǳ����� 
// �� ĳ���Ͱ� ��ȯ�Ǳ����� �̸� ���� �ʱ�ȭ�۾�
void AFPSMonster::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (MonsterCombat)
	{
		// MonsterCombat Ŭ������ friend �����س����Ƿ� private ������ ������ �����ϴ�
		MonsterCombat->Character = this;
	}
}
void AFPSMonster::PollInit()
{
	if (CharacterPlayerState == nullptr)
	{
		CharacterPlayerState = GetPlayerState<ACharacterState>();

		if (CharacterPlayerState)
		{
			CharacterPlayerState->AddToScore(0.f);
			CharacterPlayerState->AddToDefeat(0);
		}
	}
}
void AFPSMonster::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//DOREPLIFETIME(AFPSMonster, Health);//
	DOREPLIFETIME(AFPSMonster, IsDissolve);
	DOREPLIFETIME(AFPSMonster, AttackButtonPressed);
	DOREPLIFETIME(AFPSMonster, bChargeDash);
	DOREPLIFETIME(AFPSMonster, ChargedDash);
	DOREPLIFETIME_CONDITION(AFPSMonster, Dash, COND_OwnerOnly);
}
void AFPSMonster::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CurrentSlowSpeed = FMath::Clamp(CurrentSlowSpeed -= DeltaTime * SlowSpeedReduce, 0, SlowSpeed);
	Server_SpeedChange(CurrentMoveState);

	if (!GetCharacterMovement()->IsFalling())
	{
		AirDashCount = 0;
	}

	IsInAir = GetCharacterMovement()->IsFalling();

	if (!IsRun && !GetCharacterMovement()->IsFalling())
	{
		AO_Pitch = GetBaseAimRotation().Pitch;
	}

	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		// AO_Pitch �� InRange �����ȿ� ������ OutRange ������ ��ȯ�Ѵ�
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}

	// BeginPlay() �ܰ迡�� PlayerState�� ��ȿ���� ���� �� �����Ƿ� Tick���� Ȯ��ó��
	PollInit();

	// ���׹̳� ����
	ManageStemina(DeltaTime);

	// �뽬 ����
	ManageDash(DeltaTime);

	// ������ ���� ����Ʈ���̽�
	if (AttackButtonPressed)
	{
		const FVector RightWeaponStart_SocketLocation = RightWeaponStart->GetSocketTransform(GetMesh()).GetLocation();
		const FVector RightWeaponEnd_SocketLocation = RightWeaponEnd->GetSocketTransform(GetMesh()).GetLocation();
		GetWorld()->LineTraceSingleByChannel(HitResult, RightWeaponStart_SocketLocation, RightWeaponEnd_SocketLocation, ECollisionChannel::ECC_Pawn);
		
		if (HitResult.GetActor())
		{
			const AFPSCharacter* const HitHuman = Cast<AFPSCharacter>(HitResult.GetActor());

			// ó�� �Ѹ� �����ϸ� ����Ʈ���̽� ����
			if (HitHuman)
			{
				AttackButtonPressed = false;
				UGameplayStatics::ApplyDamage(HitResult.GetActor(), NormalDamage - CurrentMorningNormalDamge, Controller, this, UDamageType::StaticClass());
				UGameplayStatics::PlaySoundAtLocation(this, NormalAttackSuccessSound, HitResult.ImpactPoint);
			}
		}
	}
}
void AFPSMonster::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AFPSMonster::MoveForward);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AFPSMonster::NormalAttack);
	PlayerInputComponent->BindAction("Aimming", IE_Pressed, this, &AFPSMonster::ChargeDash);
	PlayerInputComponent->BindAction("Aimming", IE_Released, this, &AFPSMonster::StartDash);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AFPSMonster::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &AFPSMonster::StopJumping);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AFPSMonster::DoCrouch);
	PlayerInputComponent->BindAction("Run", IE_Pressed, this, &AFPSMonster::Run);
	PlayerInputComponent->BindAction("Run", IE_Released, this, &AFPSMonster::StopRun);
	PlayerInputComponent->BindAction("Dissolve", IE_Pressed, this, &AFPSMonster::DissolveButtonPressed);
	PlayerInputComponent->BindAction("ThermalVision", IE_Pressed, this, &AFPSMonster::ThermalVision);

	PlayerInputComponent->BindAction("Throw", IE_Pressed, this, &AFPSMonster::Throw);
	PlayerInputComponent->BindAction("Throw", IE_Released, this, &AFPSMonster::ChargeThrowEnd);

	PlayerInputComponent->BindAxis("MoveRight", this, &AFPSMonster::MoveRight);
	PlayerInputComponent->BindAxis("LookUp", this, &AFPSMonster::LookUp);
	PlayerInputComponent->BindAxis("LookRight", this, &AFPSMonster::LookRight);
}


// �ΰ��� ���� ��ó�� ����
void AFPSMonster::OnSphereOverlap(class UPrimitiveComponent* OverlappedComponent, class AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	AFPSCharacter* Human = Cast<AFPSCharacter>(OtherActor);
	if (Human)
	{
		Human->PlayChasingSound();
	}
}
// �ΰ��� �־�����
void AFPSMonster::OnSphereEndOverlap(class UPrimitiveComponent* OverlappedComponent, class AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	AFPSCharacter* Human = Cast<AFPSCharacter>(OtherActor);
	if (Human)
	{
		Human->PlayIdleSound();
	}
}


void AFPSMonster::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCursor)
{
	CurrentSlowSpeed = SlowSpeed;
	Server_SpeedChange(CurrentMoveState);
	// replicate ����
	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);

	UpdateHUDHealth();

	if (Health <= 0.f && !IsDead)
	{
		IsDead = true;
		AMainGameMode* CharacterGameMode = GetWorld()->GetAuthGameMode<AMainGameMode>();

		if (CharacterGameMode)
		{
			ACharacterController* AttackerController = Cast<ACharacterController>(InstigatorController);
			PlayerController = PlayerController == nullptr ? Cast<ACharacterController>(Controller) : PlayerController;

			if (AttackerController && PlayerController)
			{
				CharacterGameMode->PlayerEliminated(this, PlayerController, AttackerController);
			}
		}
	}
}
void AFPSMonster::UpdateHUDHealth()
{
	PlayerController = PlayerController == nullptr ? Cast<ACharacterController>(Controller) : PlayerController;
	if (PlayerController)
	{
		PlayerController->SetHUDHealth(Health, MaxHealth);
	}
}


// ���׹̳� ���� �Լ�
void AFPSMonster::ManageStemina(float DeltaTime)
{
	PlayerController = PlayerController == nullptr ? Cast<ACharacterController>(Controller) : PlayerController;

	if (IsDissolve && Stemina >= 0.f)
	{
		Stemina -= 5.f * DeltaTime;

		if (PlayerController)
		{
			PlayerController->SetHUDStemina(Stemina, MaxStemina);
		}

		if (Stemina <= 0.f)
		{
			IsDissolve = false;
			Server_DissolveButtonPressed(IsDissolve);
		}
	}
	else if (!IsDissolve && Stemina < MaxStemina)
	{
		Stemina += 5.f * DeltaTime;

		if (PlayerController)
		{
			PlayerController->SetHUDStemina(Stemina, MaxStemina);
		}
	}
}
void AFPSMonster::ManageDash(float DeltaTime)
{
	PlayerController = PlayerController == nullptr ? Cast<ACharacterController>(Controller) : PlayerController;

	// �������̰� �뽬�������� ���������� �뽬������ ����
	if (bChargeDash && Dash >= 0.f)
	{
		Dash -= DecreaseDash * DeltaTime * 2.5f;
		ChargedDash += 0.1f * DeltaTime;

		if (PlayerController)
		{
			if (Dash < 0.f) Dash = 0.f;

			PlayerController->SetHUDDash(Dash, MaxDash);
		}
	}
	// �������� �ƴϰ� �뽬�������� �������� ������ �뽬������ ����
	else if (!bChargeDash && Dash < MaxDash)
	{
		Dash += IncreaseDash * DeltaTime;

		if (PlayerController)
		{
			if (Dash > MaxDash) Dash = MaxDash;
			PlayerController->SetHUDDash(Dash, MaxDash);
		}
	}
}

// ���� �߰��� ������
void AFPSMonster::ServerLeftGame_Implementation()
{
	AMainGameMode* MainGameMode = GetWorld()->GetAuthGameMode<AMainGameMode>();
	if (MainGameMode)
	{
		MainGameMode->PlayerLeftGame(this);
	}
}


// �Է¿� ���� ������ ���� �Լ���
void AFPSMonster::MoveForward(float Value)
{
	if (Value == 0.f) return;

	const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
	const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X)); // YawRotation�� �������� ȸ�� ����� ������� x�� ���Ͱ��� ��ȯ(����)

	AddMovementInput(Direction, Value); // Direction �������� Value��ŭ �̵�
}
void AFPSMonster::MoveRight(float Value)
{
	if (Value == 0.f) return;

	const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
	const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));

	AddMovementInput(Direction, Value);
}
void AFPSMonster::LookUp(float Value)
{
	if (Value == 0.f) return;

	AddControllerPitchInput(Value);
}
void AFPSMonster::LookRight(float Value)
{
	if (Value == 0.f) return;

	AddControllerYawInput(Value);
}
void AFPSMonster::DoCrouch()
{
	if (!bIsCrouched)
	{
		//GetCharacterMovement()->MaxWalkSpeed = CrouchSpeed;
		//GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;
		Server_SpeedChange(MoveStateMonster::EMS_Crouch);
		Crouch();
	}

	else
	{
		//GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
		Server_SpeedChange(MoveStateMonster::EMS_Walk);
		UnCrouch();
	}
}
void AFPSMonster::Run()
{
	if (!bIsCrouched) // �������� �ƴϰ� ����������
	{
		//GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
		Server_SpeedChange(MoveStateMonster::EMS_Run);
	}
}
void AFPSMonster::StopRun()
{
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	Server_SpeedChange(MoveStateMonster::EMS_Walk);
}
void AFPSMonster::Jump()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}

	if (!bIsCrouched)
	{
		Super::Jump();
	}
}
void AFPSMonster::ChargeDash()
{
	if (!IsDash && AirDashCount < 2)
	{
		bChargeDash = true;
	}
}
void AFPSMonster::StartDash()
{
	PlayerController = PlayerController == nullptr ? Cast<ACharacterController>(Controller) : PlayerController;
	if (PlayerController == nullptr) return;

	if (GetCharacterMovement()->IsFalling())
	{
		AirDashCount++;
	}
	if (!IsDash && AirDashCount < 2)
	{
		GetCharacterMovement()->GroundFriction = 0.f; // �ٴ� ������ ����

		FVector Velocity = GetCharacterMovement()->Velocity;
		Velocity = Velocity.GetSafeNormal();
		GetCharacterMovement()->AddImpulse(Velocity * 200000.f * ChargedDash, true);
		IsDash = true;
		bChargeDash = false;
		MulticastDash();
	}
}
void AFPSMonster::MulticastDash_Implementation()
{
	// ���߿��� �뽬�� ������ ���ϰ� �ϱ����� ī��Ʈ
	if (GetCharacterMovement()->IsFalling())
	{
		AirDashCount++;
	}

	bChargeDash = false;
	IsDash = true;

	if (ChargedDash > 0.f)
	{
		GetWorldTimerManager().SetTimer(
			DashTimer,
			this,
			&AFPSMonster::EndDash,
			ChargedDash
		);
	}
	else
	{
		EndDash();
	}
}
void AFPSMonster::EndDash()
{
	IsDash = false;
	ChargedDash = 0.f;

	GetCharacterMovement()->GroundFriction = 8.f;
	GetCharacterMovement()->StopMovementImmediately();
}
void AFPSMonster::LaunchCharacter(FVector LaunchVelocity, bool bXYOverride, bool bZOverride)
{
	Super::LaunchCharacter(LaunchVelocity, bXYOverride, bZOverride);

	//GetCharacterMovement()->SetMovementMode(EMovementMode::MOVE_Walking);
}

void AFPSMonster::NormalAttack()
{
	if (ThrowButtonPressed) return;

	PlayerController = PlayerController == nullptr ? Cast<ACharacterController>(Controller) : PlayerController;
	if (PlayerController)
	{
		// ä��â�� Ȱ��ȭ �Ǿ� ���� ������
		if (!PlayerController->LeftClick())
		{
			if (MonsterCombat && !AttackButtonPressed)
			{
				Server_Attack();
			}
		}
	}
}

// ������ Ű�� ������ ȣ���(��¡)
void AFPSMonster::Throw()
{
	ThrowButtonPressed = true;

	// ��Ÿ���̸�
	if (IsThrowCoolTime)
	{
		// ��Ÿ�� ���߿� Throw ��ư�� �����ٰ� ǥ��
		// ��Ÿ�� ���� Throw Ű�� �����ٰ� ��Ÿ���� �� �Ǿ����� Ű�� �� �����ϴ°��� �����ϱ� ����
		Server_ThrowKeyPressedDuringCoolTime(true);
		ThrowButtonPressed = false;
		return;
	}

	PlayerController = PlayerController == nullptr ? Cast<ACharacterController>(Controller) : PlayerController;
	if (PlayerController && MonsterCombat)
	{
		// ä��â�� Ȱ��ȭ �Ǿ� ���� ������
		if (!PlayerController->LeftClick())
		{
			Server_ThrowKeyPressedDuringCoolTime(false);
			Multicast_Throw();
			//PlayThrowMontage();
			MonsterCombat->SetAimming(true);
		}
	}
}
void AFPSMonster::Server_ThrowKeyPressedDuringCoolTime_Implementation(bool bPress)
{
	ThrowKeyPressedDuringCoolTime = bPress;
}
void AFPSMonster::Multicast_Throw_Implementation()
{
	PlayThrowMontage();
}
void AFPSMonster::PlayThrowMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && AttackMontage)
	{
		AnimInstance->Montage_Play(AttackMontage);

		FName SectionName = FName("Throw");

		AnimInstance->Montage_JumpToSection(SectionName);
	}
}
// ������ �ڼ��� �����ϱ� ���� AnimNotify
void AFPSMonster::Loop()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && AttackMontage)
	{
		FName SectionName = FName("loop");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}
// Į ������ AnimNotify
void AFPSMonster::ThrowDagger()
{
	if (!HasAuthority()) return;
	if (MonsterCombat == nullptr) return;

	const USkeletalMeshSocket* ThrowPoint = GetMesh()->GetSocketByName(FName("ThrowPoint"));
	FTransform ThrowPointTransform = ThrowPoint->GetSocketTransform(GetMesh());

	FVector StartLocation = ThrowPointTransform.GetLocation();
	FVector ToTarget = MonsterCombat->HitTarget - StartLocation;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;

	UWorld* World = GetWorld();
	if (World)
	{
		World->SpawnActor<ADagger>(
			ThrowDaggerClass,
			StartLocation,
			ToTarget.Rotation(),
			SpawnParams
			);
	}
}
// ������ Ű�� ���� ȣ���(������)
void AFPSMonster::ChargeThrowEnd()
{
	// ��Ÿ���̰ų� ��Ÿ�� ���� Ű�� ������ ���
	if (MonsterCombat == nullptr || IsThrowCoolTime || ThrowKeyPressedDuringCoolTime) return;

	IsThrowCoolTime = true;
	Multicast_ChargeThrowEnd();
}
void AFPSMonster::Multicast_ChargeThrowEnd_Implementation()
{
	ThrowButtonPressed = false;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && AttackMontage)
	{
		FName SectionName = FName("ThrowDagger");

		AnimInstance->Montage_JumpToSection(SectionName);
	}

	MonsterCombat->SetAimming(false);

	IsThrowCoolTime = true;
	GetWorldTimerManager().SetTimer(ThrowTimer, this, &AFPSMonster::ThrowCoolTimeEnd, 1.f, true, 0.f);
}
// ������ ��Ÿ��
void AFPSMonster::ThrowCoolTimeEnd()
{
	if (IsLocallyControlled())
	{
		if (PlayerController)
		{
			PlayerController->SetThrowCoolTimeBarHUD(ThrowCoolTimePercentage);
			ThrowCoolTimePercentage -= (100 / ThrowCoolTimeCopy--);
		}
	}

	if (ThrowCoolTimeCopy == 0.f)
	{
		GetWorldTimerManager().ClearTimer(ThrowTimer);
		ThrowCoolTimePercentage = 100.f;
		ThrowCoolTimeCopy = ThrowCoolTime;
		IsThrowCoolTime = false;
	}
}


void AFPSMonster::ThermalVision()
{
	bThermalVision = !bThermalVision;

	if (bThermalVision)
	{
		PostProcessComponent->bHiddenInGame = false;
	}
	else
	{
		PostProcessComponent->bHiddenInGame = true;
	}
}


void AFPSMonster::DissolveButtonPressed()
{
	IsDissolve = !IsDissolve;

	Server_DissolveButtonPressed(IsDissolve);
}
void AFPSMonster::Server_DissolveButtonPressed_Implementation(bool b)
{
	Multicast_DissolveButtonPressed(b);
}
void AFPSMonster::Multicast_DissolveButtonPressed_Implementation(bool b)
{
	if (b)
	{
		DissolveTrack.BindDynamic(this, &AFPSMonster::UpdateDissolveMaterial);
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->Play();
	}
	else
	{
		DissolveTrack.BindDynamic(this, &AFPSMonster::UpdateDissolveMaterial);
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		// Ÿ�Ӷ��� �����
		DissolveTimeline->Reverse();
	}
}


void AFPSMonster::Server_Attack_Implementation()
{
	AttackButtonPressed = true;
	DisableInput(PlayerController);
	PlayNormalAttackMontage();
}
void AFPSMonster::NormalAttackEnd() // �ֵθ��°� ���� 
{
	UGameplayStatics::PlaySoundAtLocation(this, NormalAttackFailSound, GetActorLocation());
}
void AFPSMonster::NormalAttackFinish() // ��� ���� ������ ����
{
	AttackButtonPressed = false;
	EnableInput(PlayerController);
}
void AFPSMonster::OnRep_AttackButtonPressed()
{
	if (AttackButtonPressed)
	{
		DisableInput(PlayerController);
		PlayNormalAttackMontage();
	}
}
void AFPSMonster::PlayNormalAttackMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && AttackMontage)
	{
		AnimInstance->Montage_Play(AttackMontage);

		FName SectionName = FName("NormalAttack");

		// �ش翵������ ����
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}


// ĳ���Ͱ� �׾����� ���� �Լ���
// �������� ȣ��ȴ�
void AFPSMonster::Elim(bool bPlyerLeftGame)
{
	Multicast_Elim(bPlyerLeftGame);
}
void AFPSMonster::Multicast_Elim_Implementation(bool bPlayerLeftGame)
{
	bLeftGame = bPlayerLeftGame;

	// �״� �ִϸ��̼� ����ϱ�
	IsDead = true;

	// �װ��� ������ ���� ����
	GetCharacterMovement()->DisableMovement();
	if (IsDissolve)
	{
		IsDissolve = false;
	}
	//if (MonsterCombat)
	//{
	//	MonsterCombat->SetFireButtonPressed(false);
	//}

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	GetWorldTimerManager().SetTimer(
		ElimTimer,
		this,
		&AFPSMonster::EndDissolve,
		1.5f
	);
}
// ��Ȱ��û
void AFPSMonster::EndDissolve()
{
	AMainGameMode* CharacterGameMode = GetWorld()->GetAuthGameMode<AMainGameMode>();

	if (IsLocallyControlled() && bLeftGame)
	{
		OnLeftGames.Broadcast();
	}
	else if (HasAuthority() && CharacterGameMode)
	{
		CharacterGameMode->RequestRespone(this, Cast<ACharacterController>(Controller), true);
	}
}
void AFPSMonster::UpdateDissolveMaterial(float DissolveValue)
{
	if (DynamicDissolveMaterialInstance1 && DynamicDissolveMaterialInstance2 && DynamicDissolveMaterialInstance3 && DynamicDissolveMaterialInstance4 && DynamicDissolveMaterialInstance5 && DynamicDissolveMaterialInstance6 &&
		DynamicDissolveMaterialInstance7 && DynamicDissolveMaterialInstance8 && DynamicDissolveMaterialInstance9 && DynamicDissolveMaterialInstance10)
	{
		DynamicDissolveMaterialInstance1->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
		DynamicDissolveMaterialInstance2->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
		DynamicDissolveMaterialInstance3->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
		DynamicDissolveMaterialInstance4->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
		DynamicDissolveMaterialInstance5->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
		DynamicDissolveMaterialInstance6->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
		DynamicDissolveMaterialInstance7->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
		DynamicDissolveMaterialInstance8->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
		DynamicDissolveMaterialInstance9->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
		DynamicDissolveMaterialInstance10->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}
}


void AFPSMonster::Server_SpeedChange_Implementation(MoveStateMonster EMS)
{
	CurrentMoveState = EMS;

	switch (EMS)
	{
	case MoveStateMonster::EMS_Walk:
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed - CurrentSlowSpeed - CurrentMorningSlowSpeed;
		break;
	case MoveStateMonster::EMS_Crouch:
		GetCharacterMovement()->MaxWalkSpeed = FMath::Clamp(CrouchSpeed - CurrentSlowSpeed - CurrentMorningSlowSpeed, 10, CrouchSpeed);
		//GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;
		break;
	case MoveStateMonster::EMS_Run:
		GetCharacterMovement()->MaxWalkSpeed = RunSpeed - CurrentSlowSpeed - CurrentMorningSlowSpeed;
		break;
	}
}


// ���鸶�� �߼Ҹ��� �ٸ��� �ϱ� ���� ������ � Ÿ������
EPhysicalSurface AFPSMonster::GetSurfaceType()
{
	FHitResult HitSurfaceResult;
	const FVector Start = GetActorLocation();
	const FVector End = Start + FVector(0.f, 0.f, -400.f);
	FCollisionQueryParams QueryParams;
	QueryParams.bReturnPhysicalMaterial = true;

	GetWorld()->LineTraceSingleByChannel(HitSurfaceResult, Start, End, ECollisionChannel::ECC_Visibility, QueryParams);

	return UPhysicalMaterial::DetermineSurfaceType(HitSurfaceResult.PhysMaterial.Get());
}

void AFPSMonster::ChangeToMorning()
{
	CurrentMorningSlowSpeed = MorningSlowSpeed;
	CurrentMorningNormalDamge = MorningNormalDamage;
}
void AFPSMonster::ChangeToNight()
{
	CurrentMorningSlowSpeed = 0.f;
	CurrentMorningNormalDamge = 0.f;
}
