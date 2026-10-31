#include "Cheats/MCheatManager.h"
#include "Character/MCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Player/LobbyPlayerController.h"
#include "MLogCategories.h"

void UMCheatManager::CheatSpamAttack(int32 Count){
	/** Test console command: sends several attack requests at once, skipping the client cooldown, to show that the server rejects them. */
	AMCharacter* Character = GetClientCharacter();
	if (!Character) return;

	UE_LOG(LogMCombat, Warning, TEXT("[Client] Sending %d attack requests without the cooldown check"), Count);
	for (int32 i = 0; i < Count; i++) Character->Server_Attack();
}

void UMCheatManager::CheatSpeed(float Multiplier){
	/** Test console command: makes the character faster only on the client to show that the server corrects the position with its own speed. */
	AMCharacter* Character = GetClientCharacter();
	if (!Character) return;

	UCharacterMovementComponent* Movement = Character->GetCharacterMovement();
	const float DefaultSpeed = Character->GetClass()->GetDefaultObject<AMCharacter>()->GetCharacterMovement()->MaxWalkSpeed;
	Movement->MaxWalkSpeed = DefaultSpeed * Multiplier;
	UE_LOG(LogMGame, Warning, TEXT("[Client] Walk speed set to %.0f only on the client"), Movement->MaxWalkSpeed);
}

void UMCheatManager::CheatTeleport(float Distance){
	/** Test console command: moves the character forward only on the client to show that the server moves it back. */
	AMCharacter* Character = GetClientCharacter();
	if (!Character) return;

	const FVector NewLocation = Character->GetActorLocation() + Character->GetActorForwardVector() * Distance;
	Character->SetActorLocation(NewLocation, true);
	UE_LOG(LogMGame, Warning, TEXT("[Client] Teleported %.0f units forward only on the client"), Distance);
}

void UMCheatManager::CheatSetHealth(float NewHealth){
	/** Test console command: changes the health only on the client to show that the server keeps its own value for damage and death. */
	AMCharacter* Character = GetClientCharacter();
	if (!Character) return;

	Character->Health = NewHealth;
	Character->UpdateHealthBar();
	UE_LOG(LogMCombat, Warning, TEXT("[Client] Health set to %.0f only on the client"), NewHealth);
}

void UMCheatManager::CheatRequestSlot(int32 SlotID){
	/** Test console command: asks for any slot number, also one outside the lobby or a taken one, without the checks of the lobby widget. */
	ALobbyPlayerController* LobbyPlayerController = GetLobbyPlayerController();
	if (!LobbyPlayerController) return;

	UE_LOG(LogMLobby, Warning, TEXT("[Client] Requesting slot %d without client checks"), SlotID);
	LobbyPlayerController->Server_RequestSlotSelectionChange(static_cast<uint8>(SlotID));
}

void UMCheatManager::CheatSpamSlot(int32 SlotID, int32 Count){
	/** Test console command: sends several requests for the same slot at once to show that the server limits their rate. */
	ALobbyPlayerController* LobbyPlayerController = GetLobbyPlayerController();
	if (!LobbyPlayerController) return;

	UE_LOG(LogMLobby, Warning, TEXT("[Client] Sending %d requests for slot %d at once"), Count, SlotID);
	for (int32 i = 0; i < Count; i++) LobbyPlayerController->Server_RequestSlotSelectionChange(static_cast<uint8>(SlotID));
}

ALobbyPlayerController* UMCheatManager::GetLobbyPlayerController() const{
	/** Returns the lobby controller of a client, or null on the server or outside the lobby. */
	return Cast<ALobbyPlayerController>(GetClientController());
}

APlayerController* UMCheatManager::GetClientController() const{
	/** Returns the owning controller on a client, or null on the server where the test commands would skip the network. */
	APlayerController* PlayerController = GetOuterAPlayerController();
	return PlayerController && !PlayerController->HasAuthority() ? PlayerController : nullptr;
}

AMCharacter* UMCheatManager::GetClientCharacter() const{
	/** Returns the character controlled by a client, or null on the server or without a character. */
	const APlayerController* PlayerController = GetClientController();
	return PlayerController ? PlayerController->GetPawn<AMCharacter>() : nullptr;
}