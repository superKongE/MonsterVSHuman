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
	bReplicates = true; // Property Replicate�� �ϰڴٴ� ��

	SetReplicates(true); // �� Actor�� Replicate �ϰڴٴ� ��

	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn; // ĳ���� ��ȯ ������ �浹�� ��� ��� ó���� ������ ����, �������Ʈ������ ����

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 420.f;
	CameraBoom->bUsePawnControlRotation = true;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	Camera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false; // Camera �� CameraBoom�� �پ������Ƿ� Camera�� ȸ���ϸ� ���� ȸ���Ѵ�


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
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true; // ĳ���� Ŭ������ �⺻���� ����� ��ũ���� ����� Ȱ��ȭ

	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("Combatcomponent"));
	CombatComponent->SetIsReplicated(true); // ������Ʈ�� Replicat ��Ű�� ���� ����

	ChatComponent = CreateDefaultSubobject<UChatComponent>(TEXT("ChatComponent"));

	NetUpdateFrequency = 66.f; // �ʴ� 66�� ���� ���ø�����Ʈ(������Ʈ) �Ѵ� 
	MinNetUpdateFrequency = 33.f; // �ʴ� �ּ� 33�� �̻� ������Ʈ

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


	// ���� Ÿ��, �Ѿ� ���� HUD ����
	PlayerController = PlayerController == nullptr ? Cast<ACharacterController>(Controller) : PlayerController;
	if (PlayerController)
	{
		PlayerController->SetHUDWeaponType(CurrentWeaponType);
		PlayerController->SetHUDKnife();
		PlayerController->OwnerCharacter = this;
	}
	

	// �κ񿡼� ������ ĳ���� ��Ų ������
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

	// ������ ���ܼ��� Ž���Ǳ� ���� ����
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
// ������Ʈ���� �ʱ�ȭ���� BeginPlay()�� ȣ��Ǳ����� 
// �� ĳ���Ͱ� ��ȯ�Ǳ����� �̸� ���� �ʱ�ȭ�۾�
void AFPSCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (CombatComponent)
	{
		// CombatComponent Ŭ������ friend �����س����Ƿ� private ������ ������ �����ϴ�
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

	// �������� �����ϴ� ��ġ ����
	if (GetCharacterMovement()->IsFalling() && !IsFalling)
	{
		IsFalling = true;
		FallingStartHeight = GetActorLocation().Z; // ������ ���̿� ���� �������� �ֱ� ���� 
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

	// ������ ����
	if (HasAuthority() && OverlappedGenerator && ChargeButtonPressed)
	{
		// �����Ϸ�� false ��ȯ
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
// ������ ���̿� ���� ������ �Ա�
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


// �������� ȣ���
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

	// Į�� �¾��� ���
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
// �������� ȣ���
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
		// ���� �ӵ��� �ٽ� ������
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

// ���׹̳� ���� �Լ�
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


// ���� �߰��� ������
void AFPSCharacter::ServerLeftGame_Implementation()
{
	AMainGameMode* MainGameMode = GetWorld()->GetAuthGameMode<AMainGameMode>();
	if (MainGameMode)
	{
		MainGameMode->PlayerLeftGame(this);
	}
}


// �Է¿� ���� ������ ���� �Լ���
void AFPSCharacter::MoveForward(float Value)
{
	if (Value == 0.f) return;

	const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
	const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X)); // YawRotation�� �������� ȸ�� ����� ������� x�� ���Ͱ��� ��ȯ(����)

	AddMovementInput(Direction, Value); // Direction �������� Value��ŭ �̵�
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
	// �������� ��� ���� ����
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
		// ������ �������̸�
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
		GetMesh()->SetRenderCustomDepth(false); // �ش� Mesh�� ����� ���� ���� ���ۿ� �������� �ʵ��� �Ѵ�
		Crouch();
	}
	else
	{
		// �Ͼ�� �������̸�
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
		GetMesh()->SetRenderCustomDepth(true); // �ش� Mesh�� ����� ���� ���� ���ۿ� �����ϵ��� �Ѵ�
		UnCrouch();
	}
}

// ������ ����
void AFPSCharacter::Charge()
{
	if(OverlappedGenerator)
	{
		// �����ϴ� �ִϸ��̼� ���

		//�����Ǵ� ���α׷����� ���̰��ϱ�
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

	if (!CombatComponent->bAiming && !bIsCrouched) // �������� �ƴϰ� ����������
	{
		GetCharacterMovement()->MaxWalkSpeed = RunSpeed - SlowSpeed;
		Server_SpeedChange(MoveStateHuman::EMS_Run);
	}
	else if(CombatComponent->bAiming && !bIsCrouched) // �������� �� �޸��⸦ ������� ������ ������ �ڵ����� �޸��� ����
	{
		LastMoveState = MoveStateHuman::EMS_Run;
	}
}
void AFPSCharacter::StopRun()
{
	if (!CombatComponent->bAiming && !bIsCrouched) // �������� �ƴϰ� ����������
	{
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed - SlowSpeed;
		Server_SpeedChange(MoveStateHuman::EMS_Walk);
	}
	else if (CombatComponent->bAiming && !bIsCrouched)
	{
		LastMoveState = MoveStateHuman::EMS_Walk;
	}
}

// �� ����
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

// �� ����߸���
void AFPSCharacter::Drop()
{
	// �������̸� �� �ȶ���߸�
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

// �� �߻�
void AFPSCharacter::Fire()
{
	// ���߿� �ְų� �޸��� ������ �� ����
	if (GetCharacterMovement()->IsFalling()) return;
	if (CurrentMoveState == MoveStateHuman::EMS_Run) return;

	PlayerController = PlayerController == nullptr ? Cast<ACharacterController>(Controller) : PlayerController;
	if (PlayerController)
	{
		// ä��â�� Ȱ��ȭ �Ǿ� ���� ������
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
			// �ִϸ��̼��� ��� ������Ѵ� ���ϸ� ����
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

		// �ش翵������ ����
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

// �� ������
void AFPSCharacter::Reload()
{
	// ���߿��� ���� ����
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
			// �ִϸ��̼��� ��� ������Ѵ� ���ϸ� ����
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

		// �ش翵������ ����
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

// �� ����
void AFPSCharacter::EquipWeapon() // E ������ ȣ��Ǵ� �Լ�
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

// ������ �ѱ�
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

// ���� ����
void AFPSCharacter::OpenMap()
{
	PlayerController = PlayerController == nullptr ? Cast<ACharacterController>(Controller) : PlayerController;

	if (PlayerController)
	{
		PlayerController->ShowMap(true);
	}
}
// ���� �ݱ�
void AFPSCharacter::CloseMap()
{
	PlayerController = PlayerController == nullptr ? Cast<ACharacterController>(Controller) : PlayerController;

	if (PlayerController)
	{
		PlayerController->ShowMap(false);
	}
}



// ĳ���Ͱ� �׾����� ���� �Լ���
// �������� ȣ��ȴ�
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
	
	// �״� �ִϸ��̼� ����ϱ�
	IsDead = true;

	// �װ��� ������ ���� ����
	GetCharacterMovement()->DisableMovement();
	if (CombatComponent)
	{
		CombatComponent->SetFireButtonPressed(false);
	}

	// �浹 ����
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	GetWorldTimerManager().SetTimer(
		ElimTimer,
		this,
		&AFPSCharacter::EndElime,
		1.5f
	);
}
// �װ��� �������� ������ �˷��ش�
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


// ���� ���� ����
void AFPSCharacter::ServerEquipped_Implementation()
{
	if (CombatComponent)
	{
		CombatComponent->EquipWeapon(OverlappedWeapon);
	}
}
// AWeapon Ŭ�������� ���Ⱑ ĳ���Ϳ� ��ġ�� �ҷ����� �Լ�(�������� ȣ���)
void AFPSCharacter::SetOverlappingWeapon(AWeapon* OverlappingWeapon)
{
	// Owner ���׸� �����ǵ��� ������
	if (IsLocallyControlled()) // ���� Ŭ���̾�Ʈ���� �������϶�, �� �ٸ� Ŭ���̾�Ʈ�� ��ġ�� �ʰ� �ϱ� ���ؼ�
	{
		if (OverlappedWeapon != nullptr)
		{
			//CanCharge = false;
			OverlappedWeapon->ShowPickUpWidget(false);
		}
	}

	OverlappedWeapon = OverlappingWeapon;

	if (IsLocallyControlled()) // ���� Ŭ���̾�Ʈ���� �������϶�
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


// ������ ���� �Լ���
// �÷��̾ �����⿡ �����ϸ� ȣ��Ǵ� �Լ� (���������� ȣ���)
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
	// Generator : Replicate �Ǳ���
	// OverlappedGenerator : Replicate�� ���� �����
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


// ��� ���
void AFPSCharacter::PlayIdleSound()
{
	Client_PlayIdleSound();
}
// ������ ��ó�� ������ ���
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


// ���� �ٴ��� Physics Material Ÿ���� ��ȯ�Ѵ�
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