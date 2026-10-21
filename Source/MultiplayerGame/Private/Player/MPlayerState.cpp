// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/MPlayerState.h"
#include "Net/UnrealNetwork.h"

AMPlayerState::AMPlayerState() {
	TeamID = FGenericTeamId::NoTeam;
}

void AMPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMPlayerState, TeamID);
}

void AMPlayerState::CopyProperties(APlayerState* PlayerState) {
	Super::CopyProperties(PlayerState);

	if (AMPlayerState* NewPlayerState = Cast<AMPlayerState>(PlayerState)) {
		NewPlayerState->TeamID = TeamID;
	}
}

void AMPlayerState::SetTeamID(const FGenericTeamId& NewTeamID) {
	if (!HasAuthority()) return;

	TeamID = NewTeamID;
}
