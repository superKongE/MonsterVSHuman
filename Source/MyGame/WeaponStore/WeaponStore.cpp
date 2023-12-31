// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponStore.h"

AWeaponStore::AWeaponStore()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

void AWeaponStore::BeginPlay()
{
	Super::BeginPlay();
	
}

void AWeaponStore::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AWeaponStore::Open()
{

}

