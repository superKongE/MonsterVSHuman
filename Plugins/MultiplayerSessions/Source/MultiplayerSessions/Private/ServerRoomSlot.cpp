// Fill out your copyright notice in the Description page of Project Settings.


#include "ServerRoomSlot.h"
#include "MultiplayerSessionsSubsystem.h"


void UServerRoomSlot::ServerRoomSlot_JoinButtonClicked()
{
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
		if (MultiplayerSessionsSubsystem)
		{
			// MultiplayerSessionsSubsystem �� Ŀ���� ��������Ʈ�� ���� �ı� ��û�Ϸ�� ȣ��� �ݹ��Լ� ���ε�
			MultiplayerSessionsSubsystem->JoinSession(ArrayIndex);
		}
	}
}