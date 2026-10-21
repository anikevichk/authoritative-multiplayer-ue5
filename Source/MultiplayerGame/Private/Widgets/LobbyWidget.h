// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Player/PlayerInfoTypes.h"
#include "LobbyWidget.generated.h"


/**
 * 
 */
UCLASS()
class ULobbyWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
private:
	UPROPERTY(meta = (BindWidget))
	class UWidgetSwitcher* MainSwitcher;

	UPROPERTY(meta = (BindWidget))
	class UWidget* TeamSelectionRoot;

	UPROPERTY(meta = (BindWidget))
	class UWidget* GameRoot;

	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* CountdownText;

	UPROPERTY(meta = (BindWidget))
	class UUniformGridPanel* TeamSelectionPanel;

	UPROPERTY(EditDefaultsOnly, Category="TeamSelection")
	TSubclassOf<class UTeamSelectionWidget> TeamSelectionWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category="TeamSelection")
	TArray<class UTeamSelectionWidget*> TeamSelectionSlots;

	void ClearAndLoadTeamSelectionSlots();
	void SlotSelected(uint8 NewSlotID);
	
	UPROPERTY()
	class ALobbyPlayerController* LobbyPlayerController;
	
	void ConfigGameState();
	FTimerHandle GonfigTimer;
	
	UPROPERTY()
	class AMGameState* GameState;

	void UpdatePlayerSelectionDisplay(const TArray<FPlayerSelection>& PlayerSelections);
	void UpdateStartCountdownDisplay(int32 SecondsLeft);

	void SwitchToGame();
};

