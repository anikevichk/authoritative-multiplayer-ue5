// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/PlayerInfoTypes.h"
#include "GameFramework/PlayerState.h"
#include "Framework/MTeamStatics.h"

FPlayerSelection::FPlayerSelection()
	:Slot{InvalidSlot}, PlayerUniqueID{FUniqueNetIdRepl::Invalid()}, PlayerName {} {
	/** Creates an empty selection with an invalid slot. */
}

FPlayerSelection::FPlayerSelection(uint8 ValidSlot, const APlayerState* PlayerState):Slot{ValidSlot} {
	/** Creates a selection of the given slot for a player. */
	if (PlayerState) {
		PlayerUniqueID = PlayerState->GetUniqueId();
		PlayerName = PlayerState->GetPlayerName();
	}
}

bool FPlayerSelection::BelongsToPlayer(const APlayerState* PlayerState) const {
	/** Returns true if the selection belongs to the player, compared by unique ID. */
	if (!PlayerState) return false;
	return PlayerState->GetUniqueId() == GetPlayerUniqueID();
}

bool FPlayerSelection::HasValidPlayerAndSlot() const {
	/** Returns true if the selection has a valid player and slot. */
	if (!PlayerUniqueID.IsValid() || Slot >= UMTeamStatics::GetPlayerCountPerTeam()*2) return false;
	return true;
}
