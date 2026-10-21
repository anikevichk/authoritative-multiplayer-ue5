// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/PlayerInfoTypes.h"
#include "GameFramework/PlayerState.h"
#include "Network/MNetStatics.h"

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

bool FPlayerSelection::IsForPlayer(const APlayerState* PlayerState) const {
	/** Returns true if the selection belongs to the player: by name in the editor, by unique ID in builds. */
	if (!PlayerState) return false;

	#if WITH_EDITOR
		return PlayerState->GetPlayerName() == GetPlayerNickname();
	#else
		return PlayerState->GetUniqueId() == GetPlayerUniqueID();
	#endif
}

bool FPlayerSelection::IsValid() const {
	/** Returns true if the selection has a valid player and slot; always true in the editor. */
	#if WITH_EDITOR
		return true;
	#else
		if (!PlayerUniqueID.IsValid() || Slot >= UMNetStatics::GetPlayerCountPerTeam()*2)
			return false;
		return true;
	#endif
}
