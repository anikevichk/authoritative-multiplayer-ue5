// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/LobbyPlayerController.h"
#include "Cheats/MCheatManager.h"
#include "Framework/MGameState.h"
#include "GameFramework/PlayerState.h"
#include "MLogCategories.h"

ALobbyPlayerController::ALobbyPlayerController() {
	/** Uses the project cheat manager that holds the test commands sending invalid requests. */
	CheatClass = UMCheatManager::StaticClass();
}

void ALobbyPlayerController::BeginPlay() {
	/** Creates the cheat manager on the local client so the lobby test commands can be used. */
	Super::BeginPlay();
	if (IsLocalController()) AddCheats(true);
}

void ALobbyPlayerController::Server_RequestSlotSelectionChange_Implementation(uint8 SlotID) {
	/** Runs on the server when a client asks to take a lobby slot; requests sent faster than the minimum interval are rejected. */
	if (!GetWorld()) return;

	const float Now = GetWorld()->GetTimeSeconds();
	if (Now - LastSlotRequestTime < SlotRequestMinInterval) {
		UE_LOG(LogMLobby, Warning, TEXT("[Server] Rejected slot request from %s: too many requests (%.2fs since the last one, minimum %.2fs)"),
			PlayerState ? *PlayerState->GetPlayerName() : TEXT("unknown player"), Now - LastSlotRequestTime, SlotRequestMinInterval);
		return;
	}
	LastSlotRequestTime = Now;

	AMGameState* GameState = GetWorld()->GetGameState<AMGameState>();
	if (!GameState) return;

	GameState->RequestPlayerSelectionChange(GetPlayerState<APlayerState>(), SlotID);
}
