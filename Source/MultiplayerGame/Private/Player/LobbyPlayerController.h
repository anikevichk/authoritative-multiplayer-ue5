// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Player/MenuPlayerController.h"
#include "LobbyPlayerController.generated.h"

/**
 *
 */
UCLASS()
class ALobbyPlayerController : public AMenuPlayerController
{
	GENERATED_BODY()

public:
	ALobbyPlayerController();

	virtual void BeginPlay() override;

	UFUNCTION(Server, Reliable)
	void Server_RequestSlotSelectionChange(uint8 SlotID);

private:
	UPROPERTY(EditDefaultsOnly, Category = "Lobby")
	float SlotRequestMinInterval = 0.25f;

	float LastSlotRequestTime = -1000.f;
};
