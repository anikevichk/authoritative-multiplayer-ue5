// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Framework/MGameState.h"
#include "MGameMode.generated.h"

/**
 *
 */
UCLASS()
class AMGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;

	void CharacterKilled(class AMCharacter* Victim, AController* Killer);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Match")
	bool bMatchRulesEnabled = true;

private:
	int32 JoinedPlayerCount = 0;

	UPROPERTY(EditDefaultsOnly, Category = "Respawn")
	float RespawnDelay = 10.f;

	UPROPERTY(EditDefaultsOnly, Category = "Match")
	int32 ScoreToWin = 2;

	UPROPERTY(EditDefaultsOnly, Category = "Match")
	float MatchDuration = 60.f;

	FTimerHandle MatchTimer;

	void RespawnPlayer(TWeakObjectPtr<AController> Controller);
	void OnMatchTimeUp();
	void EndMatch(int32 WinningTeam);
	FORCEINLINE AMGameState* GetMGameState() const { return GetGameState<AMGameState>(); }
};
