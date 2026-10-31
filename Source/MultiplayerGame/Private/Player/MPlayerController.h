#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MPlayerController.generated.h"

UCLASS()
class AMPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AMPlayerController();

	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* NewPawn) override;
	virtual void AcknowledgePossession(APawn* NewPawn) override;

	UFUNCTION(Client, Reliable)
	void Client_ShowDeathScreen(const FString& KillerName, float RespawnServerTime);

private:
	void BindMatchState();
	void HandleMatchOver(int32 WinningTeam);

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UMatchHUDWidget> MatchHUDWidgetClass;

	UPROPERTY()
	class UMatchHUDWidget* MatchHUDWidget;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UDeathScreenWidget> DeathScreenWidgetClass;

	UPROPERTY()
	class UDeathScreenWidget* DeathScreenWidget;

	void HideDeathScreen();
};