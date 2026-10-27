#include "Widgets/DeathScreenWidget.h"
#include "Components/TextBlock.h"
#include "GameFramework/GameStateBase.h"

void UDeathScreenWidget::Show(const FString& KillerName, float InRespawnServerTime){
	/** Shows the name of the killer and starts the fade-in; the respawn moment is given in server time. */
	RespawnServerTime = InRespawnServerTime;
	ShownTime = GetWorld()->GetTimeSeconds();
	SetRenderOpacity(0.f);

	if (KillerText){
		KillerText->SetText(KillerName.IsEmpty() ? FText::GetEmpty() : FText::FromString(FString::Printf(TEXT("Killed by %s"), *KillerName)));
	}
}

void UDeathScreenWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime){
	/** Fades the screen in and counts down the seconds left to the respawn using the server clock replicated by the game state. */
	Super::NativeTick(MyGeometry, InDeltaTime);

	const float Elapsed = GetWorld()->GetTimeSeconds() - ShownTime;
	SetRenderOpacity(FadeInDuration > 0.f ? FMath::Clamp(Elapsed / FadeInDuration, 0.f, 1.f) : 1.f);

	const AGameStateBase* GameState = GetWorld()->GetGameState();
	if (!GameState) return;

	const int32 SecondsLeft = FMath::Max(FMath::CeilToInt(RespawnServerTime - GameState->GetServerWorldTimeSeconds()), 0);
	RespawnText->SetText(FText::FromString(FString::Printf(TEXT("Respawn in %d"), SecondsLeft)));
}
