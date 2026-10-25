// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/MenuPlayerController.h"
#include "Blueprint/UserWidget.h"

void AMenuPlayerController::BeginPlay() {
	/** Switches to UI input; the menu widget is shown in OnRep_PlayerState. */
	Super::BeginPlay();
	SetInputMode(FInputModeUIOnly());
	SetShowMouseCursor(true);
}

void AMenuPlayerController::OnRep_PlayerState() {
	/** Shows the menu widget on a client once its PlayerState has arrived. */
	Super::OnRep_PlayerState();
	if (IsLocalPlayerController()) SpawnWidget();
}

void AMenuPlayerController::SpawnWidget() {
	/** Creates the menu widget and adds it to the screen. */
	if (MenuWidgetClass) {
		MenuWidget = CreateWidget<UUserWidget>(this, MenuWidgetClass);
		if (MenuWidget) MenuWidget->AddToViewport();
	}
}
