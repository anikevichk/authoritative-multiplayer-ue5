#include "Framework/MGameState.h"
#include "Net/UnrealNetwork.h"
#include "MLogCategories.h"
#include "Network/MNetStatics.h"
#include "Player/MPlayerState.h"

void AMGameState::RequestPlayerSelectionChange(APlayerState* RequestingPlayer, uint8 DesiredSlot) {
	/** Moves a player to a free slot and sets their team. Server only. */
	if (!HasAuthority()) return;

	const FString PlayerName = RequestingPlayer ? RequestingPlayer->GetPlayerName() : TEXT("unknown player");
	if (IsSlotTaken(DesiredSlot)) {
		UE_LOG(LogMLobby, Log, TEXT("[Server] %s asked for slot %d, rejected: the slot is taken"), *PlayerName, DesiredSlot);
		return;
	}
	
	FPlayerSelection* PlayerSelectionPtr = PlayerSelectionArray.FindByPredicate([&](const FPlayerSelection& PlayerSelection) {
		return PlayerSelection.IsForPlayer(RequestingPlayer);
	});

	if (PlayerSelectionPtr) PlayerSelectionPtr->SetSlot(DesiredSlot);
	else PlayerSelectionArray.Add(FPlayerSelection(DesiredSlot, RequestingPlayer));

	if (AMPlayerState* MPlayerState = Cast<AMPlayerState>(RequestingPlayer)) {
		MPlayerState->SetTeamID(FGenericTeamId(UMNetStatics::GetTeamForSlot(DesiredSlot)));
	}
	UE_LOG(LogMLobby, Log, TEXT("[Server] %s took slot %d, team %d"), *PlayerName, DesiredSlot, UMNetStatics::GetTeamForSlot(DesiredSlot));	

	OnPlayerSelectionUpdated.Broadcast(PlayerSelectionArray);
}

void AMGameState::RemovePlayerSelection(const APlayerState* LeavingPlayer) {
	/** Frees the slot of a player who left the lobby. Server only. */
	if (!HasAuthority()) return;

	const int32 RemovedCount = PlayerSelectionArray.RemoveAll([&](const FPlayerSelection& PlayerSelection) {
		return PlayerSelection.IsForPlayer(LeavingPlayer);
	});

	if (RemovedCount > 0) OnPlayerSelectionUpdated.Broadcast(PlayerSelectionArray);
}

bool AMGameState::IsSlotTaken(uint8 SlotID) const {
	/** Returns true if a player already holds the slot. */
	for (const FPlayerSelection& PlayerSelection: PlayerSelectionArray) {
		if (PlayerSelection.GetSlot() == SlotID) return true;
		
	}
	return false;
}

void AMGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const{
	/** Replicates the slot selections and the start countdown to all clients. */
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMGameState, PlayerSelectionArray);
	DOREPLIFETIME(AMGameState, StartCountdown);
	DOREPLIFETIME(AMGameState, TeamScores);
	DOREPLIFETIME(AMGameState, MatchEndServerTime);
	DOREPLIFETIME(AMGameState, bMatchOver);
	DOREPLIFETIME(AMGameState, WinningTeam);
}

AMGameState::AMGameState() {
	/** Starts both teams with zero points. */
	TeamScores.Init(0, 2);
}

void AMGameState::StartMatch(float Duration) {
	/** Sets the server time when the match ends because of the time limit. Server only. */
	if (!HasAuthority()) return;

	MatchEndServerTime = GetServerWorldTimeSeconds() + Duration;
}

int32 AMGameState::AddTeamScore(uint8 Team) {
	/** Gives the team one point and returns its new score. Server only. */
	if (!HasAuthority() || !TeamScores.IsValidIndex(Team)) return 0;

	return ++TeamScores[Team];
}

void AMGameState::EndMatch(int32 InWinningTeam) {
	/** Marks the match as over with the given winner, or INDEX_NONE for a draw. Server only. */
	if (!HasAuthority()) return;

	bMatchOver = true;
	WinningTeam = InWinningTeam;
}

void AMGameState::OnRep_MatchOver() {
	/** Announces the end of the match with the winning team on clients. */
	if (bMatchOver) OnMatchOver.Broadcast(WinningTeam);
}

int32 AMGameState::GetTeamScore(uint8 Team) const {
	/** Returns the points of the team, or zero for an unknown team. */
	return TeamScores.IsValidIndex(Team) ? TeamScores[Team] : 0;
}

const TArray<FPlayerSelection>& AMGameState::GetPlayerSelectionChange() const {
	/** Returns the current slot selections of all players. */
	return PlayerSelectionArray;
}

bool AMGameState::CanStart() const {
	/** Returns true when every player has picked a slot and both teams have players. */
	if (PlayerArray.Num() == 0 || PlayerSelectionArray.Num() != PlayerArray.Num()) return false;

	bool bTeamHasPlayers[2] = {false, false};
	for (const FPlayerSelection& PlayerSelection : PlayerSelectionArray) {
		const uint8 Team = UMNetStatics::GetTeamForSlot(PlayerSelection.GetSlot());
		bTeamHasPlayers[Team] = true;
	}
	return bTeamHasPlayers[0] && bTeamHasPlayers[1];
}

void AMGameState::SetStartCountdown(int32 NewValue) {
	/** Sets the seconds left before the match starts, or -1 when there is no countdown. Server only. */
	if (!HasAuthority() || StartCountdown == NewValue) return;

	StartCountdown = NewValue;
	OnStartCountdownUpdated.Broadcast(NewValue);
}

void AMGameState::OnRep_PlayerSelectionArray(){
	/** Passes the new slot selections to the lobby widget on clients. */
	OnPlayerSelectionUpdated.Broadcast(PlayerSelectionArray);
}

void AMGameState::OnRep_StartCountdown() {
	/** Passes the new countdown value to the lobby widget on clients. */
	OnStartCountdownUpdated.Broadcast(StartCountdown);
}
