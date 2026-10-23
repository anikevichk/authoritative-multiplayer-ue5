// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/LobbyGameMode.h"
#include "Framework/MGameState.h"
#include "MLogCategories.h"
#include "GameFramework/PlayerState.h"
#include "TimerManager.h"

ALobbyGameMode::ALobbyGameMode() {
	/** Keeps clients connected and keeps their PlayerStates when the server moves to the game map. */
	bUseSeamlessTravel = true;
}

void ALobbyGameMode::BeginPlay() {
	/** Starts a timer that checks the lobby state once per second. */
	Super::BeginPlay();
	GetWorldTimerManager().SetTimer(StartCountdownTimer, this, &ALobbyGameMode::UpdateStartCountdown, 1.f, true);
}

void ALobbyGameMode::Logout(AController* Exiting) {
	/** Frees the slot of a player who disconnected. */
	const APlayerState* LeavingPlayer = Exiting ? Exiting->PlayerState : nullptr;
	UE_LOG(LogMLobby, Log, TEXT("[Server] %s left the lobby"), LeavingPlayer ? *LeavingPlayer->GetPlayerName() : TEXT("unknown player"));

	if (AMGameState* MGameState = GetGameState<AMGameState>()) {
		MGameState->RemovePlayerSelection(LeavingPlayer);
	}
	Super::Logout(Exiting);
}

void ALobbyGameMode::UpdateStartCountdown() {
	/**
	 * Runs once per second on the server: starts, advances or resets the countdown
	 * and moves all players to the game map when it ends.
	 */
	AMGameState* MGameState = GetGameState<AMGameState>();
	if (!MGameState) return;

	if (!MGameState->CanStart()) {
		if (MGameState->GetStartCountdown() >= 0) UE_LOG(LogMLobby, Log, TEXT("[Server] Countdown cancelled, start conditions are no longer met"));
		MGameState->SetStartCountdown(-1);
		return;
	}

	const int32 Countdown = MGameState->GetStartCountdown();
	if (Countdown < 0) {
		UE_LOG(LogMLobby, Log, TEXT("[Server] All players are ready, match starts in %d s"), StartCountdownDuration);
		MGameState->SetStartCountdown(StartCountdownDuration);
		return;
	}
	if (Countdown > 1) {
		MGameState->SetStartCountdown(Countdown - 1);
		return;
	}

	MGameState->SetStartCountdown(0);
	GetWorldTimerManager().ClearTimer(StartCountdownTimer);
	UE_LOG(LogMLobby, Log, TEXT("[Server] Travelling to %s"), *GameMapPath);
	GetWorld()->ServerTravel(GameMapPath);
}
