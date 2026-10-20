// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "MAIController.generated.h"

class UAIPerceptionComponent;
class UAISenseConfig_Sight;

/**
 *
 */
UCLASS()
class AMAIController : public AAIController
{

	GENERATED_BODY()

public:
	AMAIController();
	virtual void OnPossess(APawn* NewPawn) override;

private:
	UPROPERTY(VisibleDefaultsOnly, Category = "Perception")
	TObjectPtr<UAIPerceptionComponent> BotPerceptionComponent;

	UPROPERTY(VisibleDefaultsOnly, Category = "Perception")
	TObjectPtr<UAISenseConfig_Sight> BotSightConfig;
};
