// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MNetStatics.generated.h"

/**
 * 
 */
UCLASS()
class UMNetStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintPure, Category = "Teams")
	static uint8 GetPlayerCountPerTeam();

	UFUNCTION(BlueprintCallable, Category = "Teams")
	static void SetPlayerCountPerTeam(uint8 NewValue);

	// Slots 0..PlayerCountPerTeam-1 are team 0, the next PlayerCountPerTeam slots are team 1
	UFUNCTION(BlueprintPure, Category = "Teams")
	static uint8 GetTeamForSlot(uint8 SlotID);

private:
	static uint8 PlayerCountPerTeam;
};
