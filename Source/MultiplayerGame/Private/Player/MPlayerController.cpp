#include "Player/MPlayerController.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Framework/MGameState.h"
#include "Widgets/DeathScreenWidget.h"
#include "Widgets/MatchHUDWidget.h"
#include "MLogCategories.h"

void AMPlayerController::BeginPlay(){
	/** Switches the local player to game-only input, removes the lobby widgets, shows the match HUD and starts listening for the end of the match. */
	Super::BeginPlay();

	if (IsLocalController()){
		UWidgetLayoutLibrary::RemoveAllWidgets(this);

		if (MatchHUDWidgetClass){
			MatchHUDWidget = CreateWidget<UMatchHUDWidget>(this, MatchHUDWidgetClass);
			MatchHUDWidget->AddToViewport();
		}
		BindMatchState();

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

void AMPlayerController::BindMatchState(){
	/** Subscribes to the end of the match in the game state and handles a match that has already ended; on a client the game state always exists here, because its arrival is what starts BeginPlay. */
	AMGameState* MGameState = GetWorld()->GetGameState<AMGameState>();
	if (!MGameState) return;

	MGameState->OnMatchOver.AddUObject(this, &AMPlayerController::HandleMatchOver);
	if (MGameState->IsMatchOver()) HandleMatchOver(MGameState->GetWinningTeam());
}

void AMPlayerController::HandleMatchOver(int32 WinningTeam){
	/** Runs on the client when the game state announces the end of the match: turns off the player input, hides the death screen and shows the result. */
	UE_LOG(LogMGame, Log, TEXT("[Client] Match over, winning team %d"), WinningTeam);

	if (APawn* ControlledPawn = GetPawn()) ControlledPawn->DisableInput(this);
	HideDeathScreen();
	if (MatchHUDWidget) MatchHUDWidget->ShowResult(WinningTeam);
}

void AMPlayerController::HideDeathScreen(){
	/** Removes the death screen from the viewport if it is shown. */
	if (DeathScreenWidget) DeathScreenWidget->RemoveFromParent();
}