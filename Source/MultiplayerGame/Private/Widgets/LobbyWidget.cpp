// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/LobbyWidget.h"

#include "Components/UniformGridSlot.h"
#include "Components/UniformGridPanel.h"
#include "Widgets/TeamSelectionWidget.h"
#include "Framework/MTeamStatics.h"
#include "Player/LobbyPlayerController.h"
#include "Framework/MGameState.h"
#include "Components/TextBlock.h"

void ULobbyWidget::NativeConstruct() {
	/** Builds the slot grid and connects the widget to the game state and the player controller. */
	Super::NativeConstruct();
	
	ClearAndLoadTeamSelectionSlots();
	ConfigGameState();
	LobbyPlayerController = GetOwningPlayer<ALobbyPlayerController>();
}

void ULobbyWidget::ClearAndLoadTeamSelectionSlots() {
	/** Creates the slot buttons, one column per team. */
	TeamSelectionPanel->ClearChildren();
	const int32 PlayersPerTeam = UMTeamStatics::GetPlayerCountPerTeam();

	for (int32 i = 0; i < PlayersPerTeam * 2; i++){
		UTeamSelectionWidget* SelectionSlot = CreateWidget<UTeamSelectionWidget>(this, TeamSelectionWidgetClass);

		if (SelectionSlot) {
			SelectionSlot->SetSlotID(i);
			UUniformGridSlot* GridSlot = TeamSelectionPanel->AddChildToUniformGrid(SelectionSlot);
			if (GridSlot) {
				const int32 Row = i % PlayersPerTeam;
				const int32 Column = i / PlayersPerTeam;
				
				GridSlot->SetRow(Row);
				GridSlot->SetColumn(Column);
			}

			SelectionSlot->OnSlotClicked.AddUObject(this, &ULobbyWidget::SlotSelected);
			TeamSelectionSlots.Add(SelectionSlot);
		}
			
	}
}
void ULobbyWidget::SlotSelected(uint8 NewSlotID) {
	/** Asks the server to move the player to the clicked slot. */
	if (LobbyPlayerController) LobbyPlayerController->Server_RequestSlotSelectionChange(NewSlotID);
}

void ULobbyWidget::ConfigGameState() {
	/** Subscribes to game state updates, retrying every second until the game state has replicated. */
	UWorld* World = GetWorld();
	if (!World) return;

	GameState = World->GetGameState<AMGameState>();
	if (!GameState) World->GetTimerManager().SetTimer(GonfigTimer, this, &ULobbyWidget::ConfigGameState, 1.f);
	else {
		GameState->OnPlayerSelectionUpdated.AddUObject(this, &ULobbyWidget::UpdatePlayerSelectionDisplay);
		GameState->OnStartCountdownUpdated.AddUObject(this, &ULobbyWidget::UpdateStartCountdownDisplay);
		UpdatePlayerSelectionDisplay(GameState->GetPlayerSelectionChange());
		UpdateStartCountdownDisplay(GameState->GetStartCountdown());
	}
}

void ULobbyWidget::UpdatePlayerSelectionDisplay(const TArray<FPlayerSelection>& PlayerSelections) {
	/** Shows player names in the slots they hold; the other slots show Empty. */
	for (UTeamSelectionWidget* SelectionSlot:TeamSelectionSlots) {
		SelectionSlot->UpdateSlotInfo("Empty");
	}

	for ( const FPlayerSelection& PlayerSelection : PlayerSelections) {
		if (!PlayerSelection.HasValidPlayerAndSlot()) continue;
		TeamSelectionSlots[PlayerSelection.GetSlot()]->UpdateSlotInfo(PlayerSelection.GetPlayerNickname());
	}
}

void ULobbyWidget::UpdateStartCountdownDisplay(int32 SecondsLeft) {
	/** Shows the countdown from the server; a negative value means the lobby is waiting for players. */
	if (!CountdownText) return;

	if (SecondsLeft < 0) CountdownText->SetText(FText::FromString(TEXT("Waiting for players...")));
	else CountdownText->SetText(FText::FromString(FString::Printf(TEXT("Match starts in %d"), SecondsLeft)));
}

