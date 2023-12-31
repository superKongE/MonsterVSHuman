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
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn; // 캐릭터 소환 과정중 충돌할 경우 어떻게 처리할 것인지 설정, 블루프린트에서도 설정

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 420;
	CameraBoom->bUsePawnControlRotation = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	Camera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false; // Camera 는 CameraBoom에 붙어있으므로 Camera가 회전하면 같이 회전한다

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
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true; // 캐릭터 클래스에 기본으로 내장된 웅크리기 기능을 활성화

	MonsterCombat = CreateDefaultSubobject<UMonsterCombatComponent>(TEXT("MonsterMonsterCombat"));
	MonsterCombat->SetIsReplicated(true);

	NetUpdateFrequency = 66.f; // 초당 66번 값을 리플리케이트(업데이트) 한다 
	MinNetUpdateFrequency = 33.f; // 초당 최소 33번 이상 업데이트
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
		// 서버에게 준비가 다 됐다고 알리기
		CharacterGameMode->AddUser();
	}
}
// 컴포넌트들이 초기화된후 BeginPlay()가 호출되기전에 
// 즉 캐릭터가 소환되기전에 미리 변수 초기화작업
void AFPSMonster::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (MonsterCombat)
	{
		// MonsterCombat 클래스를 friend 설정해놨으므로 private 변수에 접근이 가능하다
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
		// AO_Pitch 가 InRange 영역안에 있으면 OutRange 범위로 변환한다
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}

	// BeginPlay() 단계에서 PlayerState가 유효하지 않을 수 있으므로 Tick에서 확인처리
	PollInit();

	// 스테미나 관리
	ManageStemina(DeltaTime);

	// 대쉬 관리
	ManageDash(DeltaTime);

	// 공격을 위한 라인트레이싱
	if (AttackButtonPressed)
	{
		const FVector RightWeaponStart_SocketLocation = RightWeaponStart->GetSocketTransform(GetMesh()).GetLocation();
		const FVector RightWeaponEnd_SocketLocation = RightWeaponEnd->GetSocketTransform(GetMesh()).GetLocation();
		GetWorld()->LineTraceSingleByChannel(HitResult, RightWeaponStart_SocketLocation, RightWeaponEnd_SocketLocation, ECollisionChannel::ECC_Pawn);
		
		if (HitResult.GetActor())
		{
			const AFPSCharacter* const HitHuman = Cast<AFPSCharacter>(HitResult.GetActor());

			// 처음 한명 공격하면 라인트레이싱 멈춤
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


// 인간이 괴물 근처에 오면
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
// 인간이 멀어지면
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
	// replicate 변수
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


// 스테미나 관리 함수
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

	// 차지중이고 대쉬에너지가 남아있으면 대쉬에너지 감소
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
	// 차지중이 아니고 대쉬에너지가 꽉차있지 않으면 대쉬에너지 증가
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

// 게임 중간에 나가기
void AFPSMonster::ServerLeftGame_Implementation()
{
	AMainGameMode* MainGameMode = GetWorld()->GetAuthGameMode<AMainGameMode>();
	if (MainGameMode)
	{
		MainGameMode->PlayerLeftGame(this);
	}
}


// 입력에 대한 움직임 관련 함수들
void AFPSMonster::MoveForward(float Value)
{
	if (Value == 0.f) return;

	const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
	const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X)); // YawRotation을 바탕으로 회전 행렬을 만든다음 x축 벡터값을 반환(방향)

	AddMovementInput(Direction, Value); // Direction 방향으로 Value만큼 이동
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
	if (!bIsCrouched) // 조준중이 아니고 서있을때만
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
		GetCharacterMovement()->GroundFriction = 0.f; // 바닥 마찰력 없앰

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
	// 공중에서 대쉬를 여러번 못하게 하기위해 카운트
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
		// 채팅창이 활성화 되어 있지 않으면
		if (!PlayerController->LeftClick())
		{
			if (MonsterCombat && !AttackButtonPressed)
			{
				Server_Attack();
			}
		}
	}
}

// 던지는 키를 누르면 호출됨(차징)
void AFPSMonster::Throw()
{
	ThrowButtonPressed = true;

	// 쿨타임이면
	if (IsThrowCoolTime)
	{
		// 쿨타임 도중에 Throw 버튼을 눌렀다고 표시
		// 쿨타임 도중 Throw 키를 눌렀다가 쿨타임이 다 되었을때 키를 때 동작하는것을 방지하기 위함
		Server_ThrowKeyPressedDuringCoolTime(true);
		ThrowButtonPressed = false;
		return;
	}

	PlayerController = PlayerController == nullptr ? Cast<ACharacterController>(Controller) : PlayerController;
	if (PlayerController && MonsterCombat)
	{
		// 채팅창이 활성화 되어 있지 않으면
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
// 던지는 자세를 유지하기 위한 AnimNotify
void AFPSMonster::Loop()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && AttackMontage)
	{
		FName SectionName = FName("loop");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}
// 칼 던지는 AnimNotify
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
// 던지는 키를 때면 호출됨(던지기)
void AFPSMonster::ChargeThrowEnd()
{
	// 쿨타임이거나 쿨타임 도중 키를 눌렀을 경우
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
// 던지기 쿨타임
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
		// 타임라인 역재생
		DissolveTimeline->Reverse();
	}
}


void AFPSMonster::Server_Attack_Implementation()
{
	AttackButtonPressed = true;
	DisableInput(PlayerController);
	PlayNormalAttackMontage();
}
void AFPSMonster::NormalAttackEnd() // 휘두르는게 끝남 
{
	UGameplayStatics::PlaySoundAtLocation(this, NormalAttackFailSound, GetActorLocation());
}
void AFPSMonster::NormalAttackFinish() // 모든 공격 동작이 끝남
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

		// 해당영역으로 점프
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}


// 캐릭터가 죽었을때 관련 함수들
// 서버에서 호출된다
void AFPSMonster::Elim(bool bPlyerLeftGame)
{
	Multicast_Elim(bPlyerLeftGame);
}
void AFPSMonster::Multicast_Elim_Implementation(bool bPlayerLeftGame)
{
	bLeftGame = bPlayerLeftGame;

	// 죽는 애니메이션 재생하기
	IsDead = true;

	// 죽고나면 움직임 등을 막음
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
// 부활요청
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


// 지면마다 발소리를 다르게 하기 위한 지면이 어떤 타입인지
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
