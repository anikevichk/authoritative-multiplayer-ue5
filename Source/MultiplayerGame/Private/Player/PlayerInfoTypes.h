// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayerInfoTypes.generated.h"

class APlayerState;
USTRUCT()

struct FPlayerSelection {
	GENERATED_BODY()
public:
	FPlayerSelection();
	FPlayerSelection(uint8 ValidSlot, const APlayerState* PlayerState);

	FORCEINLINE void SetSlot(uint8 NewSlot){Slot = NewSlot;}
	FORCEINLINE uint8 GetSlot() const {return Slot;}
	FORCEINLINE FUniqueNetIdRepl GetPlayerUniqueID() const {return PlayerUniqueID;}
	FORCEINLINE FString GetPlayerNickname() const {return PlayerName;}

	bool IsForPlayer(const APlayerState* PlayerState) const;
	bool HasValidPlayerAndSlot() const;

	uint8 InvalidSlot = 255;
private:
	UPROPERTY()
	uint8 Slot;

	UPROPERTY()
	FUniqueNetIdRepl PlayerUniqueID;

	UPROPERTY()
	FString PlayerName;
};