// Fill out your copyright notice in the Description page of Project Settings.


#include "Network/MNetStatics.h"

uint8 UMNetStatics::PlayerCountPerTeam = 5;

uint8 UMNetStatics::GetPlayerCountPerTeam() {
	return PlayerCountPerTeam;
}

void UMNetStatics::SetPlayerCountPerTeam(uint8 NewValue){
	PlayerCountPerTeam = NewValue;
}

uint8 UMNetStatics::GetTeamForSlot(uint8 SlotID) {
	/** Returns the team of a lobby slot: the left column is team 0, the right column is team 1. */
	return SlotID / PlayerCountPerTeam;
}