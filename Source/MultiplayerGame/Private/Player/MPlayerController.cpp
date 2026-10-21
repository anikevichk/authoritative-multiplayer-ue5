#include "Player/MPlayerController.h"
#include "Blueprint/WidgetLayoutLibrary.h"

void AMPlayerController::BeginPlay(){
	/** Switches the local player to game-only input and removes the lobby widgets. */
	Super::BeginPlay();

	if (IsLocalController())
	{
		UWidgetLayoutLibrary::RemoveAllWidgets(this);

		FInputModeGameOnly InputMode;
		SetInputMode(InputMode);

		SetShowMouseCursor(false);
		bEnableClickEvents = false;
		bEnableMouseOverEvents = false;

		UE_LOG(LogTemp, Warning, TEXT("Game input mode set"));
	}
}

void AMPlayerController::OnPossess(APawn* NewPawn){

	Super::OnPossess(NewPawn);

	UE_LOG(LogTemp, Warning, TEXT("SERVER possessed pawn: %s"), *GetNameSafe(NewPawn));
}

void AMPlayerController::AcknowledgePossession(APawn* NewPawn){

	Super::AcknowledgePossession(NewPawn);

	UE_LOG(LogTemp, Warning, TEXT("CLIENT acknowledged pawn: %s"), *GetNameSafe(NewPawn));
}