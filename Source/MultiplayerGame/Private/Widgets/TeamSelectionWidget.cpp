// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/TeamSelectionWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void UTeamSelectionWidget::SetSlotID(uint8 NewSlotID) {
	SlotID = NewSlotID;
}
void UTeamSelectionWidget::UpdateSlotInfo(const FString& PlayerNickname) {
	/** Shows the name of the player in this slot. */
	if (InfoText) InfoText->SetText(FText::FromString(PlayerNickname));
}

void UTeamSelectionWidget::NativeConstruct() {
	/** Binds the click of the slot button. */
	Super::NativeConstruct();
	if (SelectButton)
		SelectButton->OnClicked.AddDynamic(this, &UTeamSelectionWidget::SelectButtonClicked);
}

void UTeamSelectionWidget::SelectButtonClicked() {
	/** Notifies the lobby widget that this slot was clicked. */
	OnSlotClicked.Broadcast(SlotID);
}
