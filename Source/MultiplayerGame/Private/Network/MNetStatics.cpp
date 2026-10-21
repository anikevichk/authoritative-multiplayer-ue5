// Fill out your copyright notice in the Description page of Project Settings.


#include "Network/MNetStatics.h"

uint8 UMNetStatics::PlayerCountPerTeam = 5;

uint8 UMNetStatics::GetPlayerCountPerTeam() {
	return PlayerCountPerTeam;
}

void UMNetStatics::SetPlayerCountPerTeam(uint8 NewValue)
{
	PlayerCountPerTeam = NewValue;
}

uint8 UMNetStatics::GetTeamForSlot(uint8 SlotID) {
	return SlotID / PlayerCountPerTeam;
}