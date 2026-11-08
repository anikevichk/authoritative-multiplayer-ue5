// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/MGameMode.h"
#include "MLogCategories.h"
#include "Character/MCharacter.h"
#include "EngineUtils.h"
#include "Framework/MGameState.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Player/MPlayerController.h"
#include "Player/MPlayerState.h"
#include "TimerManager.h"

void AMGameMode::BeginPlay() {
	/** Starts the match time limit on the game map; the lobby turns the match rules off. */
	Super::BeginPlay();
	if (!bMatchRulesEnabled) return;

	if (AMGameState* MGameState = GetMGameState()) MGameState->StartMatch(MatchDuration);
	GetWorldTimerManager().SetTimer(MatchTimer, this, &AMGameMode::OnMatchTimeUp, MatchDuration, false);
	UE_LOG(LogMGame, Log, TEXT("[Server] Match started: %d kills to win, time limit %.0fs"), ScoreToWin, MatchDuration);
}

FString AMGameMode::InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal) {
	/** Marks the player as a bot when the client connected with the ?Bot URL option; the flag is replicated with the player state. */
	if (NewPlayerController && NewPlayerController->PlayerState) {
		NewPlayerController->PlayerState->SetIsABot(UGameplayStatics::HasOption(Options, TEXT("Bot")));
	}
	return Super::InitNewPlayer(NewPlayerController, UniqueId, Options, Portal);
}

void AMGameMode::PostLogin(APlayerController* NewPlayer) {
	/** Names every new player Player1, Player2, ... and every bot Bot1, Bot2, ... in the order they joined; the name is chosen by the server, not by the client. */
	Super::PostLogin(NewPlayer);

	const bool bIsBot = NewPlayer->PlayerState && NewPlayer->PlayerState->IsABot();
	const FString PlayerName = bIsBot ? FString::Printf(TEXT("Bot%d"), ++JoinedBotCount) : FString::Printf(TEXT("Player%d"), ++JoinedPlayerCount);
	ChangeName(NewPlayer, PlayerName, false);
	UE_LOG(LogMGame, Log, TEXT("[Server] %s joined"), *PlayerName);
}

AActor* AMGameMode::ChoosePlayerStart_Implementation(AController* Player) {
	/** Picks a player start of the player's team (tagged Team0 or Team1), farthest from every living character so that players do not spawn inside each other; starts without a team tag are used only when the team has none. */
	const AMPlayerState* MPlayerState = Player ? Player->GetPlayerState<AMPlayerState>() : nullptr;
	const FName TeamTag = MPlayerState && MPlayerState->GetTeamID() != FGenericTeamId::NoTeam ? FName(FString::Printf(TEXT("Team%d"), MPlayerState->GetTeamID().GetId())) : NAME_None;

	TArray<APlayerStart*> Candidates;
	for (APlayerStart* PlayerStart : TActorRange<APlayerStart>(GetWorld())) {
		if (PlayerStart->PlayerStartTag == TeamTag) Candidates.Add(PlayerStart);
	}
	if (Candidates.IsEmpty()) {
		for (APlayerStart* PlayerStart : TActorRange<APlayerStart>(GetWorld())) Candidates.Add(PlayerStart);
	}

	AActor* BestStart = nullptr;
	float BestDistanceSquared = -1.f;

	for (APlayerStart* PlayerStart : Candidates) {
		float NearestDistanceSquared = TNumericLimits<float>::Max();
		for (const AMCharacter* Character : TActorRange<AMCharacter>(GetWorld())) {
			if (Character->IsDead()) continue;
			NearestDistanceSquared = FMath::Min(NearestDistanceSquared, FVector::DistSquared(Character->GetActorLocation(), PlayerStart->GetActorLocation()));
		}
		if (NearestDistanceSquared > BestDistanceSquared) {
			BestDistanceSquared = NearestDistanceSquared;
			BestStart = PlayerStart;
		}
	}
	return BestStart ? BestStart : Super::ChoosePlayerStart_Implementation(Player);
}

bool AMGameMode::ShouldSpawnAtStartSpot(AController* Player) {
	/** Always chooses a new player start, because by default the engine reuses the start of the first spawn on every respawn. */
	return false;
}

void AMGameMode::CharacterKilled(AMCharacter* Victim, AController* Killer) {
	/** Gives a point to the team of the killer, ends the match when the team reaches the winning score and otherwise schedules the respawn of a killed player; all of it is decided only by the server. */
	AMGameState* MGameState = GetMGameState();
	if (!Victim || !MGameState || MGameState->IsMatchOver()) return;

	const FGenericTeamId KillerTeam = FGenericTeamId::GetTeamIdentifier(Killer ? Killer->GetPawn() : nullptr);
	if (KillerTeam != FGenericTeamId::NoTeam) {
		const int32 TeamScore = MGameState->AddTeamScore(KillerTeam.GetId());
		UE_LOG(LogMGame, Log, TEXT("[Server] Team %d scored, score %d:%d"), KillerTeam.GetId(), MGameState->GetTeamScore(0), MGameState->GetTeamScore(1));

		if (TeamScore >= ScoreToWin) {
			EndMatch(KillerTeam.GetId());
			return;
		}
	}

	AMPlayerController* VictimController = Victim->GetController<AMPlayerController>();
	if (!VictimController) return;

	const FString KillerName = Killer && Killer->PlayerState ? Killer->PlayerState->GetPlayerName() : FString();
	const float RespawnServerTime = MGameState->GetServerWorldTimeSeconds() + RespawnDelay;
	VictimController->Client_ShowDeathScreen(KillerName, RespawnServerTime);

	FTimerHandle RespawnTimer;
	GetWorldTimerManager().SetTimer(RespawnTimer, FTimerDelegate::CreateUObject(this, &AMGameMode::RespawnPlayer, TWeakObjectPtr<AController>(VictimController)), RespawnDelay, false);
	UE_LOG(LogMGame, Log, TEXT("[Server] %s will respawn in %.0fs"), *VictimController->PlayerState->GetPlayerName(), RespawnDelay);
}

void AMGameMode::RespawnPlayer(TWeakObjectPtr<AController> Controller) {
	/** Removes the dead body and spawns a new character at a player start; skipped if the player has left or the match has ended in the meantime. */
	if (!Controller.IsValid()) return;
	if (const AMGameState* MGameState = GetMGameState(); MGameState && MGameState->IsMatchOver()) return;

	if (APawn* DeadPawn = Controller->GetPawn()) {
		Controller->UnPossess();
		DeadPawn->Destroy();
	}
	RestartPlayer(Controller.Get());
	UE_LOG(LogMGame, Log, TEXT("[Server] %s respawned"), *Controller->PlayerState->GetPlayerName());
}

void AMGameMode::OnMatchTimeUp() {
	/** Ends the match when the time limit runs out: the team with more points wins, equal points give a draw. */
	const AMGameState* MGameState = GetMGameState();
	if (!MGameState || MGameState->IsMatchOver()) return;

	const int32 ScoreTeam0 = MGameState->GetTeamScore(0);
	const int32 ScoreTeam1 = MGameState->GetTeamScore(1);
	UE_LOG(LogMGame, Log, TEXT("[Server] Time is up, score %d:%d"), ScoreTeam0, ScoreTeam1);
	EndMatch(ScoreTeam0 == ScoreTeam1 ? INDEX_NONE : (ScoreTeam0 > ScoreTeam1 ? 0 : 1));
}

void AMGameMode::EndMatch(int32 WinningTeam) {
	/** Ends the match: saves the result in the game state, which replicates it to every client, and stops every character on the server. */
	AMGameState* MGameState = GetMGameState();
	if (!MGameState || MGameState->IsMatchOver()) return;

	MGameState->EndMatch(WinningTeam);
	GetWorldTimerManager().ClearTimer(MatchTimer);

	for (TActorIterator<AMCharacter> It(GetWorld()); It; ++It) {
		It->GetCharacterMovement()->DisableMovement();
	}

	const FString Result = WinningTeam == INDEX_NONE ? TEXT("draw") : FString::Printf(TEXT("team %d wins"), WinningTeam);
	UE_LOG(LogMGame, Log, TEXT("[Server] Match over: %s %d:%d"), *Result, MGameState->GetTeamScore(0), MGameState->GetTeamScore(1));
}
