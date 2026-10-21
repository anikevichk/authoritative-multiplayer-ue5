// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/MenuPlayerController.h"
#include "Blueprint/UserWidget.h"

void AMenuPlayerController::BeginPlay() {
	/** Switches to UI input; on a listen server host the menu widget is shown right away. */
	Super::BeginPlay();
	SetInputMode(FInputModeUIOnly());
	SetShowMouseCursor(true);

	if (HasAuthority() && IsLocalPlayerController()) {
		SpawnWidget();
	}
	
}

void AMenuPlayerController::OnRep_PlayerState() {
	/** Shows the menu widget on a client once its PlayerState has arrived. */
	Super::OnRep_PlayerState();
	if (IsLocalPlayerController()) {
		SpawnWidget();
	}
}

void AMenuPlayerController::SpawnWidget() {
	/** Creates the menu widget and adds it to the screen. */
	if (MenuWidgetClass) {
		MenuWidget = CreateWidget<UUserWidget>(this, MenuWidgetClass);
		if (MenuWidget) {
			MenuWidget->AddToViewport();
		}
	}
}
