#include "Widgets/MatchHUDWidget.h"
#include "Components/TextBlock.h"
#include "Framework/MGameState.h"
#include "Player/MPlayerState.h"

void UMatchHUDWidget::NativeConstruct(){
	/** Hides the result and the darkening until the match ends. */
	Super::NativeConstruct();

	if (ResultText) ResultText->SetVisibility(ESlateVisibility::Collapsed);
	if (ResultOverlay) ResultOverlay->SetVisibility(ESlateVisibility::Collapsed);
}

void UMatchHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime){
	/** Shows the score from the point of view of the local player's team and the time left, both read from the replicated game state. */
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (ResultShownTime >= 0.f){
		const float Elapsed = GetWorld()->GetTimeSeconds() - ResultShownTime;
		SetResultOpacity(FadeInDuration > 0.f ? FMath::Clamp(Elapsed / FadeInDuration, 0.f, 1.f) : 1.f);
	}

	const AMGameState* GameState = GetWorld()->GetGameState<AMGameState>();
	if (!GameState) return;

	const uint8 OwnTeam = GetOwnTeam();
	ScoreText->SetText(FText::FromString(FString::Printf(TEXT("You %d : %d Enemy"), GameState->GetTeamScore(OwnTeam), GameState->GetTeamScore(1 - OwnTeam))));

	const float TimeLeft = GameState->IsMatchOver() ? 0.f : GameState->GetMatchEndServerTime() - GameState->GetServerWorldTimeSeconds();
	const int32 SecondsLeft = FMath::Max(FMath::CeilToInt(TimeLeft), 0);
	TimerText->SetText(FText::FromString(FString::Printf(TEXT("%d:%02d"), SecondsLeft / 60, SecondsLeft % 60)));
}

void UMatchHUDWidget::ShowResult(int32 WinningTeam){
	/** Darkens the screen and fades in victory in green, defeat in red or draw depending on the winning team and the team of the local player. */
	if (!ResultText) return;

	const bool bDraw = WinningTeam == INDEX_NONE;
	const bool bVictory = !bDraw && WinningTeam == GetOwnTeam();
	ResultText->SetText(FText::FromString(bDraw ? TEXT("DRAW") : (bVictory ? TEXT("VICTORY") : TEXT("DEFEAT"))));
	ResultText->SetColorAndOpacity(FSlateColor(bDraw ? DrawColor : (bVictory ? VictoryColor : DefeatColor)));
	ResultText->SetVisibility(ESlateVisibility::HitTestInvisible);
	if (ResultOverlay) ResultOverlay->SetVisibility(ESlateVisibility::HitTestInvisible);

	ResultShownTime = GetWorld()->GetTimeSeconds();
	SetResultOpacity(0.f);
}

void UMatchHUDWidget::SetResultOpacity(float Opacity){
	/** Sets the opacity of the result text and the darkening together. */
	ResultText->SetRenderOpacity(Opacity);
	if (ResultOverlay) ResultOverlay->SetRenderOpacity(Opacity);
}

uint8 UMatchHUDWidget::GetOwnTeam() const{
	/** Returns the team of the local player, or the first team while the player state has not arrived yet. */
	const AMPlayerState* MPlayerState = GetOwningPlayerState<AMPlayerState>();
	return MPlayerState && MPlayerState->GetTeamID().GetId() < 2 ? MPlayerState->GetTeamID().GetId() : 0;
}
