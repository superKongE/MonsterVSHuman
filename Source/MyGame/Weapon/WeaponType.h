#pragma once

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	ET_Knife UMETA(DisplayName = "Knife"),
	ET_Rifle UMETA(DisplayName = "Rifle"),
	ET_Gun UMETA(DisplayName = "Gun"),
	ET_ShotGun UMETA(DisplayName = "ShotGun"),
	ET_Rocket UMETA(DisplayName = "Rocket")
};