// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/LobbyPlayerController.h"
#include "Framework/MGameState.h"
#include "GameFramework/PlayerState.h"
#include "Framework/MGameState.h"

void ALobbyPlayerController::Server_RequestSlotSelectionChange_Implementation(uint8 SlotID) {
	/** Runs on the server when a client asks to take a lobby slot. */
	if (!GetWorld()) return;

	AMGameState* GameState = GetWorld()->GetGameState<AMGameState>();
	if (!GameState) return;

	GameState->RequestPlayerSelectionChange(GetPlayerState<APlayerState>(), SlotID);
}

bool ALobbyPlayerController::Server_RequestSlotSelectionChange_Validate(uint8 SlotID) {
	/** Validates the slot request before it runs on the server; no checks yet. */
	return true;
}

void ALobbyPlayerController::Client_Start_Implementation() {
	/** Asks the lobby widget to switch to the game screen; currently unused. */
	OnSwitchToGame.ExecuteIfBound();
}
