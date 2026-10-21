// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "GenericTeamAgentInterface.h"
#include "MPlayerState.generated.h"

/**
 *
 */
UCLASS()
class AMPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AMPlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Seamless travel: copies the team from the lobby PlayerState to the game map one
	virtual void CopyProperties(APlayerState* PlayerState) override;

	// Server only
	void SetTeamID(const FGenericTeamId& NewTeamID);
	FORCEINLINE FGenericTeamId GetTeamID() const { return TeamID; }

private:
	UPROPERTY(Replicated)
	FGenericTeamId TeamID;
};
