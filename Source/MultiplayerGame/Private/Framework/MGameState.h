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

DECLARE_MULTICAST_DELEGATE_OneParam(
	FOnMatchOver,
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
	FOnMatchOver OnMatchOver;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	const TArray<FPlayerSelection>& GetPlayerSelectionChange() const;

	bool CanStart() const;

	void SetStartCountdown(int32 NewValue);
	FORCEINLINE int32 GetStartCountdown() const { return StartCountdown; }

	AMGameState();

	void StartMatch(float Duration);
	int32 AddTeamScore(uint8 Team);
	void EndMatch(int32 InWinningTeam);

	int32 GetTeamScore(uint8 Team) const;
	FORCEINLINE float GetMatchEndServerTime() const { return MatchEndServerTime; }
	FORCEINLINE bool IsMatchOver() const { return bMatchOver; }
	FORCEINLINE int32 GetWinningTeam() const { return WinningTeam; }
private:
	UPROPERTY(Replicated)
	TArray<int32> TeamScores;

	UPROPERTY(Replicated)
	float MatchEndServerTime = -1.f;

	UPROPERTY(ReplicatedUsing = OnRep_MatchOver)
	bool bMatchOver = false;

	UFUNCTION()
	void OnRep_MatchOver();

	UPROPERTY(Replicated)
	int32 WinningTeam = INDEX_NONE;

	UPROPERTY(ReplicatedUsing = OnRep_PlayerSelectionArray)
	TArray<FPlayerSelection> PlayerSelectionArray;

	UFUNCTION()
	void OnRep_PlayerSelectionArray();

	bool CanTakeSlot(const APlayerState* RequestingPlayer, uint8 DesiredSlot, FString& OutRejectReason) const;
	TArray<FPlayerSelection> GetSelectionsAfterMove(const APlayerState* RequestingPlayer, uint8 DesiredSlot) const;
	static bool HasPlayersInBothTeams(const TArray<FPlayerSelection>& Selections);

	UPROPERTY(ReplicatedUsing = OnRep_StartCountdown)
	int32 StartCountdown = -1;

	UFUNCTION()
	void OnRep_StartCountdown();
};