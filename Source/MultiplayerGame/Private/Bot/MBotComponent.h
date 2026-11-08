#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MBotComponent.generated.h"

class AMCharacter;

UCLASS()
class UMBotComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMBotComponent();

	static bool IsBotProcess();
	static void AddToController(class APlayerController* PlayerController);

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY(EditDefaultsOnly, Category = "Bot")
	float SlotRequestInterval = 1.f;

	UPROPERTY(EditDefaultsOnly, Category = "Bot")
	float TargetSearchInterval = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Bot")
	float ReactionTime = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category = "Bot")
	float AttackRangeFactor = 0.8f;

	float LastSlotRequestTime = -1000.f;
	float LastTargetSearchTime = -1000.f;
	float TargetAcquiredTime = 0.f;

	TWeakObjectPtr<AMCharacter> Target;

	void TickLobby(class ALobbyPlayerController* LobbyController);
	void TickMatch(AMCharacter* Character);

	int32 ChooseFreeSlot(const class AMGameState* MGameState) const;
	int32 ChooseBalancingSlot(const class AMGameState* MGameState, const struct FPlayerSelection& OwnSelection) const;
	static void CountTeamPlayers(const class AMGameState* MGameState, int32 OutTeamCounts[2]);
	static int32 FindFreeSlotInTeam(const class AMGameState* MGameState, uint8 Team);
	static bool IsBotSelection(const class AMGameState* MGameState, const struct FPlayerSelection& PlayerSelection);
	AMCharacter* FindNearestEnemy(const AMCharacter* Character) const;
};
