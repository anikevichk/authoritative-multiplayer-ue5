// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/LobbyGameMode.h"
#include "Framework/MGameState.h"
#include "GameFramework/PlayerState.h"
#include "TimerManager.h"

ALobbyGameMode::ALobbyGameMode() {
	bUseSeamlessTravel = true;
}

void ALobbyGameMode::BeginPlay() {
	Super::BeginPlay();
	GetWorldTimerManager().SetTimer(StartCountdownTimer, this, &ALobbyGameMode::UpdateStartCountdown, 1.f, true);
}

void ALobbyGameMode::Logout(AController* Exiting) {
	// A player who left must not keep their slot, otherwise the match could never start
	if (AMGameState* MGameState = GetGameState<AMGameState>()) {
		MGameState->RemovePlayerSelection(Exiting ? Exiting->PlayerState : nullptr);
	}
	Super::Logout(Exiting);
}

// Runs once per second on the server. The countdown starts when everyone is ready and is
// cancelled by anything that breaks it: a new player, a slot change or a disconnect.
void ALobbyGameMode::UpdateStartCountdown() {
	AMGameState* MGameState = GetGameState<AMGameState>();
	if (!MGameState) return;

	if (!MGameState->CanStart()) {
		MGameState->SetStartCountdown(-1);
		return;
	}

	const int32 Countdown = MGameState->GetStartCountdown();
	if (Countdown < 0) {
		MGameState->SetStartCountdown(StartCountdownDuration);
		return;
	}
	if (Countdown > 1) {
		MGameState->SetStartCountdown(Countdown - 1);
		return;
	}

	MGameState->SetStartCountdown(0);
	GetWorldTimerManager().ClearTimer(StartCountdownTimer);
	GetWorld()->ServerTravel(GameMapPath);
}
