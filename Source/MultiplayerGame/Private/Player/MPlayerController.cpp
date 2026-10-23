#include "Player/MPlayerController.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "MLogCategories.h"

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

		UE_LOG(LogMGame, Verbose, TEXT("[Client] Game input mode set"));
	}
}

void AMPlayerController::OnPossess(APawn* NewPawn){

	Super::OnPossess(NewPawn);

	UE_LOG(LogMGame, Verbose, TEXT("[Server] %s possessed %s"), *GetName(), *GetNameSafe(NewPawn));
}

void AMPlayerController::AcknowledgePossession(APawn* NewPawn){

	Super::AcknowledgePossession(NewPawn);

	UE_LOG(LogMGame, Verbose, TEXT("[Client] %s acknowledged %s"), *GetName(), *GetNameSafe(NewPawn));
}