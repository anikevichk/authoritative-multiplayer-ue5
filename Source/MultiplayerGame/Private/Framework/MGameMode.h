// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MGameMode.generated.h"

/**
 *
 */
UCLASS()
class AMGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	virtual void PostLogin(APlayerController* NewPlayer) override;

	void CharacterKilled(class AMCharacter* Victim, AController* Killer);

private:
	int32 JoinedPlayerCount = 0;

	UPROPERTY(EditDefaultsOnly, Category = "Respawn")
	float RespawnDelay = 10.f;

	void RespawnPlayer(TWeakObjectPtr<AController> Controller);
};
