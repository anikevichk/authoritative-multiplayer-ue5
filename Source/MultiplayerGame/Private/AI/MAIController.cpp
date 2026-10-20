// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/MAIController.h"

#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"

AMAIController::AMAIController()
{
	BotPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("BotPerceptionComponent"));
	BotSightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("BotSightConfig"));

	// Hardcoded bot team; players are team 0 (see AMCharacter)
	SetGenericTeamId(FGenericTeamId(1));

	BotSightConfig->DetectionByAffiliation.bDetectEnemies = true;
	BotSightConfig->DetectionByAffiliation.bDetectFriendlies = false;
	BotSightConfig->DetectionByAffiliation.bDetectNeutrals = false;

	BotPerceptionComponent->ConfigureSense(*BotSightConfig);
	BotPerceptionComponent->SetDominantSense(BotSightConfig->GetSenseImplementation());

    BotSightConfig->SightRadius = 1000.f;                 
    BotSightConfig->LoseSightRadius = 1500.f;             
    BotSightConfig->PeripheralVisionAngleDegrees = 180.f;  
    BotSightConfig->SetMaxAge(5.f);

}

void AMAIController::OnPossess(APawn* NewPawn)
{
	Super::OnPossess(NewPawn);

	if (IGenericTeamAgentInterface* PawnTeamInterface = Cast<IGenericTeamAgentInterface>(NewPawn)){
		PawnTeamInterface->SetGenericTeamId(GetGenericTeamId());
	}
}
