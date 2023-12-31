// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameState.h"
#include "MyGame/Character/FPSCharacter.h"
#include "MyGame/Character/FPSMonster.h"

void AMyGameState::AddHumanArray(AFPSCharacter* Human)
{
	HumanArray.Add(Human);
}
void AMyGameState::AddMonsterArray(AFPSMonster* Monster)
{
	MonsterArray.Add(Monster);
}


// 다음에 관전할 플레이어를 반환하는 함수
AFPSCharacter* AMyGameState::GetSpectatorNextHumanPlayer()
{
	int i = 0;

	while (i < HumanArray.Num())
	{
		Humanindex = (Humanindex + 1) % HumanArray.Num();
		i++;
		if (!HumanArray[Humanindex]->GetIsDead()) break;
	}

	return HumanArray[Humanindex];
}
// 몬스터 플레이어 반환
AFPSMonster* AMyGameState::GetSpectatorMonsterPlayer()
{
	int i = 0;

	while (i < MonsterArray.Num())
	{
		MonsterIndex = (MonsterIndex + 1) % MonsterArray.Num();
		i++;
		if (!HumanArray[MonsterIndex]->GetIsDead()) break;
	}

	return MonsterArray[MonsterIndex];
}