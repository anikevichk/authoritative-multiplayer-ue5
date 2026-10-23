// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/MGameMode.h"
#include "MLogCategories.h"
#include "GameFramework/PlayerState.h"

void AMGameMode::PostLogin(APlayerController* NewPlayer) {
	/** Names every new player Player1, Player2, ... in the order they joined; the name is chosen by the server, not by the client. */
	Super::PostLogin(NewPlayer);

	const FString PlayerName = FString::Printf(TEXT("Player%d"), ++JoinedPlayerCount);
	ChangeName(NewPlayer, PlayerName, false);
	UE_LOG(LogMGame, Log, TEXT("[Server] %s joined"), *PlayerName);
}