#include "Framework/MGameState.h"
#include "Net/UnrealNetwork.h"
#include "Network/MNetStatics.h"
#include "Player/MPlayerState.h"

void AMGameState::RequestPlayerSelectionChange(APlayerState* RequestingPlayer, uint8 DesiredSlot) {
	if (!HasAuthority() || IsSlotTaken(DesiredSlot)) return;
	
	FPlayerSelection* PlayerSelectionPtr = PlayerSelectionArray.FindByPredicate([&](const FPlayerSelection& PlayerSelection) {
		return PlayerSelection.IsForPlayer(RequestingPlayer);
	});

	if (PlayerSelectionPtr) PlayerSelectionPtr->SetSlot(DesiredSlot);
	else PlayerSelectionArray.Add(FPlayerSelection(DesiredSlot, RequestingPlayer));

	if (AMPlayerState* MPlayerState = Cast<AMPlayerState>(RequestingPlayer)) {
		MPlayerState->SetTeamID(FGenericTeamId(UMNetStatics::GetTeamForSlot(DesiredSlot)));
	}	

	OnPlayerSelectionUpdated.Broadcast(PlayerSelectionArray);
}

void AMGameState::RemovePlayerSelection(const APlayerState* LeavingPlayer) {
	if (!HasAuthority()) return;

	const int32 RemovedCount = PlayerSelectionArray.RemoveAll([&](const FPlayerSelection& PlayerSelection) {
		return PlayerSelection.IsForPlayer(LeavingPlayer);
	});

	if (RemovedCount > 0) OnPlayerSelectionUpdated.Broadcast(PlayerSelectionArray);
}

bool AMGameState::IsSlotTaken(uint8 SlotID) const {
	for (const FPlayerSelection& PlayerSelection: PlayerSelectionArray) {
		if (PlayerSelection.GetSlot() == SlotID) return true;
		
	}
	return false;
}

void AMGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(
		AMGameState,
		PlayerSelectionArray,
		COND_None,
		REPNOTIFY_Always
	);
	DOREPLIFETIME(AMGameState, StartCountdown);
}

const TArray<FPlayerSelection>& AMGameState::GetPlayerSelectionChange() const {
	return PlayerSelectionArray;
}

// Every connected player has picked a slot and both teams have at least one player
bool AMGameState::CanStart() const {
	if (PlayerArray.Num() == 0 || PlayerSelectionArray.Num() != PlayerArray.Num()) return false;

	bool bTeamHasPlayers[2] = {false, false};
	for (const FPlayerSelection& PlayerSelection : PlayerSelectionArray) {
		const uint8 Team = UMNetStatics::GetTeamForSlot(PlayerSelection.GetSlot());
		bTeamHasPlayers[Team] = true;
	}
	return bTeamHasPlayers[0] && bTeamHasPlayers[1];
}

void AMGameState::SetStartCountdown(int32 NewValue) {
	if (!HasAuthority() || StartCountdown == NewValue) return;

	StartCountdown = NewValue;
	OnStartCountdownUpdated.Broadcast(NewValue);
}

void AMGameState::OnRep_PlayerSelectionArray(){
	OnPlayerSelectionUpdated.Broadcast(PlayerSelectionArray);
}

void AMGameState::OnRep_StartCountdown() {
	OnStartCountdownUpdated.Broadcast(StartCountdown);
}
