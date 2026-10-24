// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "HealthBarWidget.generated.h"

UCLASS()
class UHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetHealthPercent(float Percent) {if (HealthBar) HealthBar->SetPercent(Percent);}

private:
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthBar;
};
