#include "FPSCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SkeletalMeshComponent.h"
#include "MyGame/Weapon.h"
#include "MyGame/Combat/CombatComponent.h"
#include "MyGame/ChatComponent.h"
#include "MyGame/HUD/GeneratorWidget.h"
#include "MyGame/DamageType/SlowEffectDamage.h"
#include "Animation/AnimInstance.h"
#include "MyGame/PlayerController/CharacterController.h"
#include "MyGame/GameMode/MainGameMode.h"
#include "MyGame/PlayerState/CharacterState.h"
#include "MyGame/HumanGameInstance.h"
#include "MyGame/Weapon/WeaponType.h"
#include "MyGame/HUD/CharacterHUD.h"
#include "MyAnimInstance.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "MyGame/Generator.h"
#include "Components/SpotLightComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "PhysicalMaterials/PhysicalMaterial.h"


AFPSCharacter::AFPSCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true; // Property Replicate를 하겠다는 뜻

	SetReplicates(true); // 이 Actor를 Replicate 하겠다는 뜻

	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn; // 캐릭터 소환 과정중 충돌할 경우 어떻게 처리할 것인지 설정, 블루프린트에서도 설정

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 420.f;
	CameraBoom->bUsePawnControlRotation = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	Camera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false; // Camera 는 CameraBoom에 붙어있으므로 Camera가 회전하면 같이 회전한다


	Spotlight = CreateDefaultSubobject<USpotLightComponent>(TEXT("Spotlight"));
	Spotlight->SetupAttachment(Camera);
	Spotlight->SetVisibility(false);

	//NameWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("NameWidgetComponent"));
	//NameWidget->SetupAttachment(RootComponent);

	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	GetCharacterMovement()->JumpZVelocity = 600;
	GetCharacterMovement()->AirControl = 700.f;
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true; // 캐릭터 클래스에 기본으로 내장된 웅크리기 기능을 활성화

	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("Combatcomponent"));
	CombatComponent->SetIsReplicated(true); // 컴포넌트를 Replicat 시키기 위한 설정

	ChatComponent = CreateDefaultSubobject<UChatComponent>(TEXT("ChatComponent"));

	NetUpdateFrequency = 66.f; // 초당 66번 값을 리플리케이트(업데이트) 한다 
	MinNetUpdateFrequency = 33.f; // 초당 최소 33번 이상 업데이트

	IdleSoundCue = CreateDefaultSubobject<USoundCue>(TEXT("IdleSound"));
	HumanChasingCue = CreateDefaultSubobject<USoundCue>(TEXT("HumanChasing"));

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> SkeletalMesh1(TEXT("/Game/SCK_Casual01/Models/Premade_Characters/MESH_PC_00"));
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> SkeletalMesh2(TEXT("/Game/SCK_Casual01/Models/Premade_Characters/MESH_PC_01"));
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> SkeletalMesh3(TEXT("/Game/SCK_Casual01/Models/Premade_Characters/MESH_PC_02"));
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> SkeletalMesh4(TEXT("/Game/SCK_Casual01/Models/Premade_Characters/MESH_PC_03"));

	SkeletalMesh01 = SkeletalMesh1.Object;
	SkeletalMesh02 = SkeletalMesh2.Object;
	SkeletalMesh03 = SkeletalMesh3.Object;
	SkeletalMesh04 = SkeletalMesh4.Object;
}
void AFPSCharacter::BeginPlay()
{
	Super::BeginPlay();
	//NameWidget->SetVisibility(true);

	if (IdleSoundCue && HumanChasingCue)
	{
		IdleAudio = UGameplayStatics::CreateSound2D(this, IdleSoundCue, 1.0f, 1.0f, 0.0f, nullptr, false, false);
		ChasingAudio = UGameplayStatics::CreateSound2D(this, HumanChasingCue, 1.0f, 1.0f, 0.0f, nullptr, false, false);
	}

	UpdateHUDHealth();

	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &AFPSCharacter::ReceiveDamage);

		AMainGameMode* CharacterGameMode = GetWorld()->GetAuthGameMode<AMainGameMode>();
		if (CharacterGameMode)
		{
			CharacterGameMode->AddUser();
		}
	}


	// 무기 타입, 총알 개수 HUD 띄우기
	PlayerController = PlayerController == nullptr ? Cast<ACharacterController>(Controller) : PlayerController;
	if (PlayerController)
	{
		PlayerController->SetHUDWeaponType(CurrentWeaponType);
		PlayerController->SetHUDKnife();
		PlayerController->OwnerCharacter = this;
	}
	

	// 로비에서 선택한 캐릭터 스킨 입히기
	if (IsLocallyControlled())
	{
		HumanGameInstance = HumanGameInstance == nullptr ? Cast<UHumanGameInstance>(GetGameInstance()) : HumanGameInstance;
		if (HumanGameInstance)
		{
			int32 num = HumanGameInstance->GetCharacterSkin();
			Server_SetSkeletalMesh(num);
			if (num == 0)
			{
				GetMesh()->SetSkeletalMesh(SkeletalMesh01);
			}
			else if (num == 1)
			{
				GetMesh()->SetSkeletalMesh(SkeletalMesh02);
			}
			else if (num == 2)
			{
				GetMesh()->SetSkeletalMesh(SkeletalMesh03);
			}
			else if (num == 3)
			{
				GetMesh()->SetSkeletalMesh(SkeletalMesh04);
			}
		}
	}

	if (IsLocallyControlled())
	{
		IdleAudio->Play();
	}

	if (IsLocallyControlled() && PlayerController)
	{
		PlayerController->SetHUDVisible();
		PlayerController->SetHumanInputMode();
	}

	// 괴물의 적외선에 탐지되기 위한 설정
	GetMesh()->SetRenderCustomDepth(true);
}
void AFPSCharacter::OnRep_SkinIndex()
{
	if (SkinIndex == 0)
	{
		GetMesh()->SetSkeletalMesh(SkeletalMesh01);
	}
	else if (SkinIndex == 1)
	{
		GetMesh()->SetSkeletalMesh(SkeletalMesh02);
	}
	else if (SkinIndex == 2)
	{
		GetMesh()->SetSkeletalMesh(SkeletalMesh03);
	}
	else if (SkinIndex == 3)
	{
		GetMesh()->SetSkeletalMesh(SkeletalMesh04);
	}
}
void AFPSCharacter::Server_SetSkeletalMesh_Implementation(int num)
{
	SkinIndex = num;

	if (num == 0)
	{
		GetMesh()->SetSkeletalMesh(SkeletalMesh01);
	}
	else if (num == 1)
	{
		GetMesh()->SetSkeletalMesh(SkeletalMesh02);
	}
	else if (num == 2)
	{
		GetMesh()->SetSkeletalMesh(SkeletalMesh03);
	}
	else if (num == 3)
	{
		GetMesh()->SetSkeletalMesh(SkeletalMesh04);
	}
}
// 컴포넌트들이 초기화된후 BeginPlay()가 호출되기전에 
// 즉 캐릭터가 소환되기전에 미리 변수 초기화작업
void AFPSCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (CombatComponent)
	{
		// CombatComponent 클래스를 friend 설정해놨으므로 private 변수에 접근이 가능하다
		CombatComponent->Character = this;
	}
}
void AFPSCharacter::PollInit()
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
void AFPSCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AFPSCharacter, OverlappedWeapon, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AFPSCharacter, OverlappedGenerator, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AFPSCharacter, ChargeButtonPressed, COND_OwnerOnly);///
	DOREPLIFETIME_CONDITION(AFPSCharacter, Health, COND_OwnerOnly);///
	DOREPLIFETIME(AFPSCharacter, IsAimming);
	DOREPLIFETIME(AFPSCharacter, IsRun);
	DOREPLIFETIME(AFPSCharacter, CurrentMoveState);
	DOREPLIFETIME(AFPSCharacter, SkinIndex);
}
void AFPSCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 떨어지기 시작하는 위치 저장
	if (GetCharacterMovement()->IsFalling() && !IsFalling)
	{
		IsFalling = true;
		FallingStartHeight = GetActorLocation().Z; // 떨어지 높이에 따른 데미지를 주기 위해 
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

	// 발전기 충전
	if (HasAuthority() && OverlappedGenerator && ChargeButtonPressed)
	{
		// 충전완료시 false 반환
		if (!OverlappedGenerator->AddChargingUser())
		{
			ChargeButtonPressed = false;
			OverlappedGenerator->ShowGenerateWidget(false);
			OverlappedGenerator = nullptr;
		}
	}
}
void AFPSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AFPSCharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AFPSCharacter::DoCrouch);
	PlayerInputComponent->BindAction("Run", IE_Pressed, this, &AFPSCharacter::Run);
	PlayerInputComponent->BindAction("Run", IE_Released, this, &AFPSCharacter::StopRun);
	PlayerInputComponent->BindAction("Aimming", IE_Pressed, this, &AFPSCharacter::Aimming);
	PlayerInputComponent->BindAction("Aimming", IE_Released, this, &AFPSCharacter::StopAimming);
	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &AFPSCharacter::EquipWeapon);
	PlayerInputComponent->BindAction("Drop", IE_Pressed, this, &AFPSCharacter::Drop);
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &AFPSCharacter::Fire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &AFPSCharacter::FireEnd);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AFPSCharacter::Reload);
	PlayerInputComponent->BindAction("Charge", IE_Pressed, this, &AFPSCharacter::Charge);
	PlayerInputComponent->BindAction("Charge", IE_Released, this, &AFPSCharacter::StopCharge);
	PlayerInputComponent->BindAction("Spotlight", IE_Pressed, this, &AFPSCharacter::StartSpotlight);
	PlayerInputComponent->BindAction("Map", IE_Pressed, this, &AFPSCharacter::OpenMap);
	PlayerInputComponent->BindAction("Map", IE_Released, this, &AFPSCharacter::CloseMap);

	PlayerInputComponent->BindAxis("MoveForward", this, &AFPSCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AFPSCharacter::MoveRight);
	PlayerInputComponent->BindAxis("LookUp", this, &AFPSCharacter::LookUp);
	PlayerInputComponent->BindAxis("LookRight", this, &AFPSCharacter::LookRight);
}
// 떨어진 높이에 따라 데미지 입기
void AFPSCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

	if (PrevMovementMode == EMovementMode::MOVE_Falling && GetCharacterMovement()->MovementMode == EMovementMode::MOVE_Walking)
	{
		IsFalling = false;

		FallingEndHeight = GetActorLocation().Z;
		float FallingDistance = FallingStartHeight - FallingEndHeight;

		if (FallingDistance > FallingLowDistance && FallingDistance < FallingMediumDistance)
		{
			Health = FMath::Clamp(Health - 5.f, 0.f, MaxHealth);
			UpdateHUDHealth();
		}
		else if (FallingDistance > FallingMediumDistance && FallingDistance < FallingHighDistance)
		{
			Health = FMath::Clamp(Health - 10.f, 0.f, MaxHealth);
			UpdateHUDHealth();
		}
		else if (FallingDistance > FallingHighDistance)
		{
			Health = FMath::Clamp(Health - 20.f, 0.f, MaxHealth);
			UpdateHUDHealth();
		}

		if (Health <= 0.f)
		{
			AMainGameMode* CharacterGameMode = GetWorld()->GetAuthGameMode<AMainGameMode>();
			ACharacterController* AttackerController = Cast<ACharacterController>(Controller);

			if (CharacterGameMode)
			{
				PlayerController = PlayerController == nullptr ? Cast<ACharacterController>(Controller) : PlayerController;

				if (PlayerController)
				{
					CharacterGameMode->PlayerEliminated(this, PlayerController, AttackerController);
				}
			}
		}
	}
}


// 서버에서 호출됨
void AFPSCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCursor)
{
	if (PlayerController == nullptr) return;
	if (PlayerController->GetCurrentMatchState() == MatchState::HumanWin || PlayerController->GetCurrentMatchState() == MatchState::MonsterWin) return;

	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);

	if (Health <= 0.f && !IsDead)
	{
		IsDead = true;

		AMainGameMode* CharacterGameMode = GetWorld()->GetAuthGameMode<AMainGameMode>();

		if (CharacterGameMode)
		{
			ACharacterController* AttackerController = Cast<ACharacterController>(InstigatorController);
			PlayerController = PlayerController == nullptr ? Cast<ACharacterController>(Controller) : PlayerController;

			if (AttackerController)
			{
				CharacterGameMode->PlayerEliminated(this, PlayerController, AttackerController);
			}
		}
	}

	// 칼에 맞았을 경우
	if (Cast<USlowEffectDamage>(DamageType))
	{
		GetWorldTimerManager().SetTimer(
			SlowSpeedTimer,
			this,
			&AFPSCharacter::SlowSpeedTimerEnd,
			SlowSpeedTimerDelay,
			false
		);

		SlowSpeed = SlowSpeedVariable;
		IsSlow = true;
		Client_GetSlowed(true);
	}
}
// 서버에서 호출됨
void AFPSCharacter::RecoveryHealth(float RecoverHelath)
{
	if (Health > 0.f)
	{
		Health = FMath::Clamp(Health + RecoverHelath, 0.f, MaxHealth);
	}
}
void AFPSCharacter::Client_GetSlowed_Implementation(bool bSlow)
{
	if (bSlow)
	{
		IsSlow = true;
		SlowSpeed = SlowSpeedVariable;
		GetCharacterMovement()->MaxWalkSpeed = GetCharacterMovement()->MaxWalkSpeed - SlowSpeed;
		Server_SpeedChange(CurrentMoveState);
	}
	else
	{
		IsSlow = false;
		// 뺏던 속도를 다시 더해줌
		GetCharacterMovement()->MaxWalkSpeed = GetCharacterMovement()->MaxWalkSpeed + SlowSpeed;
		SlowSpeed = 0.f;
		Server_SpeedChange(CurrentMoveState);
	}
}
void AFPSCharacter::SlowSpeedTimerEnd()
{
	SlowSpeed = 0.f;
	Client_GetSlowed(false);
}
void AFPSCharacter::UpdateHUDHealth()
{
	PlayerController = PlayerController == nullptr ? Cast<ACharacterController>(Controller) : PlayerController;

	if (PlayerController)
	{
		PlayerController->SetHUDHealth(Health, MaxHealth);
	}
}
void AFPSCharacter::OnRep_Health()
{
	UpdateHUDHealth();
}

// 스테미나 관리 함수
void AFPSCharacter::ManageStemina(float DeltaTime)
{
	PlayerController = PlayerController == nullptr ? Cast<ACharacterController>(Controller) : PlayerController;

	if (IsRun && GetCharacterMovement()->Velocity.Size() >= 400.f && Stemina > 0.f)
	{
		Stemina = FMath::Clamp(Stemina - 10.f * DeltaTime, 0, 100);

		if (PlayerController)
		{
			PlayerController->SetHUDStemina(Stemina, MaxStemina);
		}

		if (Stemina <= 0.f)
		{
			GetCharacterMovement()->MaxWalkSpeed = WalkSpeed - SlowSpeed;;
		}
	}
	else if(GetCharacterMovement()->Velocity.Size() < 400.f && Stemina < MaxStemina)
	{
		Stemina = FMath::Clamp(Stemina + 5.f * DeltaTime, 0, 100);

		if (PlayerController)
		{
			PlayerController->SetHUDStemina(Stemina, MaxStemina);
		}
	}
}


// 게임 중간에 나가기
void AFPSCharacter::ServerLeftGame_Implementation()
{
	AMainGameMode* MainGameMode = GetWorld()->GetAuthGameMode<AMainGameMode>();
	if (MainGameMode)
	{
		MainGameMode->PlayerLeftGame(this);
	}
}


// 입력에 대한 움직임 관련 함수들
void AFPSCharacter::MoveForward(float Value)
{
	if (Value == 0.f) return;

	const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
	const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X)); // YawRotation을 바탕으로 회전 행렬을 만든다음 x축 벡터값을 반환(방향)

	AddMovementInput(Direction, Value); // Direction 방향으로 Value만큼 이동
}
void AFPSCharacter::MoveRight(float Value)
{
	if (Value == 0.f) return;

	const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
	const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));

	AddMovementInput(Direction, Value);
}
void AFPSCharacter::LookUp(float Value)
{
	if (Value == 0.f) return;

	AddControllerPitchInput(Value);
}
void AFPSCharacter::LookRight(float Value)
{
	if (Value == 0.f) return;

	AddControllerYawInput(Value);
}
void AFPSCharacter::Jump()
{
	// 장전중일 경우 점프 못함
	if (CombatComponent && CombatComponent->CombatState == ECombatState::CS_Reloading) return;

	if (bIsCrouched)
	{
		DoCrouch();
	}

	if (!bIsCrouched)
	{
		Super::Jump();
	}
}
void AFPSCharacter::DoCrouch()
{
	if (!bIsCrouched)
	{
		// 앉을때 조준중이면
		if (CombatComponent->bAiming)
		{
			GetCharacterMovement()->MaxWalkSpeed = AimWalkSpeed - SlowSpeed;
			Server_SpeedChange(MoveStateHuman::EMS_Aim);
		}
		else
		{
			GetCharacterMovement()->MaxWalkSpeed = CrouchSpeed - SlowSpeed;
			Server_SpeedChange(MoveStateHuman::EMS_Crouch);
		}
		GetMesh()->SetRenderCustomDepth(false); // 해당 Mesh가 사용자 정의 깊이 버퍼에 참여하지 않도록 한다
		Crouch();
	}
	else
	{
		// 일어설때 조준중이면
		if (CombatComponent->bAiming)
		{
			GetCharacterMovement()->MaxWalkSpeed = AimWalkSpeed - SlowSpeed;
			Server_SpeedChange(MoveStateHuman::EMS_Aim);
		}
		else
		{
			GetCharacterMovement()->MaxWalkSpeed = WalkSpeed - SlowSpeed;
			Server_SpeedChange(MoveStateHuman::EMS_Walk);
		}
		GetMesh()->SetRenderCustomDepth(true); // 해당 Mesh가 사용자 정의 깊이 버퍼에 참여하도록 한다
		UnCrouch();
	}
}

// 발전기 충전
void AFPSCharacter::Charge()
{
	if(OverlappedGenerator)
	{
		// 충전하는 애니메이션 재생

		//충전되는 프로그래스바 보이게하기
		Server_SetCanCharge(true); // = ChargeButtonPressed = true;
		//OverlappedGenerator->ShowGenerateWidget(true);
		//OverlappedGenerator->AddChargingUser(1);
	}
}
void AFPSCharacter::StopCharge()
{
	if (OverlappedGenerator)
	{
		//ChargeButtonPressed = false;
		Server_SetCanCharge(false); // = ChargeButtonPressed = false;
		//OverlappedGenerator->ShowGenerateWidget(false);
		//OverlappedGenerator->AddChargingUser(-1);
	}
}

void AFPSCharacter::Run()
{
	if (Stemina <= 0.f) return;

	if (!CombatComponent->bAiming && !bIsCrouched) // 조준중이 아니고 서있을때만
	{
		GetCharacterMovement()->MaxWalkSpeed = RunSpeed - SlowSpeed;
		Server_SpeedChange(MoveStateHuman::EMS_Run);
	}
	else if(CombatComponent->bAiming && !bIsCrouched) // 조준중일 때 달리기를 누를경우 조준이 끝나면 자동으로 달리기 위함
	{
		LastMoveState = MoveStateHuman::EMS_Run;
	}
}
void AFPSCharacter::StopRun()
{
	if (!CombatComponent->bAiming && !bIsCrouched) // 조준중이 아니고 서있을때만
	{
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed - SlowSpeed;
		Server_SpeedChange(MoveStateHuman::EMS_Walk);
	}
	else if (CombatComponent->bAiming && !bIsCrouched)
	{
		LastMoveState = MoveStateHuman::EMS_Walk;
	}
}

// 총 조준
void AFPSCharacter::Aimming()
{
	if (!GetCharacterMovement()->IsFalling() && CurrentWeaponType != EWeaponType::ET_Knife && 
		CurrentWeaponType != EWeaponType::ET_Rocket)
	{
		CombatComponent->SetAiming(true);
		LastMoveState = CurrentMoveState;
		GetCharacterMovement()->MaxWalkSpeed = AimWalkSpeed - SlowSpeed;
		Server_SpeedChange(MoveStateHuman::EMS_Aim);
	}
}
void AFPSCharacter::StopAimming()
{
	if (!GetCharacterMovement()->IsFalling() && CurrentWeaponType != EWeaponType::ET_Knife && 
		CurrentWeaponType != EWeaponType::ET_Rocket && CombatComponent)
	{
		CombatComponent->SetAiming(false);

		switch (LastMoveState)
		{
		case MoveStateHuman::EMS_Walk:
			GetCharacterMovement()->MaxWalkSpeed = WalkSpeed - SlowSpeed;
			break;
		case MoveStateHuman::EMS_Crouch:
			GetCharacterMovement()->MaxWalkSpeed = CrouchSpeed - SlowSpeed;
			break;
		case MoveStateHuman::EMS_Run:
			GetCharacterMovement()->MaxWalkSpeed = RunSpeed - SlowSpeed;
			break;
		case MoveStateHuman::EMS_Aim:
			GetCharacterMovement()->MaxWalkSpeed = AimWalkSpeed - SlowSpeed;
			break;
		}

		Server_SpeedChange(LastMoveState);
	}
}

// 총 떨어뜨리기
void AFPSCharacter::Drop()
{
	// 장전중이면 총 안떨어뜨림
	if (CombatComponent->GetIsReloading()) return;

	PlayerController = PlayerController == nullptr ? Cast<ACharacterController>(Controller) : PlayerController;
	if (Controller)
	{
		OverlappedWeapon = nullptr;
		PlayerController->SetHUDWeaponType(EWeaponType::ET_Knife);
	}
	ServerDropped();
}
void AFPSCharacter::ServerDropped_Implementation()
{
	if (CombatComponent->EquippedWeapon)
	{
		CombatComponent->EquippedWeapon->DropWeapon();
		CombatComponent->EquippedWeapon = nullptr;
	}
}

// 총 발사
void AFPSCharacter::Fire()
{
	// 공중에 있거나 달리고 있으면 총 못쏨
	if (GetCharacterMovement()->IsFalling()) return;
	if (CurrentMoveState == MoveStateHuman::EMS_Run) return;

	PlayerController = PlayerController == nullptr ? Cast<ACharacterController>(Controller) : PlayerController;
	if (PlayerController)
	{
		// 채팅창이 활성화 되어 있지 않으면
		if (!PlayerController->LeftClick())
		{
			if (CombatComponent && CombatComponent->EquippedWeapon)
			{
				CombatComponent->Fire(true);
			}
		}
	}
}
void AFPSCharacter::PlayFireAnimation()
{
	if (CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance && FireMontage)
	{
		AnimInstance->Montage_Play(FireMontage);

		FName SectionName;

		switch (CombatComponent->EquippedWeapon->GetWeaponType())
		{
		case EWeaponType::ET_Rifle:
			if (IsAimming)
				SectionName = FName("RifleAim");
			else
				SectionName = FName("RifleHip");
			break;

		case EWeaponType::ET_Knife:
			// 애니메이션이 없어서 재생안한다 구하면 넣자
			break;


		case EWeaponType::ET_Gun:
			if (IsAimming)
				SectionName = FName("RifleAim");
			else
				SectionName = FName("RifleHip");
			break;

		case EWeaponType::ET_ShotGun:
			SectionName = FName("Shotgun_Hip");
			break;

		case EWeaponType::ET_Rocket:
			SectionName = FName("Shotgun_Hip");
			break;
		}

		// 해당영역으로 점프
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}
void AFPSCharacter::FireEnd()
{
	if (CombatComponent && CombatComponent->EquippedWeapon)
	{
		CombatComponent->Fire(false);
	}
}

// 총 재장전
void AFPSCharacter::Reload()
{
	// 공중에선 장전 못함
	if (GetCharacterMovement()->IsFalling()) return;

	if (CombatComponent && CombatComponent->EquippedWeapon && CurrentWeaponType != EWeaponType::ET_Rocket)
	{
		CombatComponent->Reload();
	}
}
void AFPSCharacter::PlayReloadAnimation()
{
	if (CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr || CurrentWeaponType == EWeaponType::ET_Rocket) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance && FireMontage)
	{
		AnimInstance->Montage_Play(FireMontage);

		FName SectionName;

		switch (CurrentWeaponType)
		{
		case EWeaponType::ET_Rifle:
			if (IsAimming)
				SectionName = FName("ReloadRifleAim");
			else
				SectionName = FName("ReloadRifleHip");
			break;

		case EWeaponType::ET_Rocket:
			// 애니메이션이 없어서 재생안한다 구하면 넣자
			break;

		case EWeaponType::ET_Gun:
			SectionName = FName("ReloadPistol");
			break;

		case EWeaponType::ET_ShotGun:
			if (IsAimming)
				SectionName = FName("ReloadShotgunAim");
			else
				SectionName = FName("ReloadShotgunHip");
			break;

		case EWeaponType::ET_Knife:
			break;
		}

		// 해당영역으로 점프
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

// 총 장착
void AFPSCharacter::EquipWeapon() // E 누르면 호출되는 함수
{
	if (GetLocalRole() == ENetRole::ROLE_Authority)
	{
		if (CombatComponent)
		{
			CombatComponent->EquipWeapon(OverlappedWeapon);
		}
	}
	else
	{
		ServerEquipped();
	}
}

// 손전등 켜기
void AFPSCharacter::StartSpotlight()
{
	OnSpotlight = !OnSpotlight;

	Spotlight->SetVisibility(OnSpotlight);
	ServerSpotlight(OnSpotlight);
}
void AFPSCharacter::ServerSpotlight_Implementation(bool bLight)
{
	OnSpotlight = bLight;
	MulticastSpotlight(bLight);
}
void AFPSCharacter::MulticastSpotlight_Implementation(bool bLight)
{
	OnSpotlight = bLight;
	Spotlight->SetVisibility(bLight);
}

// 지도 열기
void AFPSCharacter::OpenMap()
{
	PlayerController = PlayerController == nullptr ? Cast<ACharacterController>(Controller) : PlayerController;

	if (PlayerController)
	{
		PlayerController->ShowMap(true);
	}
}
// 지도 닫기
void AFPSCharacter::CloseMap()
{
	PlayerController = PlayerController == nullptr ? Cast<ACharacterController>(Controller) : PlayerController;

	if (PlayerController)
	{
		PlayerController->ShowMap(false);
	}
}



// 캐릭터가 죽었을때 관련 함수들
// 서버에서 호출된다
void AFPSCharacter::Elim(bool bPlyerLeftGame)
{
	if (CombatComponent->EquippedWeapon)
	{
		CombatComponent->EquippedWeapon->DropWeapon();
		CombatComponent->EquippedWeapon = nullptr;
	}

	Multicast_Elim(bPlyerLeftGame);
}
void AFPSCharacter::Multicast_Elim_Implementation(bool bPlayerLeftGame)
{
	bLeftGame = bPlayerLeftGame;
	
	// 죽는 애니메이션 재생하기
	IsDead = true;

	// 죽고나면 움직임 등을 막음
	GetCharacterMovement()->DisableMovement();
	if (CombatComponent)
	{
		CombatComponent->SetFireButtonPressed(false);
	}

	// 충돌 끄기
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	GetWorldTimerManager().SetTimer(
		ElimTimer,
		this,
		&AFPSCharacter::EndElime,
		1.5f
	);
}
// 죽고나면 서버에게 죽음을 알려준다
void AFPSCharacter::EndElime()
{
	AMainGameMode* CharacterGameMode = GetWorld()->GetAuthGameMode<AMainGameMode>();

	if (!bLeftGame && HasAuthority() && CharacterGameMode)
	{
		CharacterGameMode->RequestRespone(this, Cast<ACharacterController>(Controller), false);
	}
	else if (IsLocallyControlled() && bLeftGame)
	{
		OnLeftGame.Broadcast();
	}
}


// 무기 장착 관련
void AFPSCharacter::ServerEquipped_Implementation()
{
	if (CombatComponent)
	{
		CombatComponent->EquipWeapon(OverlappedWeapon);
	}
}
// AWeapon 클래스에서 무기가 캐릭터와 겹치면 불려지는 함수(서버에서 호출됨)
void AFPSCharacter::SetOverlappingWeapon(AWeapon* OverlappingWeapon)
{
	// Owner 한테만 복제되도록 설정됨
	if (IsLocallyControlled()) // 현재 클라이언트에서 조종중일때, 즉 다른 클라이언트와 겹치지 않게 하기 위해서
	{
		if (OverlappedWeapon != nullptr)
		{
			//CanCharge = false;
			OverlappedWeapon->ShowPickUpWidget(false);
		}
	}

	OverlappedWeapon = OverlappingWeapon;

	if (IsLocallyControlled()) // 현재 클라이언트에서 조종중일때
	{
		if (OverlappedWeapon != nullptr)
		{
			//CanCharge = true;
			OverlappedWeapon->ShowPickUpWidget(true);
		}
	}
}
void AFPSCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappedWeapon != nullptr)
	{
		OverlappedWeapon->ShowPickUpWidget(true);
	}

	if (LastWeapon != nullptr)
	{
		LastWeapon->ShowPickUpWidget(false);
	}
}


bool AFPSCharacter::GetIsEquippedWeapon()
{
	if (CombatComponent->EquippedWeapon)
	{
		return true;
	}
	
	return false;
}
AWeapon* AFPSCharacter::GetEquippedWeapon()
{
	if (CombatComponent == nullptr) return nullptr;
	return CombatComponent->GetEquippedWeapon();
}


// 발전기 관련 함수들
// 플레이어가 발전기에 접근하면 호출되는 함수 (서버에서만 호출됨)
void AFPSCharacter::OverlappingGenerator(class AGenerator* Generator)
{
	if (Generator == nullptr)
	{
		ChargeButtonPressed = false;
	}

	if (IsLocallyControlled())
	{
		if (Generator == nullptr && OverlappedGenerator != nullptr)
		{
			OverlappedGenerator->ShowGenerateWidget(false);
		}
	}

	OverlappedGenerator = Generator;

	if (IsLocallyControlled())
	{
		if (OverlappedGenerator != nullptr)
		{
			OverlappedGenerator->ShowGenerateWidget(true);
		}
	}
}
void AFPSCharacter::OnRep_OverlappingGenerator(AGenerator* Generator)
{
	// Generator : Replicate 되기전
	// OverlappedGenerator : Replicate에 의해 변경됨
	if (Generator == nullptr && OverlappedGenerator != nullptr)
	{
		OverlappedGenerator->ShowGenerateWidget(true);
	}
	else if(Generator != nullptr && OverlappedGenerator == nullptr)
	{
		Generator->ShowGenerateWidget(false);
	}
}
void AFPSCharacter::Server_SetCanCharge_Implementation(bool bCanCharge)
{
	ChargeButtonPressed = bCanCharge;
	//Generator->FullCharge();
}


void AFPSCharacter::Server_SpeedChange_Implementation(MoveStateHuman EMS)
{
	CurrentMoveState = EMS;

	switch (EMS)
	{
	case MoveStateHuman::EMS_Walk:
		IsRun = false;
		GetMesh()->SetRenderCustomDepth(true);
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed - SlowSpeed;
		break;
	case MoveStateHuman::EMS_Crouch:
		IsRun = false;
		GetMesh()->SetRenderCustomDepth(false);
		GetCharacterMovement()->MaxWalkSpeed = CrouchSpeed - SlowSpeed;
		break;
	case MoveStateHuman::EMS_Run:
		IsRun = true;
		GetCharacterMovement()->MaxWalkSpeed = RunSpeed - SlowSpeed;
		break;
	case MoveStateHuman::EMS_Aim:
		IsRun = false;
		GetCharacterMovement()->MaxWalkSpeed = AimWalkSpeed - SlowSpeed;
		break;
	}
}
void AFPSCharacter::OnRep_MovementState()
{
	switch (CurrentMoveState)
	{
	case MoveStateHuman::EMS_Walk:
		GetMesh()->SetRenderCustomDepth(true);
		break;
	case MoveStateHuman::EMS_Crouch:
		GetMesh()->SetRenderCustomDepth(false);
		break;
	case MoveStateHuman::EMS_Run:

		break;
	}
}


// 평소 브금
void AFPSCharacter::PlayIdleSound()
{
	Client_PlayIdleSound();
}
// 괴물이 근처에 있을때 브금
void AFPSCharacter::PlayChasingSound()
{
	Client_ChasingSound();
}
void AFPSCharacter::Client_PlayIdleSound_Implementation()
{
	if (IdleAudio && ChasingAudio)
	{
		ChasingAudio->Stop();
		IdleAudio->Play();
	}
}
void AFPSCharacter::Client_ChasingSound_Implementation()
{
	if (IdleAudio && ChasingAudio)
	{
		IdleAudio->Stop();
		ChasingAudio->Play();
	}
}


// 밟은 바닥의 Physics Material 타입을 반환한다
EPhysicalSurface AFPSCharacter::GetSurfaceType()
{
	FHitResult HitResult;
	const FVector Start = GetActorLocation();
	const FVector End = Start + FVector(0.f, 0.f, -400.f);
	FCollisionQueryParams QueryParams;
	QueryParams.bReturnPhysicalMaterial = true;

	GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECollisionChannel::ECC_Visibility, QueryParams);

	return UPhysicalMaterial::DetermineSurfaceType(HitResult.PhysMaterial.Get());
}