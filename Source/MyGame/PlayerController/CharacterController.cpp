
#include "CharacterController.h"
#include "MultiplayerSessionsSubsystem.h"
#include "GameFramework/PlayerController.h"
#include "MyGame/Spectator/HumanSpectatorPawn.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Components/EditableText.h"
#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/EditableTextBox.h"
#include "Kismet/KismetStringLibrary.h"
#include "Containers/UnrealString.h"
#include "Fonts/SlateFontInfo.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"

#include "MyGame/HUD/CharacterOverlay.h"
#include "MyGame/HUD/CharacterHUD.h"
#include "MyGame/HUD/ReturnToMenu.h"
#include "MyGame/HUD/LobbyMenu.h"
#include "MyGame/HUD/LobbyPawnNameWidget.h"
#include "MyGame/HUD/MapWidget.h"
#include "MyGame/Character/FPSCharacter.h"
#include "MyGame/Character/FPSMonster.h"
#include "MyGame/PlayerState/CharacterState.h"
#include "MyGame/Lobby/LobbyPawn.h"
#include "MyGame/Lobby/LobbyHUD.h"
#include "MyGame/HumanGameInstance.h"
#include "MyGame/Weapon/WeaponType.h"
#include "MyGame/GameMode/MainGameMode.h"
#include "MyGame/LobbyGameMode.h"
#include "MyGame/Announcement.h"

void ACharacterController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// �� ���ӽ���
	if (!IsLobby)
	{
		SetShowMouseCursor(false);
		SetInputMode(FInputModeGameOnly());
	}
	// �κ�
	else
	{
		SetInputMode(FInputModeGameOnly());
		SetShowMouseCursor(true);
	}
}
void ACharacterController::SetHumanInputMode()
{
	// �� ���ӽ���
	if (!IsLobby)
	{
		SetShowMouseCursor(false);
		SetInputMode(FInputModeGameOnly());
	}
	// �κ�
	else
	{
		SetInputMode(FInputModeGameOnly());
		SetShowMouseCursor(true);
	}
}
void ACharacterController::BeginPlay()
{
	Super::BeginPlay();
	CharacterHUD = Cast<ACharacterHUD>(GetHUD());
}
void ACharacterController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ACharacterController, MatchState);
	DOREPLIFETIME(ACharacterController, ServerTime);
	DOREPLIFETIME(ACharacterController, LobbyReady);
	DOREPLIFETIME(ACharacterController, IsLobby);
	DOREPLIFETIME(ACharacterController, LevelStartingTime);
}
// �÷��̾� ��Ʈ�ѷ��� ����Ʈ�� ��Ʈ��ũ�� ������ �ǰ� ���� ȣ��Ǵ� �Լ�
void ACharacterController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	if (IsLocalController())
	{
		// ������ Ŭ���̾�Ʈ���� �ð����̸� ���Ѵ�
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}
void ACharacterController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TimeSyncRunningTime += DeltaTime;
	// TimeSyncFrequency(5��) ���� ������ Ŭ���̾�Ʈ�� �ð� ���̸� ����� ����ȭ
	if (IsLocalController())
	{
		CheckTimeSync(DeltaTime);
		//ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}

	SetHUDTime();

	CharacterHUD = CharacterHUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : CharacterHUD;
	PlayerState = PlayerState == nullptr ? GetPlayerState<ACharacterState>() : PlayerState;
	if (GetLocalRole() == ENetRole::ROLE_AutonomousProxy && PlayerState && CharacterHUD && CharacterHUD->CharacterOverlay && CharacterHUD->CharacterOverlay->PingText)
	{
		FString PingText = FString::Printf(TEXT("%d ms"), PlayerState->GetPing());
		CharacterHUD->CharacterOverlay->PingText->SetText(FText::FromString(PingText));
	}

	// ���� 50�� ������ ��� ǥ��
	CheckPing(DeltaTime);
}
void ACharacterController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAction("Quit", IE_Pressed, this, &ACharacterController::ShowReturnToMainMenu);
	InputComponent->BindAction("ShowChat", IE_Pressed, this, &ACharacterController::ShowChat);
}


// Client���� GameInstance�� ����� �̸��� ������
void ACharacterController::Client_SetUserName_Implementation()
{
	HumanGameInstance = HumanGameInstance == nullptr ? Cast<UHumanGameInstance>(GetGameInstance()) : HumanGameInstance;
	if (HumanGameInstance)
	{
		Server_SetUserName(HumanGameInstance->GetUserName());
	}
}
// ������ �ִ� PlayerState�� ����
void ACharacterController::Server_SetUserName_Implementation(const FString& name)
{
	ACharacterState* CharacterState = Cast<ACharacterState>(PlayerState);
	if (CharacterState)
	{
		CharacterState->Server_SetUserName(name);
	}
}


// SeamlessTravel�� ���� �̵��� ���͸� �����ϴ� �Լ�
// ���ο� Level�� �̵��Ҷ� �ڵ����� ȣ��Ǵ� �Լ�
void ACharacterController::GetSeamlessTravelActorList(bool bToEntry, TArray<class AActor*>& ActorList)
{
	/*if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			15.f,
			FColor::Red,
			FString(TEXT("Travel"))
		);
	}*/
	//ActorList.Add(this);

	Super::GetSeamlessTravelActorList(bToEntry, ActorList);
}


// ���� �޴� ǥ��
void ACharacterController::ShowReturnToMainMenu()
{
	if (ReturnToMainMenuWidget == nullptr) return;
	if (ReturnToMenu == nullptr)
	{
		// ����� ReturnToMenu::Initialize�� �ڵ�ȣ�� �ȴ�
		ReturnToMenu = CreateWidget<UReturnToMenu>(this, ReturnToMainMenuWidget);
	}
	if (ReturnToMenu)
	{
		bReturnToMenuOpen = !bReturnToMenuOpen; 

		if (bReturnToMenuOpen)
		{
			// �޴� ǥ��
			if (IsLobby)
			{
				ReturnToMenu->MenuSetup(true);
			}
			else
			{
				ReturnToMenu->MenuSetup(false);
			}
		}
		else
		{
			// �޴� ����
			ReturnToMenu->MenuTearDown();
			ReturnToMenu = nullptr;

			if (IsLobby)
			{
				SetShowMouseCursor(true);
			}
			else
			{
				SetShowMouseCursor(false);
				FInputModeGameOnly InputModeData;
				SetInputMode(InputModeData);
			}
		}
	}
}


// HUD �׸��� �Լ���
// ���, ������ ���� ǥ�õǴ� UI�� �ٸ��� �Ѵ�
void ACharacterController::SetHUDVisible()
{
	CharacterHUD = CharacterHUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : CharacterHUD;
	HumanGameInstance = HumanGameInstance = nullptr ? GetGameInstance<UHumanGameInstance>() : HumanGameInstance;

	if (CharacterHUD && Cast<AFPSCharacter>(GetCharacter()))
	{	
		if (CharacterHUD->CharacterOverlay && CharacterHUD->CharacterOverlay->DashAmount && CharacterHUD->CharacterOverlay->DashBar)
		{
			CharacterHUD->CharacterOverlay->DashAmount->SetVisibility(ESlateVisibility::Hidden);
			CharacterHUD->CharacterOverlay->DashBar->SetVisibility(ESlateVisibility::Hidden);
			CharacterHUD->CharacterOverlay->ThrowCoolTimeBar->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}
void ACharacterController::SetHUDHealth(float Health, float MaxHealth)
{
	if (!IsLocalController()) return;
	// HUD�� ���� Ŭ���̾�Ʈ������ �����Ѵ�
	// ���������� ��� �������� �÷����ϴ� ĳ���ʹ� HUD�� ������ �����Ѵ�
	CharacterHUD = CharacterHUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : CharacterHUD;

	if (CharacterHUD && CharacterHUD->CharacterOverlay && CharacterHUD->CharacterOverlay->HealthBar && CharacterHUD->CharacterOverlay->HealthText)
	{
		const float HealthPercent = Health / MaxHealth;
		CharacterHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent); // ���α׷����� �� ����(ü�� ��)
		FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		CharacterHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText)); // ���α׷����ٿ� �ִ� �ؽ�Ʈ �� �ؽ�Ʈ ����
	}
}
void ACharacterController::SetHUDStemina(float Stemina, float MaxStemina)
{
	if (!IsLocalController()) return;

	CharacterHUD = CharacterHUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : CharacterHUD;

	if (CharacterHUD && CharacterHUD->CharacterOverlay && CharacterHUD->CharacterOverlay->SteminaAmount && CharacterHUD->CharacterOverlay->SteminaBar)
	{
		const float SteminaPercent = Stemina / MaxStemina;

		CharacterHUD->CharacterOverlay->SteminaBar->SetPercent(SteminaPercent); // ���α׷����� �� ����(ü�� ��)
		FString SteminaText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Stemina), FMath::CeilToInt(MaxStemina));
		CharacterHUD->CharacterOverlay->SteminaAmount->SetText(FText::FromString(SteminaText)); // ���α׷����ٿ� �ִ� �ؽ�Ʈ �� �ؽ�Ʈ ����
	}
}
void ACharacterController::SetHUDDash(float Dash, float MaxDash)
{
	if (!IsLocalController()) return;

	CharacterHUD = CharacterHUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : CharacterHUD;

	if (CharacterHUD && CharacterHUD->CharacterOverlay && CharacterHUD->CharacterOverlay->DashAmount && CharacterHUD->CharacterOverlay->DashBar)
	{
		const float DashPercent = Dash / MaxDash;

		CharacterHUD->CharacterOverlay->DashBar->SetPercent(DashPercent); // ���α׷����� �� ����(ü�� ��)
		FString DashText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Dash), FMath::CeilToInt(MaxDash));
		CharacterHUD->CharacterOverlay->DashAmount->SetText(FText::FromString(DashText)); // ���α׷����ٿ� �ִ� �ؽ�Ʈ �� �ؽ�Ʈ ����
	}
}
void ACharacterController::SetHUDScore(float ScoreAmount)
{
	if (!IsLocalController()) return;

	CharacterHUD = CharacterHUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : CharacterHUD;

	if (CharacterHUD && CharacterHUD->CharacterOverlay && CharacterHUD->CharacterOverlay->ScoreAmount)
	{
		FString Text = FString::Printf(TEXT("%d"), FMath::CeilToInt(ScoreAmount));
		CharacterHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(Text));
	}
}
void ACharacterController::SetHUDDefeat(int32 DefeatAmount)
{
	if (!IsLocalController()) return;

	CharacterHUD = CharacterHUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : CharacterHUD;

	if (CharacterHUD && CharacterHUD->CharacterOverlay && CharacterHUD->CharacterOverlay->DefeatAmount)
	{
		FString Text = FString::Printf(TEXT("%d"), DefeatAmount);
		CharacterHUD->CharacterOverlay->DefeatAmount->SetText(FText::FromString(Text));
	}
}
void ACharacterController::SetHUDAmmo(int32 AmmoAmount)
{
	if (!IsLocalController()) return;

	CharacterHUD = CharacterHUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : CharacterHUD;

	if (CharacterHUD && CharacterHUD->CharacterOverlay && CharacterHUD->CharacterOverlay->AmmoAmount)
	{
		FString AmmoText = FString::Printf(TEXT("%d / "), AmmoAmount);
		CharacterHUD->CharacterOverlay->AmmoAmount->SetText(FText::FromString(AmmoText));
	}
}
void ACharacterController::SetHUDKnife()
{
	if (!IsLocalController()) return;

	CharacterHUD = CharacterHUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : CharacterHUD;
	if (CharacterHUD && CharacterHUD->CharacterOverlay && CharacterHUD->CharacterOverlay->AmmoAmount && CharacterHUD->CharacterOverlay->CarriedAmmoAmount)
	{
		FString AmmoText = FString::Printf(TEXT(""));
		FString CarriedAmmoText = FString::Printf(TEXT(""));
		CharacterHUD->CharacterOverlay->AmmoAmount->SetText(FText::FromString(AmmoText));
		CharacterHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(CarriedAmmoText));
	}
}
void ACharacterController::SetHUDCarriedAmmo(int32 CarriedAmmoAmount)
{
	if (!IsLocalController()) return;

	CharacterHUD = CharacterHUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : CharacterHUD;

	if (CharacterHUD && CharacterHUD->CharacterOverlay && CharacterHUD->CharacterOverlay->CarriedAmmoAmount)
	{
		FString CarriedAmmoText = FString::Printf(TEXT("%d"), CarriedAmmoAmount);
		CharacterHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(CarriedAmmoText));
	}
}
void ACharacterController::SetHUDWeaponType(EWeaponType WeaponType)
{
	if (!IsLocalController()) return;

	CharacterHUD = CharacterHUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : CharacterHUD;

	if (CharacterHUD && CharacterHUD->CharacterOverlay && CharacterHUD->CharacterOverlay->WeaponType)
	{
		FString WeapnTypeText;

		switch (WeaponType)
		{
		case EWeaponType::ET_Rifle:
			WeapnTypeText = FString::Printf(TEXT("Rifle"));
			break;
		case EWeaponType::ET_Knife:
			WeapnTypeText = FString::Printf(TEXT("Knife"));
			SetHUDKnife();
			break;
		case EWeaponType::ET_Rocket:
			WeapnTypeText = FString::Printf(TEXT("Rocket"));
			break;
		case EWeaponType::ET_ShotGun:
			WeapnTypeText = FString::Printf(TEXT("Shotgun"));
			break;
		case EWeaponType::ET_Gun:
			WeapnTypeText = FString::Printf(TEXT("Gun"));
			break;
		}

		CharacterHUD->CharacterOverlay->WeaponType->SetText(FText::FromString(WeapnTypeText));
	}
}
void ACharacterController::SetThrowCoolTimeBarHUD(float CoolTime)
{
	if (!IsLocalController()) return;
	// HUD�� ���� Ŭ���̾�Ʈ������ �����Ѵ�
	// ���������� ��� �������� �÷����ϴ� ĳ���ʹ� HUD�� ������ �����Ѵ�
	CharacterHUD = CharacterHUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : CharacterHUD;

	if (CharacterHUD && CharacterHUD->CharacterOverlay && CharacterHUD->CharacterOverlay->ThrowCoolTimeBar)
	{
		const float CoolTimePercent = CoolTime / 100.f;
		CharacterHUD->CharacterOverlay->ThrowCoolTimeBar->SetPercent(CoolTimePercent); // ���α׷����� �� ����(ü�� ��)
	}
}
void ACharacterController::ShowMap(bool bShow)
{
	if (!IsLocalController()) return;

	CharacterHUD = CharacterHUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : CharacterHUD;

	if (CharacterHUD && CharacterHUD->MapWidget && CharacterHUD->MapWidget->MapImage)
	{
		if (bShow)
		{
			CharacterHUD->MapWidget->MapImage->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			CharacterHUD->MapWidget->MapImage->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}


// �ð� ǥ��
void ACharacterController::SetHUDMatchCountDown(float Time)
{
	if (!IsLocalController()) return;

	CharacterHUD = CharacterHUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : CharacterHUD;
	if (CharacterHUD && CharacterHUD->CharacterOverlay && CharacterHUD->CharacterOverlay->MatchTime)
	{
		int32 Minute = FMath::FloorToInt(Time / 60.f);
		int32 Second = Time - Minute * 60;

		FString MatchCountTimeText = FString::Printf(TEXT("%02d : %02d"), Minute, Second);
		CharacterHUD->CharacterOverlay->MatchTime->SetText(FText::FromString(MatchCountTimeText));
	}
}
void ACharacterController::SetHUDTime()
{
	uint32 SecondsLeft = FMath::CeilToInt(GetServerTime() - LevelStartingTime);

	MainGameMode = MainGameMode == nullptr ? GetWorld()->GetAuthGameMode<AMainGameMode>() : MainGameMode;

	if (CountdownInt != SecondsLeft)
	{
		SetHUDMatchCountDown(GetServerTime() - LevelStartingTime);
	}

	CountdownInt = SecondsLeft;
}


// �� ���� �Լ���
void ACharacterController::HighPingWarning()
{
	CharacterHUD = CharacterHUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : CharacterHUD;
	if (CharacterHUD && CharacterHUD->CharacterOverlay && CharacterHUD->CharacterOverlay->HighPingImage && CharacterHUD->CharacterOverlay->HighPingAnimation)
	{
		// ���̰� ����
		CharacterHUD->CharacterOverlay->HighPingImage->SetOpacity(1.f);
		// �ִϸ��̼� ���																				���� ���� �������, ��� �ݺ�����
		CharacterHUD->CharacterOverlay->PlayAnimation(CharacterHUD->CharacterOverlay->HighPingAnimation, 0.f, 5);
	}
}
void ACharacterController::StopHighPingWarning()
{
	CharacterHUD = CharacterHUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : CharacterHUD;
	if (CharacterHUD && CharacterHUD->CharacterOverlay && CharacterHUD->CharacterOverlay->HighPingImage && CharacterHUD->CharacterOverlay->HighPingAnimation)
	{
		// �Ⱥ��̰� ����
		CharacterHUD->CharacterOverlay->HighPingImage->SetOpacity(0.f);
		// �ִϸ��̼��� ������̸� ����� �����
		if (CharacterHUD->CharacterOverlay->IsAnimationPlaying(CharacterHUD->CharacterOverlay->HighPingAnimation))
		{
			CharacterHUD->CharacterOverlay->StopAnimation(CharacterHUD->CharacterOverlay->HighPingAnimation);
		}
	}
}
// �� üũ �ϸ鼭 �޽��� ���� ����
void ACharacterController::CheckPing(float DeltaTime)
{
	HighPingRunningTime += DeltaTime;
	if (HighPingRunningTime >= CheckPingFrequncy)
	{
		PlayerState = PlayerState == nullptr ? GetPlayerState<ACharacterState>() : PlayerState;
		if (PlayerState)
		{
			// GetPing() �� ���� ������ ���� �������� ���� 4�� ������ �����ؼ� ������ �����̴�(Ÿ���� uint8�̱� ����)
			if (PlayerState->GetPing() * 4 > HighPingThreshold)
			{
				HighPingWarning();
				PingAnimationRunningTime = 0.f;
			}
		}

		HighPingRunningTime = 0.f;
	}

	if (CharacterHUD && CharacterHUD->CharacterOverlay && CharacterHUD->CharacterOverlay->HighPingAnimation && CharacterHUD->CharacterOverlay->IsAnimationPlaying(CharacterHUD->CharacterOverlay->HighPingAnimation))
	{
		PingAnimationRunningTime += DeltaTime;
		if (PingAnimationRunningTime >= HighPingDuration)
		{
			StopHighPingWarning();
		}
	}
}


// ������ Ŭ���̾�Ʈ���� �ð� ����ȭ ���� �Լ���
void ACharacterController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();

	//       Ŭ���̾�Ʈ�� ServerRPC�� ȣ���� �ð�, ���� ���� �ð�
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}
// Ŭ���̾�Ʈ�� ������ �ð� ���̸� ���
void ACharacterController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerReceivedClientRequest)
{
	// Ŭ���̾�Ʈ�� ������ ȣ���ؼ� �ٽ� Ŭ���̾�Ʈ���� ������� �պ��ð�
	// �պ����� �ð� - �պ� ���� �ð� = �պ��� �ð�
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;

	// ���� �����ð� = ServerRPC�� ȣ������� ���� �ð� + ����->Ŭ���̾�Ʈ���� �ɸ� �ð�
	float CurrentServerTime = TimeServerReceivedClientRequest + (RoundTripTime * 0.5f);

	// ������ Ŭ���̾�Ʈ���� �ð� ����
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}
// ���� �����ð� ��������
float ACharacterController::ACharacterController::GetServerTime()
{
	// ������ �����ð� �״�� ��ȯ
	if (HasAuthority()) return GetWorld()->GetTimeSeconds();
	else return GetWorld()->GetTimeSeconds() + ClientServerDelta;

	//return LastServerTime = GetWorld()->GetTimeSeconds() + ClientServerDelta;
}
// Ư���ð� ���� ������ Ŭ���̾�Ʈ�� �ð� ���̸� ����ȭ
void ACharacterController::CheckTimeSync(float DeltaTime)
{
	if (IsLocalController() && TimeSyncRunningTime >= TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}


// gamemode(����)���� ȣ��Ǵ� �Լ�
void ACharacterController::OnMatchStateSet(FName State)
{
	MainGameMode = MainGameMode == nullptr ? Cast<AMainGameMode>(UGameplayStatics::GetGameMode(this)) : MainGameMode;
	if (MainGameMode == nullptr) return;

	MatchState = State;

	if (MatchState == MatchState::InProgress)
	{
		LevelStartingTime = MainGameMode->GetLevelStartingTime();

		//GetWorldTimerManager().SetTimer(ReadyTimer, this, &ACharacterController::HandleReady, 1.f, true);
	}
	else if (MatchState == MatchState::HumanWin)
	{
		GetWorldTimerManager().SetTimer(HumanWinTimer, this, &ACharacterController::HumanWin, 1.f, true);
	}
	else if (MatchState == MatchState::MonsterWin)
	{
		GetWorldTimerManager().SetTimer(MonsterWinTimer, this, &ACharacterController::MonsterWin, 1.f, true);
	}
	else if (MatchState == MatchState::GameStart)
	{
		GameStart = true;
	}
	else if (MatchState == MatchState::Night)
	{
		GameStart = true;

		if (AmIMonster)
		{
			MonsterPlayer = MonsterPlayer == nullptr ? Cast<AFPSMonster>(GetPawn()) : MonsterPlayer;
			if (MonsterPlayer)
			{
				MonsterPlayer->ChangeToNight();
			}
		}
	}
	else if (MatchState == MatchState::Morning)
	{
		GameStart = true;

		if (AmIMonster)
		{
			MonsterPlayer = MonsterPlayer == nullptr ? Cast<AFPSMonster>(GetPawn()) : MonsterPlayer;
			if (MonsterPlayer)
			{
				MonsterPlayer->ChangeToMorning();
			}
		}
	}
}
void ACharacterController::OnRep_MatchState()
{
	if (MatchState == MatchState::InProgress)
	{
	}
	else if (MatchState == MatchState::HumanWin)
	{
		GetWorldTimerManager().SetTimer(HumanWinTimer, this, &ACharacterController::HumanWin, 1.f, true);
	}
	else if (MatchState == MatchState::MonsterWin)
	{
		GetWorldTimerManager().SetTimer(MonsterWinTimer, this, &ACharacterController::MonsterWin, 1.f, true);
	}
	else if (MatchState == MatchState::GameStart)
	{
		GameStart = true;
	}
	else if (MatchState == MatchState::Night)
	{
		GameStart = true;
	}
	else if (MatchState == MatchState::Morning)
	{
		GameStart = true;
	}
}


// ��� ����
void ACharacterController::HumanWin()
{
	CharacterHUD = CharacterHUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : CharacterHUD;

	if (CharacterHUD)
	{
		// CharacterOverlay ����
		CharacterHUD->CharacterOverlay->RemoveFromParent();

		if (CharacterHUD->Announcment && CharacterHUD->Announcment->AnnouncementText)
		{
			CooldownTime--;
			CharacterHUD->Announcment->SetVisibility(ESlateVisibility::Visible);
			FString AnnouncementText = FString::Printf(TEXT("Human Win\n    %d"), CooldownTime);
			CharacterHUD->Announcment->AnnouncementText->SetText(FText::FromString(AnnouncementText));

			if (CooldownTime <= 0)
			{
				GetWorldTimerManager().ClearTimer(HumanWinTimer);

				if (IsLocalPlayerController())
				{
					LeftGame();
				}
			}
		}
	}
}
// ������ ������ ������ ��û
void ACharacterController::LeftGame()
{
	UGameInstance* GameInstance = GetGameInstance();
	MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
	
	if (GameInstance)
	{
		if (MultiplayerSessionsSubsystem)
		{
			// MultiplayerSessionsSubsystem �� Ŀ���� ��������Ʈ�� ���� �ı� ��û�Ϸ�� ȣ��� �ݹ��Լ� ���ε�
			MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &ACharacterController::GameFinished);
		}
	}

	if (MultiplayerSessionsSubsystem)
	{
		// ���� �ı�
		MultiplayerSessionsSubsystem->DestroySession();
	}
}
void ACharacterController::GameFinished(bool bWasSuccessful)
{
	if (!bWasSuccessful)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (World)
	{
		AGameModeBase* GameMode = World->GetAuthGameMode<AGameModeBase>();
		// �����ϰ��
		if (GameMode)
		{
			// �޴��� ���ư���
			GameMode->ReturnToMainMenuHost();
		}
		// Ŭ���̾�Ʈ�� ���
		else
		{
			ClientReturnToMainMenuWithTextReason(FText());
		}
	}
}
void ACharacterController::MonsterWin()
{
	CharacterHUD = CharacterHUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : CharacterHUD;

	if (CharacterHUD)
	{
		// CharacterOverlay ����
		CharacterHUD->CharacterOverlay->RemoveFromParent();

		if (CharacterHUD->Announcment && CharacterHUD->Announcment->AnnouncementText)
		{
			CooldownTime--;
			CharacterHUD->Announcment->SetVisibility(ESlateVisibility::Visible);
			FString AnnouncementText = FString::Printf(TEXT("Monster Win\n    %d"), CooldownTime);
			CharacterHUD->Announcment->AnnouncementText->SetText(FText::FromString(AnnouncementText));
		}

		if (CooldownTime <= 0)
		{
			GetWorldTimerManager().ClearTimer(MonsterWinTimer);
			if (IsLocalPlayerController())
			{
				LeftGame();
			}
		}
	}
}


// �κ񿡼� ����ϴ� �Լ���
// �ΰ����� ���������� ���� UI�� �ٸ��� ǥ��
void ACharacterController::SetLobbyHUD()
{
	if (!IsLocalController()) return;

	LobbyHUD = LobbyHUD == nullptr ? Cast<ALobbyHUD>(GetHUD()) : LobbyHUD;
	HumanGameInstance = HumanGameInstance == nullptr ? GetGameInstance<UHumanGameInstance>() : HumanGameInstance;

	if (HumanGameInstance->GetAmIMonster())
	{
		if (LobbyHUD && LobbyHUD->LobbyMenu->ReadyButton && LobbyHUD->LobbyMenu->LeftButton && LobbyHUD->GetLobbyMenu()->RightButton)
		{
			LobbyHUD->LobbyMenu->LeftButton->SetVisibility(ESlateVisibility::Hidden);
			LobbyHUD->LobbyMenu->RightButton->SetVisibility(ESlateVisibility::Hidden);
			LobbyHUD->LobbyMenu->ReadyButton->SetVisibility(ESlateVisibility::Hidden);
		}
	}
	else
	{
		if (LobbyHUD && LobbyHUD->LobbyMenu->StartButton)
		{
			LobbyHUD->LobbyMenu->StartButton->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	LobbyHUD->GetLobbyMenu()->SetVisibility(ESlateVisibility::Visible);
}
// �غ� �Ϸ� ��ư ������ ȣ��Ǵ� �Լ�
void ACharacterController::SetReadyPlayerCount()
{
	if (!LobbyReady)
	{
		Server_SetIsLobby(true);
		LobbyReady = true;
		Server_SetReadyPlayerCount(true);
	}
	else
	{
		Server_SetIsLobby(false);
		LobbyReady = false;
		Server_SetReadyPlayerCount(false);
	}
}
void ACharacterController::Server_SetIsLobby_Implementation(bool bLobby)
{
	LobbyReady = bLobby;
}
void ACharacterController::StartGame()
{
	Server_StartGame();
}
void ACharacterController::Server_SetReadyPlayerCount_Implementation(bool bReady)
{
	ALobbyGameMode* LobbyGameMode = GetWorld()->GetAuthGameMode<ALobbyGameMode>();
	if (LobbyGameMode && bReady)
	{
		LobbyGameMode->SetReadyPlayerCount(true);
	}
	else if(LobbyGameMode && !bReady)
	{
		LobbyGameMode->SetReadyPlayerCount(false);
	}
}
void ACharacterController::Server_StartGame_Implementation()
{
	ALobbyGameMode* LobbyGameMode = GetWorld()->GetAuthGameMode<ALobbyGameMode>();
	if (LobbyGameMode)
	{
		LobbyGameMode->StartGameBeforeDestoryLobbyHUD();
	}
}
// �κ� UI ����
void ACharacterController::DestroyLobbyHUD()
{
	ClientDestroyLobbyHUD();
}
void ACharacterController::ClientDestroyLobbyHUD_Implementation()
{
	LobbyHUD->LobbyMenu->SetVisibility(ESlateVisibility::Hidden);
	LobbyHUD->Destroy();

	Server_DestroyLobbyHUDComplete();
}
// LobbyHUD�� �ı����� LobbyGameMode(����)�� �˷��ش�
void ACharacterController::Server_DestroyLobbyHUDComplete_Implementation()
{
	ALobbyGameMode* LobbyGameMode = GetWorld()->GetAuthGameMode<ALobbyGameMode>();
	if (LobbyGameMode)
	{
		LobbyGameMode->DestroyLobbyHUDCompleted();
	}
}


// ä�� ���� �Լ���
void ACharacterController::ShowChat()
{
	if (!IsLocalController() || IsOpenChatBox) return;

	if (IsLobby)
	{
		IsOpenChatBox = true;
		LobbyShowChatBoxHUD();
	}
	else
	{
		IsOpenChatBox = true;
		ShowChatBoxHUD();

	}
}
// ä��â�� Ȱ��ȭ�� ä�� �ٸ����� ��Ŭ���ϸ� �ٽ� ĳ���͸� �����ϰ� �Ѵ�
bool ACharacterController::LeftClick()
{
	if (IsOpenChatBox)
	{
		IsOpenChatBox = false;
		CloseChatBoxHUD();

		return true;
	}

	return false;
}
// ä��â ����
void ACharacterController::ShowChatBoxHUD()
{
	if (!IsLocalController()) return;

	CharacterHUD = CharacterHUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : CharacterHUD;

	if (CharacterHUD && CharacterHUD->ChatBox && CharacterHUD->ChatBox->WriteMessage)
	{
		FInputModeGameAndUI InputModeData;
		InputModeData.SetHideCursorDuringCapture(true);
		InputModeData.SetWidgetToFocus(CharacterHUD->ChatBox->WriteMessage->TakeWidget());
		SetInputMode(InputModeData);
		SetShowMouseCursor(false);
	}
}
// ä��â �ݱ�
void ACharacterController::CloseChatBoxHUD()
{
	if (!IsLocalController()) return;

	CharacterHUD = CharacterHUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : CharacterHUD;
	if (CharacterHUD && CharacterHUD->ChatBox)
	{
		IsOpenChatBox = false;
		SetInputMode(FInputModeGameOnly());
		SetShowMouseCursor(false);
	}
}
// �޽��� ������
void ACharacterController::SendMessage()
{
	if (!IsLocalController()) return;

	CharacterHUD = CharacterHUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : CharacterHUD;
	HumanGameInstance = HumanGameInstance == nullptr ? GetGameInstance<UHumanGameInstance>() : HumanGameInstance;
	if (HumanGameInstance && CharacterHUD && CharacterHUD->ChatBox && CharacterHUD->ChatBox->WriteMessage)
	{
		FString Message = CharacterHUD->ChatBox->WriteMessage->GetText().ToString();
		if (Message.Len() > 0)
		{
			Server_SendMessage(HumanGameInstance->GetUserName() + " : " + Message, IsTeamChat);
			CharacterHUD->ChatBox->WriteMessage->SetText(FText::FromString(""));
			FInputModeGameAndUI InputModeData;
			InputModeData.SetWidgetToFocus(CharacterHUD->ChatBox->WriteMessage->TakeWidget());
			SetInputMode(InputModeData);
		}
		// �Է��� �޽����� ������ ä��â ����
		else
		{
			CloseChatBoxHUD();
		}
	}
}
// ä��â�� �޽��� ǥ���ϱ�
void ACharacterController::SetChatBoxHUDMessage(const FString& Message)
{
	if (!IsLocalController()) return;

	CharacterHUD = CharacterHUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : CharacterHUD;

	if (CharacterHUD && CharacterHUD->ChatBox && CharacterHUD->ChatBox->ScrollBox)
	{
		UEditableTextBox* TempTextBlock = NewObject<UEditableTextBox>(CharacterHUD->ChatBox->ScrollBox);
		TempTextBlock->SetForegroundColor(FLinearColor(0.f, 0.f, 0.f, 0.3f));
		TempTextBlock->WidgetStyle.Font.Size = 20;
		TempTextBlock->bIsEnabled = false;
		TempTextBlock->SetText(FText::FromString(Message));
		CharacterHUD->ChatBox->ScrollBox->AddChild(TempTextBlock);
		CharacterHUD->ChatBox->ScrollBox->ScrollToEnd(); // ��ũ���� ���� �Ʒ��� ����
	}
}
void ACharacterController::Server_SendMessage_Implementation(const FString& Message, bool IsTeam)
{
	TArray<AActor*> Actors;
	// �� ä��
	if (IsTeam)
	{
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFPSCharacter::StaticClass(), Actors);

		for (AActor* Actor : Actors)
		{
			AFPSCharacter* Human = Cast<AFPSCharacter>(Actor);
			if (ACharacterController * PlayerController = Human->GetPlayerController())
			{
				PlayerController->Client_SendMessage(Message);
			}
		}
	}
	// ��ü ä��
	else
	{
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacterController::StaticClass(), Actors);
		for (AActor* Actor : Actors)
		{
			if (ACharacterController* Controller = Cast<ACharacterController>(Actor))
			{
				Controller->Client_SendMessage(Message);
			}
		}
	}
}
void ACharacterController::Client_SendMessage_Implementation(const FString& Message)
{
	SetChatBoxHUDMessage(Message);
}
// ä�� �޽��� �Է� ���� ����
void ACharacterController::MakeMessage()
{
	if (!IsLocalController()) return;

	CharacterHUD = CharacterHUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : CharacterHUD;
	if (CharacterHUD && CharacterHUD->ChatBox && CharacterHUD->ChatBox->WriteMessage)
	{
		FString Text = CharacterHUD->ChatBox->WriteMessage->GetText().ToString();
		// �޽��� ���̰� ���� ���̸� �Ѿ��
		if (Text.Len() > MessageLengthLimit)
		{
			// ó������ Ư�� ���̱��� �ڸ� ������ �޽����� ���
			Text = UKismetStringLibrary::GetSubstring(Text, 0, MessageLengthLimit);
			CharacterHUD->ChatBox->WriteMessage->SetText(FText::FromString(Text));
		}
	}
}


// �κ� ä��
// ä��â ����
void ACharacterController::LobbyShowChatBoxHUD()
{
	if (!IsLocalController()) return;
	
	LobbyHUD = LobbyHUD == nullptr ? Cast<ALobbyHUD>(GetHUD()) : LobbyHUD;

	if (LobbyHUD && LobbyHUD->LobbyMenu && LobbyHUD->LobbyMenu->WriteMessage)
	{
		// Game �Է�(�÷��̾�, �÷��̾� ��Ʈ�ѷ�) �� UI �Է��� �Ѵ� �޴� ���
		FInputModeGameAndUI InputModeData;
		InputModeData.SetWidgetToFocus(LobbyHUD->LobbyMenu->WriteMessage->TakeWidget());
		SetInputMode(InputModeData);
		SetShowMouseCursor(true);
	}
}
// ä��â �ݱ�
void ACharacterController::LobbyCloseChatBoxHUD()
{
	if (!IsLocalController()) return;

	LobbyHUD = LobbyHUD == nullptr ? Cast<ALobbyHUD>(GetHUD()) : LobbyHUD;
	if (LobbyHUD && LobbyHUD->LobbyMenu)
	{
		IsOpenChatBox = false;
		// Game �Է�(�÷��̾�, �÷��̾� ��Ʈ�ѷ�)�� �޴´�
		SetInputMode(FInputModeGameOnly());
		SetShowMouseCursor(true);
	}
}
void ACharacterController::LobbySendMessage()
{
	if (!IsLocalController()) return;

	LobbyHUD = LobbyHUD == nullptr ? Cast<ALobbyHUD>(GetHUD()) : LobbyHUD;
	HumanGameInstance = HumanGameInstance == nullptr ? GetGameInstance<UHumanGameInstance>() : HumanGameInstance;
	if (HumanGameInstance && LobbyHUD && LobbyHUD->LobbyMenu && LobbyHUD->LobbyMenu->WriteMessage)
	{
		FString Message = LobbyHUD->LobbyMenu->WriteMessage->GetText().ToString();
		if (Message.Len() > 0)
		{
			LobbyServer_SendMessage(HumanGameInstance->GetUserName() + " : " + Message);
			LobbyHUD->LobbyMenu->WriteMessage->SetText(FText::FromString(""));
			FInputModeGameAndUI InputModeData;
			InputModeData.SetWidgetToFocus(LobbyHUD->LobbyMenu->WriteMessage->TakeWidget());
			SetInputMode(InputModeData);
		}
		else
		{
			LobbyCloseChatBoxHUD();
		}
	}
}
// ��ũ�� �ڽ��� �޽��� �߰�
void ACharacterController::LobbySetChatBoxHUDMessage(const FString& Message)
{
	if (!IsLocalController()) return;

	LobbyHUD = LobbyHUD == nullptr ? Cast<ALobbyHUD>(GetHUD()) : LobbyHUD;

	if (LobbyHUD && LobbyHUD->LobbyMenu && LobbyHUD->LobbyMenu->ScrollBox)
	{
		UEditableTextBox* TempTextBlock = NewObject<UEditableTextBox>(LobbyHUD->LobbyMenu->ScrollBox);
		TempTextBlock->SetForegroundColor(FLinearColor(0.f, 0.f, 0.f, 0.9f));
		TempTextBlock->WidgetStyle.Font.Size = 20;
		TempTextBlock->SetText(FText::FromString(Message));
		TempTextBlock->SetIsEnabled(false);
		LobbyHUD->LobbyMenu->ScrollBox->AddChild(TempTextBlock);
		
		LobbyHUD->LobbyMenu->ScrollBox->ScrollToEnd(); // ��ũ���� ���� �Ʒ��� ����
	}
}
void ACharacterController::LobbyServer_SendMessage_Implementation(const FString& Message)
{
	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACharacterController::StaticClass(), Actors);
	for (AActor* Actor : Actors)
	{
		if (ACharacterController* Controller = Cast<ACharacterController>(Actor))
		{
			Controller->LobbyClient_SendMessage(Message);
		}
	}
}
void ACharacterController::LobbyClient_SendMessage_Implementation(const FString& Message)
{
	LobbySetChatBoxHUDMessage(Message);
}
// ä�� ���� ����
void ACharacterController::LobbyMakeMessage()
{
	if (!IsLocalController()) return;

	LobbyHUD = LobbyHUD == nullptr ? Cast<ALobbyHUD>(GetHUD()) : LobbyHUD;
	if (LobbyHUD && LobbyHUD->LobbyMenu && LobbyHUD->LobbyMenu->WriteMessage)
	{
		FString Text = LobbyHUD->LobbyMenu->WriteMessage->GetText().ToString();
		if (Text.Len() > MessageLengthLimit)
		{
			Text = UKismetStringLibrary::GetSubstring(Text, 0, MessageLengthLimit);
			LobbyHUD->LobbyMenu->WriteMessage->SetText(FText::FromString(Text));
		}
	}
}



void ACharacterController::BroadcastElim(ACharacterState* Attacker, ACharacterState* Victim)
{
	ClientElimAnnouncement(Attacker, Victim);
}
void ACharacterController::ClientElimAnnouncement_Implementation(ACharacterState* Attacker, ACharacterState* Victim)
{
	ACharacterState* Self = GetPlayerState<ACharacterState>();
	if (Attacker && Victim && Self)
	{
		CharacterHUD = CharacterHUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : CharacterHUD;
		if (CharacterHUD)
		{
			CharacterHUD->AddElimAnnouncement(Attacker->GetUserName(), Victim->GetUserName());
		}
	}
}


// ���� ĳ���� ��ȯ ��û(�������� ȣ��)
void ACharacterController::SpawnPlayerIsMonster()
{
	AMainGameMode* CharacterGameMode = GetWorld()->GetAuthGameMode<AMainGameMode>();
	if (CharacterGameMode == nullptr) return;

	if (AmIMonster)
	{
		// ���� ĳ���� ��ȯ��û
		CharacterGameMode->SpawnMonsterPlayer(this);
	}
	else
	{
		// �ΰ� ĳ���� ��ȯ��û
		CharacterGameMode->SpawnHumanPlayer(this);
	}
}


// ClientRPC�� �̿��Ͽ� Client�� GameInstance�� �������� �ƴ��� ����Ѵ�
void ACharacterController::SetAmIMonster(bool bMonster)
{
	AmIMonster = bMonster;
	if (bMonster)
		SetClientMonster();

	else
		SetClientHuman();
}
void ACharacterController::SetClientMonster_Implementation()
{
	AmIMonster = true;

	HumanGameInstance = HumanGameInstance == nullptr ? Cast<UHumanGameInstance>(GetGameInstance()) : HumanGameInstance;
	if (HumanGameInstance)
		HumanGameInstance->SetAmIMonster(true);
}
void ACharacterController::SetClientHuman_Implementation()
{
	AmIMonster = false;

	HumanGameInstance = HumanGameInstance == nullptr ? Cast<UHumanGameInstance>(GetGameInstance()) : HumanGameInstance;
	if (HumanGameInstance)
		HumanGameInstance->SetAmIMonster(false);
}

void ACharacterController::SetIsLobby(bool bLobby)
{
	IsLobby = bLobby;

	Client_SetIsLobby(bLobby);
}
void ACharacterController::Client_SetIsLobby_Implementation(bool bLobby)
{
	IsLobby = bLobby;
}