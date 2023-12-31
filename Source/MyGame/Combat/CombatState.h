#pragma once

UENUM(BlueprintType)
enum class ECombatState : uint8
{
	CS_Reloading UMETA(DisplayName = "Reloading"),
	CS_Idle UMETA(DisplayName = "Idle"),
};