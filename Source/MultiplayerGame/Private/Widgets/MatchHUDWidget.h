#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MatchHUDWidget.generated.h"

UCLASS()
class UMatchHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void ShowResult(int32 WinningTeam);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ScoreText;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* TimerText;

	UPROPERTY(meta = (BindWidgetOptional))
	class UTextBlock* ResultText;

	UPROPERTY(meta = (BindWidgetOptional))
	class UWidget* ResultOverlay;

	UPROPERTY(EditDefaultsOnly, Category = "Result")
	FLinearColor VictoryColor = FLinearColor(0.02f, 0.35f, 0.08f);

	UPROPERTY(EditDefaultsOnly, Category = "Result")
	FLinearColor DefeatColor = FLinearColor(0.3f, 0.01f, 0.03f);

	UPROPERTY(EditDefaultsOnly, Category = "Result")
	FLinearColor DrawColor = FLinearColor::White;

	UPROPERTY(EditDefaultsOnly, Category = "Result")
	float FadeInDuration = 1.5f;

	float ResultShownTime = -1.f;

	void SetResultOpacity(float Opacity);

	uint8 GetOwnTeam() const;
};
