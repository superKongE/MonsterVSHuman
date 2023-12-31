
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

	// 본 게임시작
	if (!IsLobby)
	{
		SetShowMouseCursor(false);
		SetInputMode(FInputModeGameOnly());
	}
	// 로비
	else
	{
		SetInputMode(FInputModeGameOnly());
		SetShowMouseCursor(true);
	}
}
void ACharacterController::SetHumanInputMode()
{
	// 본 게임시작
	if (!IsLobby)
	{
		SetShowMouseCursor(false);
		SetInputMode(FInputModeGameOnly());
	}
	// 로비
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
// 플레이어 컨트롤러가 뷰포트와 네트워크에 연결이 되고 나면 호출되는 함수
void ACharacterController::ReceivedPlayer()
{
	Super::ReceivedPlayer();

	if (IsLocalController())
	{
		// 서버와 클라이언트간의 시간차이를 구한다
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}
void ACharacterController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TimeSyncRunningTime += DeltaTime;
	// TimeSyncFrequency(5초) 마다 서버와 클라이언트의 시간 차이를 계산해 동기화
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

	// 핑이 50을 넘으면 경고 표시
	CheckPing(DeltaTime);
}
void ACharacterController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InputComponent->BindAction("Quit", IE_Pressed, this, &ACharacterController::ShowReturnToMainMenu);
	InputComponent->BindAction("ShowChat", IE_Pressed, this, &ACharacterController::ShowChat);
}


// Client에서 GameInstance에 저장된 이름을 가져와
void ACharacterController::Client_SetUserName_Implementation()
{
	HumanGameInstance = HumanGameInstance == nullptr ? Cast<UHumanGameInstance>(GetGameInstance()) : HumanGameInstance;
	if (HumanGameInstance)
	{
		Server_SetUserName(HumanGameInstance->GetUserName());
	}
}
// 서버에 있는 PlayerState에 전달
void ACharacterController::Server_SetUserName_Implementation(const FString& name)
{
	ACharacterState* CharacterState = Cast<ACharacterState>(PlayerState);
	if (CharacterState)
	{
		CharacterState->Server_SetUserName(name);
	}
}


// SeamlessTravel시 같이 이동할 액터를 지정하는 함수
// 새로운 Level로 이동할때 자동으로 호출되는 함수
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


// 메인 메뉴 표시
void ACharacterController::ShowReturnToMainMenu()
{
	if (ReturnToMainMenuWidget == nullptr) return;
	if (ReturnToMenu == nullptr)
	{
		// 여기사 ReturnToMenu::Initialize가 자동호출 된다
		ReturnToMenu = CreateWidget<UReturnToMenu>(this, ReturnToMainMenuWidget);
	}
	if (ReturnToMenu)
	{
		bReturnToMenuOpen = !bReturnToMenuOpen; 

		if (bReturnToMenuOpen)
		{
			// 메뉴 표시
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
			// 메뉴 제거
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


// HUD 그리는 함수들
// 사람, 괴물에 따라 표시되는 UI를 다르게 한다
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
	// HUD는 오직 클라이언트에서만 존재한다
	// 리슨서버일 경우 서버에서 플레이하는 캐릭터는 HUD가 서버에 존재한다
	CharacterHUD = CharacterHUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : CharacterHUD;

	if (CharacterHUD && CharacterHUD->CharacterOverlay && CharacterHUD->CharacterOverlay->HealthBar && CharacterHUD->CharacterOverlay->HealthText)
	{
		const float HealthPercent = Health / MaxHealth;
		CharacterHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent); // 프로그레스바 값 변경(체력 바)
		FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		CharacterHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText)); // 프로그레스바에 있는 텍스트 블럭 텍스트 변경
	}
}
void ACharacterController::SetHUDStemina(float Stemina, float MaxStemina)
{
	if (!IsLocalController()) return;

	CharacterHUD = CharacterHUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : CharacterHUD;

	if (CharacterHUD && CharacterHUD->CharacterOverlay && CharacterHUD->CharacterOverlay->SteminaAmount && CharacterHUD->CharacterOverlay->SteminaBar)
	{
		const float SteminaPercent = Stemina / MaxStemina;

		CharacterHUD->CharacterOverlay->SteminaBar->SetPercent(SteminaPercent); // 프로그레스바 값 변경(체력 바)
		FString SteminaText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Stemina), FMath::CeilToInt(MaxStemina));
		CharacterHUD->CharacterOverlay->SteminaAmount->SetText(FText::FromString(SteminaText)); // 프로그레스바에 있는 텍스트 블럭 텍스트 변경
	}
}
void ACharacterController::SetHUDDash(float Dash, float MaxDash)
{
	if (!IsLocalController()) return;

	CharacterHUD = CharacterHUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : CharacterHUD;

	if (CharacterHUD && CharacterHUD->CharacterOverlay && CharacterHUD->CharacterOverlay->DashAmount && CharacterHUD->CharacterOverlay->DashBar)
	{
		const float DashPercent = Dash / MaxDash;

		CharacterHUD->CharacterOverlay->DashBar->SetPercent(DashPercent); // 프로그레스바 값 변경(체력 바)
		FString DashText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Dash), FMath::CeilToInt(MaxDash));
		CharacterHUD->CharacterOverlay->DashAmount->SetText(FText::FromString(DashText)); // 프로그레스바에 있는 텍스트 블럭 텍스트 변경
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
	// HUD는 오직 클라이언트에서만 존재한다
	// 리슨서버일 경우 서버에서 플레이하는 캐릭터는 HUD가 서버에 존재한다
	CharacterHUD = CharacterHUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : CharacterHUD;

	if (CharacterHUD && CharacterHUD->CharacterOverlay && CharacterHUD->CharacterOverlay->ThrowCoolTimeBar)
	{
		const float CoolTimePercent = CoolTime / 100.f;
		CharacterHUD->CharacterOverlay->ThrowCoolTimeBar->SetPercent(CoolTimePercent); // 프로그레스바 값 변경(체력 바)
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


// 시간 표시
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


// 핑 관련 함수들
void ACharacterController::HighPingWarning()
{
	CharacterHUD = CharacterHUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : CharacterHUD;
	if (CharacterHUD && CharacterHUD->CharacterOverlay && CharacterHUD->CharacterOverlay->HighPingImage && CharacterHUD->CharacterOverlay->HighPingAnimation)
	{
		// 보이게 설정
		CharacterHUD->CharacterOverlay->HighPingImage->SetOpacity(1.f);
		// 애니메이션 재생																				몇초 부터 재생할지, 몇번 반복할지
		CharacterHUD->CharacterOverlay->PlayAnimation(CharacterHUD->CharacterOverlay->HighPingAnimation, 0.f, 5);
	}
}
void ACharacterController::StopHighPingWarning()
{
	CharacterHUD = CharacterHUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : CharacterHUD;
	if (CharacterHUD && CharacterHUD->CharacterOverlay && CharacterHUD->CharacterOverlay->HighPingImage && CharacterHUD->CharacterOverlay->HighPingAnimation)
	{
		// 안보이게 설정
		CharacterHUD->CharacterOverlay->HighPingImage->SetOpacity(0.f);
		// 애니메이션이 재생중이면 재생을 멈춘다
		if (CharacterHUD->CharacterOverlay->IsAnimationPlaying(CharacterHUD->CharacterOverlay->HighPingAnimation))
		{
			CharacterHUD->CharacterOverlay->StopAnimation(CharacterHUD->CharacterOverlay->HighPingAnimation);
		}
	}
}
// 핑 체크 하면서 메시지 도배 관리
void ACharacterController::CheckPing(float DeltaTime)
{
	HighPingRunningTime += DeltaTime;
	if (HighPingRunningTime >= CheckPingFrequncy)
	{
		PlayerState = PlayerState == nullptr ? GetPlayerState<ACharacterState>() : PlayerState;
		if (PlayerState)
		{
			// GetPing() 을 통해 얻어오는 값을 서버에서 값을 4로 나누어 압축해서 보내기 때문이다(타입이 uint8이기 때문)
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


// 서버와 클라이언트간의 시간 동기화 관련 함수들
void ACharacterController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();

	//       클라이언트가 ServerRPC를 호출한 시간, 현재 서버 시간
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}
// 클라이언트와 서버의 시간 차이를 계산
void ACharacterController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerReceivedClientRequest)
{
	// 클라이언트가 서버를 호출해서 다시 클라이언트한태 오기까지 왕복시간
	// 왕복한후 시간 - 왕복 시작 시간 = 왕복한 시간
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;

	// 현재 서버시간 = ServerRPC가 호출됐을때 서버 시간 + 서버->클라이언트까지 걸린 시간
	float CurrentServerTime = TimeServerReceivedClientRequest + (RoundTripTime * 0.5f);

	// 서버와 클라이언트간의 시간 차이
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}
// 현재 서버시간 가져오기
float ACharacterController::ACharacterController::GetServerTime()
{
	// 서버면 서버시간 그대로 반환
	if (HasAuthority()) return GetWorld()->GetTimeSeconds();
	else return GetWorld()->GetTimeSeconds() + ClientServerDelta;

	//return LastServerTime = GetWorld()->GetTimeSeconds() + ClientServerDelta;
}
// 특정시간 마다 서버와 클라이언트의 시간 차이를 동기화
void ACharacterController::CheckTimeSync(float DeltaTime)
{
	if (IsLocalController() && TimeSyncRunningTime >= TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}


// gamemode(서버)에서 호출되는 함수
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


// 경기 끝남
void ACharacterController::HumanWin()
{
	CharacterHUD = CharacterHUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : CharacterHUD;

	if (CharacterHUD)
	{
		// CharacterOverlay 제거
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
// 게임이 끝나서 나가기 요청
void ACharacterController::LeftGame()
{
	UGameInstance* GameInstance = GetGameInstance();
	MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
	
	if (GameInstance)
	{
		if (MultiplayerSessionsSubsystem)
		{
			// MultiplayerSessionsSubsystem 의 커스템 델리게이트에 세션 파괴 요청완료시 호출될 콜백함수 바인딩
			MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &ACharacterController::GameFinished);
		}
	}

	if (MultiplayerSessionsSubsystem)
	{
		// 세션 파괴
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
		// 서버일경우
		if (GameMode)
		{
			// 메뉴로 돌아가기
			GameMode->ReturnToMainMenuHost();
		}
		// 클라이언트일 경우
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
		// CharacterOverlay 제거
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


// 로비에서 사용하는 함수들
// 인간인지 괴물인지에 따라 UI를 다르게 표시
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
// 준비 완료 버튼 누르면 호출되는 함수
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
// 로비 UI 제거
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
// LobbyHUD를 파괴됨을 LobbyGameMode(서버)에 알려준다
void ACharacterController::Server_DestroyLobbyHUDComplete_Implementation()
{
	ALobbyGameMode* LobbyGameMode = GetWorld()->GetAuthGameMode<ALobbyGameMode>();
	if (LobbyGameMode)
	{
		LobbyGameMode->DestroyLobbyHUDCompleted();
	}
}


// 채팅 관련 함수들
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
// 채팅창이 활성화된 채로 다른곳을 좌클릭하면 다시 캐릭터를 조종하게 한다
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
// 채팅창 열기
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
// 채팅창 닫기
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
// 메시지 보내기
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
		// 입력한 메시지가 없으면 채팅창 닫음
		else
		{
			CloseChatBoxHUD();
		}
	}
}
// 채팅창에 메시지 표시하기
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
		CharacterHUD->ChatBox->ScrollBox->ScrollToEnd(); // 스크롤을 가장 아래로 내림
	}
}
void ACharacterController::Server_SendMessage_Implementation(const FString& Message, bool IsTeam)
{
	TArray<AActor*> Actors;
	// 팀 채팅
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
	// 전체 채팅
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
// 채팅 메시지 입력 길이 제한
void ACharacterController::MakeMessage()
{
	if (!IsLocalController()) return;

	CharacterHUD = CharacterHUD == nullptr ? Cast<ACharacterHUD>(GetHUD()) : CharacterHUD;
	if (CharacterHUD && CharacterHUD->ChatBox && CharacterHUD->ChatBox->WriteMessage)
	{
		FString Text = CharacterHUD->ChatBox->WriteMessage->GetText().ToString();
		// 메시지 길이가 일정 길이를 넘어가면
		if (Text.Len() > MessageLengthLimit)
		{
			// 처음부터 특정 길이까지 자른 문장을 메시지로 사용
			Text = UKismetStringLibrary::GetSubstring(Text, 0, MessageLengthLimit);
			CharacterHUD->ChatBox->WriteMessage->SetText(FText::FromString(Text));
		}
	}
}


// 로비 채팅
// 채팅창 열기
void ACharacterController::LobbyShowChatBoxHUD()
{
	if (!IsLocalController()) return;
	
	LobbyHUD = LobbyHUD == nullptr ? Cast<ALobbyHUD>(GetHUD()) : LobbyHUD;

	if (LobbyHUD && LobbyHUD->LobbyMenu && LobbyHUD->LobbyMenu->WriteMessage)
	{
		// Game 입력(플레이어, 플레이어 컨트롤러) 과 UI 입력을 둘다 받는 모드
		FInputModeGameAndUI InputModeData;
		InputModeData.SetWidgetToFocus(LobbyHUD->LobbyMenu->WriteMessage->TakeWidget());
		SetInputMode(InputModeData);
		SetShowMouseCursor(true);
	}
}
// 채팅창 닫기
void ACharacterController::LobbyCloseChatBoxHUD()
{
	if (!IsLocalController()) return;

	LobbyHUD = LobbyHUD == nullptr ? Cast<ALobbyHUD>(GetHUD()) : LobbyHUD;
	if (LobbyHUD && LobbyHUD->LobbyMenu)
	{
		IsOpenChatBox = false;
		// Game 입력(플레이어, 플레이어 컨트롤러)만 받는다
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
// 스크롤 박스에 메시지 추가
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
		
		LobbyHUD->LobbyMenu->ScrollBox->ScrollToEnd(); // 스크롤을 가장 아래로 내림
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
// 채팅 길이 제한
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


// 유저 캐릭터 소환 요청(서버에서 호출)
void ACharacterController::SpawnPlayerIsMonster()
{
	AMainGameMode* CharacterGameMode = GetWorld()->GetAuthGameMode<AMainGameMode>();
	if (CharacterGameMode == nullptr) return;

	if (AmIMonster)
	{
		// 괴물 캐릭터 소환요청
		CharacterGameMode->SpawnMonsterPlayer(this);
	}
	else
	{
		// 인간 캐릭터 소환요청
		CharacterGameMode->SpawnHumanPlayer(this);
	}
}


// ClientRPC를 이용하여 Client의 GameInstance에 몬스터인지 아닌지 기록한다
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