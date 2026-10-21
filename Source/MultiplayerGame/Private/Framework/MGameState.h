#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Player/PlayerInfoTypes.h"
#include "MGameState.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(
	FOnPlayerSelectionUpdated,
	const TArray<FPlayerSelection>&
);

DECLARE_MULTICAST_DELEGATE_OneParam(
	FOnStartCountdownUpdated,
	int32
);

UCLASS()
class MULTIPLAYERGAME_API AMGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	void RequestPlayerSelectionChange(APlayerState* RequestingPlayer, uint8 DesiredSlot);
	void RemovePlayerSelection(const APlayerState* LeavingPlayer);
	bool IsSlotTaken(uint8 SlotID) const;

	FOnPlayerSelectionUpdated OnPlayerSelectionUpdated;
	FOnStartCountdownUpdated OnStartCountdownUpdated;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	const TArray<FPlayerSelection>& GetPlayerSelectionChange() const;

	bool CanStart() const;

	void SetStartCountdown(int32 NewValue);
	FORCEINLINE int32 GetStartCountdown() const { return StartCountdown; }
private:
	UPROPERTY(ReplicatedUsing = OnRep_PlayerSelectionArray)
	TArray<FPlayerSelection> PlayerSelectionArray;

	UFUNCTION()
	void OnRep_PlayerSelectionArray();

	UPROPERTY(ReplicatedUsing = OnRep_StartCountdown)
	int32 StartCountdown = -1;

	UFUNCTION()
	void OnRep_StartCountdown();
};