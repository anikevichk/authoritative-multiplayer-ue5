#include "Player/MPlayerController.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Widgets/DeathScreenWidget.h"
#include "MLogCategories.h"

void AMPlayerController::BeginPlay(){
	/** Switches the local player to game-only input and removes the lobby widgets. */
	Super::BeginPlay();

	if (IsLocalController()){
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

	/** Hides the death screen once the client takes control of the respawned character. */
	Super::AcknowledgePossession(NewPawn);

	HideDeathScreen();
	UE_LOG(LogMGame, Verbose, TEXT("[Client] %s acknowledged %s"), *GetName(), *GetNameSafe(NewPawn));
}

void AMPlayerController::Client_ShowDeathScreen_Implementation(const FString& KillerName, float RespawnServerTime){
	/** Runs on the owning client when the server reports its death: shows the death screen with the killer and the respawn countdown. */
	UE_LOG(LogMGame, Log, TEXT("[Client] Died, killed by %s, respawn at server time %.1f"), KillerName.IsEmpty() ? TEXT("someone") : *KillerName, RespawnServerTime);
	if (!DeathScreenWidgetClass) return;

	if (!DeathScreenWidget) DeathScreenWidget = CreateWidget<UDeathScreenWidget>(this, DeathScreenWidgetClass);
	if (!DeathScreenWidget->IsInViewport()) DeathScreenWidget->AddToViewport();
	DeathScreenWidget->Show(KillerName, RespawnServerTime);
}

void AMPlayerController::HideDeathScreen(){
	/** Removes the death screen from the viewport if it is shown. */
	if (DeathScreenWidget) DeathScreenWidget->RemoveFromParent();
}