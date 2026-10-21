// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Framework/MGameMode.h"
#include "LobbyGameMode.generated.h"

/**
 * 
 */
UCLASS()
class ALobbyGameMode : public AMGameMode
{
	GENERATED_BODY()
public:
	ALobbyGameMode();
	virtual void BeginPlay() override;
	virtual void Logout(AController* Exiting) override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Lobby")
	int32 StartCountdownDuration = 5;

	UPROPERTY(EditDefaultsOnly, Category = "Lobby")
	FString GameMapPath = TEXT("/Game/Maps/NewMap");

	FTimerHandle StartCountdownTimer;
	void UpdateStartCountdown();
};
