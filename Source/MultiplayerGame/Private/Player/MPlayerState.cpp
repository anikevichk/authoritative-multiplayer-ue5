// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/MPlayerState.h"
#include "Net/UnrealNetwork.h"

AMPlayerState::AMPlayerState() {
	/** A new player has no team until they pick a slot in the lobby. */
	TeamID = FGenericTeamId::NoTeam;
}

void AMPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	/** Replicates the team to all clients. */
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMPlayerState, TeamID);
}

void AMPlayerState::CopyProperties(APlayerState* PlayerState) {
	/** Keeps the team when the PlayerState is recreated during seamless travel. */
	Super::CopyProperties(PlayerState);

	if (AMPlayerState* NewPlayerState = Cast<AMPlayerState>(PlayerState)) NewPlayerState->TeamID = TeamID;
	
}

void AMPlayerState::SetTeamID(const FGenericTeamId& NewTeamID) {
	/** Sets the team of the player; has an effect only on the server. */
	if (!HasAuthority()) return;

	TeamID = NewTeamID;
}
