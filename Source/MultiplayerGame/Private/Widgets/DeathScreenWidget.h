#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DeathScreenWidget.generated.h"

UCLASS()
class UDeathScreenWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void Show(const FString& KillerName, float InRespawnServerTime);

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* RespawnText;

	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* KillerText;

	UPROPERTY(EditDefaultsOnly, Category = "Death Screen")
	float FadeInDuration = 1.5f;

	float RespawnServerTime = 0.f;
	float ShownTime = 0.f;
};
