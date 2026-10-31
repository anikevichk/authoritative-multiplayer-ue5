// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MTeamStatics.generated.h"

/**
 *
 */
UCLASS()
class UMTeamStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintPure, Category = "Teams")
	static FORCEINLINE uint8 GetPlayerCountPerTeam() { return PlayerCountPerTeam; }

	UFUNCTION(BlueprintPure, Category = "Teams")
	static FORCEINLINE uint8 GetTeamForSlot(uint8 SlotID) { return SlotID / PlayerCountPerTeam; }

	UFUNCTION(BlueprintPure, Category = "Teams")
	static FORCEINLINE uint8 GetSlotCount() { return PlayerCountPerTeam * 2; }

	UFUNCTION(BlueprintPure, Category = "Teams")
	static FORCEINLINE bool IsValidSlot(uint8 SlotID) { return SlotID < GetSlotCount(); }

private:
	static constexpr uint8 PlayerCountPerTeam = 5;
};
