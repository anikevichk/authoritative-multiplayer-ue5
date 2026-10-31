#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "MCheatManager.generated.h"

UCLASS()
class UMCheatManager : public UCheatManager
{
	GENERATED_BODY()

public:
	UFUNCTION(Exec)
	void CheatSpamAttack(int32 Count = 10);

	UFUNCTION(Exec)
	void CheatSpeed(float Multiplier = 3.f);

	UFUNCTION(Exec)
	void CheatTeleport(float Distance = 1000.f);

	UFUNCTION(Exec)
	void CheatSetHealth(float NewHealth = 1000.f);

	UFUNCTION(Exec)
	void CheatRequestSlot(int32 SlotID);

	UFUNCTION(Exec)
	void CheatSpamSlot(int32 SlotID, int32 Count = 10);

private:
	APlayerController* GetClientController() const;
	class AMCharacter* GetClientCharacter() const;
	class ALobbyPlayerController* GetLobbyPlayerController() const;
};
