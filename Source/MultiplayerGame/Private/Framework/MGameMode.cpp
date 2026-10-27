// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/MGameMode.h"
#include "MLogCategories.h"
#include "Character/MCharacter.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Player/MPlayerController.h"
#include "TimerManager.h"

void AMGameMode::PostLogin(APlayerController* NewPlayer) {
	/** Names every new player Player1, Player2, ... in the order they joined; the name is chosen by the server, not by the client. */
	Super::PostLogin(NewPlayer);

	const FString PlayerName = FString::Printf(TEXT("Player%d"), ++JoinedPlayerCount);
	ChangeName(NewPlayer, PlayerName, false);
	UE_LOG(LogMGame, Log, TEXT("[Server] %s joined"), *PlayerName);
}

void AMGameMode::CharacterKilled(AMCharacter* Victim, AController* Killer) {
	/** Schedules the respawn of a killed player and tells their client when it happens; the respawn time is decided only by the server. */
	AMPlayerController* VictimController = Victim ? Victim->GetController<AMPlayerController>() : nullptr;
	if (!VictimController) return;

	const FString KillerName = Killer && Killer->PlayerState ? Killer->PlayerState->GetPlayerName() : FString();
	const float RespawnServerTime = GameState->GetServerWorldTimeSeconds() + RespawnDelay;
	VictimController->Client_ShowDeathScreen(KillerName, RespawnServerTime);

	FTimerHandle RespawnTimer;
	GetWorldTimerManager().SetTimer(RespawnTimer, FTimerDelegate::CreateUObject(this, &AMGameMode::RespawnPlayer, TWeakObjectPtr<AController>(VictimController)), RespawnDelay, false);
	UE_LOG(LogMGame, Log, TEXT("[Server] %s will respawn in %.0fs"), *VictimController->PlayerState->GetPlayerName(), RespawnDelay);
}

void AMGameMode::RespawnPlayer(TWeakObjectPtr<AController> Controller) {
	/** Removes the dead body and spawns a new character at a player start; skipped if the player has left in the meantime. */
	if (!Controller.IsValid()) return;

	if (APawn* DeadPawn = Controller->GetPawn()) {
		Controller->UnPossess();
		DeadPawn->Destroy();
	}
	RestartPlayer(Controller.Get());
	UE_LOG(LogMGame, Log, TEXT("[Server] %s respawned"), *Controller->PlayerState->GetPlayerName());
}
