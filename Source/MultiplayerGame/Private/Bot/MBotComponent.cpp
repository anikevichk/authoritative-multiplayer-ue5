#include "Bot/MBotComponent.h"
#include "Character/MCharacter.h"
#include "EngineUtils.h"
#include "Framework/MGameState.h"
#include "Framework/MTeamStatics.h"
#include "GameFramework/PlayerState.h"
#include "Misc/CommandLine.h"
#include "Player/LobbyPlayerController.h"
#include "MLogCategories.h"

UMBotComponent::UMBotComponent(){
	/** Enables ticking; the bot makes its decisions every frame on the client it runs on. */
	PrimaryComponentTick.bCanEverTick = true;
}

bool UMBotComponent::IsBotProcess(){
	/** Returns true when the client was started with the -bot flag. */
	return FParse::Param(FCommandLine::Get(), TEXT("bot"));
}

void UMBotComponent::AddToController(APlayerController* PlayerController){
	/** Adds the bot to the local controller of a bot client; the bot sends the same requests as a human player, so the server checks it the same way. */
	if (!PlayerController || !PlayerController->IsLocalController() || !IsBotProcess()) return;
	if (PlayerController->FindComponentByClass<UMBotComponent>()) return;

	UMBotComponent* Bot = NewObject<UMBotComponent>(PlayerController, TEXT("BotComponent"));
	Bot->RegisterComponent();
	UE_LOG(LogMBot, Log, TEXT("[Bot] Controlling %s"), *PlayerController->GetName());
}

void UMBotComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction){
	/** Picks a lobby slot while in the lobby and fights while controlling a character in the match. */
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	APlayerController* PlayerController = Cast<APlayerController>(GetOwner());
	if (!PlayerController) return;

	if (ALobbyPlayerController* LobbyController = Cast<ALobbyPlayerController>(PlayerController)){
		TickLobby(LobbyController);
		return;
	}
	if (AMCharacter* Character = PlayerController->GetPawn<AMCharacter>()) TickMatch(Character);
}

void UMBotComponent::TickLobby(ALobbyPlayerController* LobbyController){
	/** Asks the server for a free slot until the replicated selections show that the bot holds one, then moves to the smaller team whenever the teams become uneven. */
	const float Now = GetWorld()->GetTimeSeconds();
	if (Now - LastSlotRequestTime < SlotRequestInterval) return;

	const AMGameState* MGameState = GetWorld()->GetGameState<AMGameState>();
	const APlayerState* OwnPlayerState = LobbyController->PlayerState;
	if (!MGameState || !OwnPlayerState) return;

	const FPlayerSelection* OwnSelection = MGameState->GetPlayerSelectionChange().FindByPredicate([&](const FPlayerSelection& PlayerSelection){
		return PlayerSelection.BelongsToPlayer(OwnPlayerState);
	});

	const int32 Slot = OwnSelection ? ChooseBalancingSlot(MGameState, *OwnSelection) : ChooseFreeSlot(MGameState);
	if (Slot == INDEX_NONE) return;

	LastSlotRequestTime = Now;
	if (OwnSelection) {
		UE_LOG(LogMBot, Log, TEXT("[Bot] Teams are uneven, moving from slot %d to slot %d"), OwnSelection->GetSlot(), Slot);
	}
	else {
		UE_LOG(LogMBot, Log, TEXT("[Bot] Requesting slot %d"), Slot);
	}

	LobbyController->Server_RequestSlotSelectionChange(static_cast<uint8>(Slot));
}

int32 UMBotComponent::ChooseFreeSlot(const AMGameState* MGameState) const{
	/** Returns a free slot in the team with fewer players, so bots fill both teams evenly, or INDEX_NONE when the lobby is full. */
	int32 TeamCounts[2];
	CountTeamPlayers(MGameState, TeamCounts);

	const uint8 PreferredTeam = TeamCounts[1] < TeamCounts[0] ? 1 : 0;
	const int32 Slot = FindFreeSlotInTeam(MGameState, PreferredTeam);
	return Slot != INDEX_NONE ? Slot : FindFreeSlotInTeam(MGameState, static_cast<uint8>(1 - PreferredTeam));
}

int32 UMBotComponent::ChooseBalancingSlot(const AMGameState* MGameState, const FPlayerSelection& OwnSelection) const{
	/** Returns a free slot in the other team when the own team has at least two players more and this bot holds the highest bot slot in it, so that only one bot moves at a time; otherwise INDEX_NONE. */
	const uint8 OwnTeam = UMTeamStatics::GetTeamForSlot(OwnSelection.GetSlot());
	if (OwnTeam > 1) return INDEX_NONE;
	const uint8 OtherTeam = 1 - OwnTeam;

	int32 TeamCounts[2];
	CountTeamPlayers(MGameState, TeamCounts);
	if (TeamCounts[OwnTeam] - TeamCounts[OtherTeam] < 2) return INDEX_NONE;

	for (const FPlayerSelection& PlayerSelection : MGameState->GetPlayerSelectionChange()){
		const bool bSameTeam = UMTeamStatics::GetTeamForSlot(PlayerSelection.GetSlot()) == OwnTeam;
		if (bSameTeam && PlayerSelection.GetSlot() > OwnSelection.GetSlot() && IsBotSelection(MGameState, PlayerSelection)) return INDEX_NONE;
	}
	return FindFreeSlotInTeam(MGameState, OtherTeam);
}

void UMBotComponent::CountTeamPlayers(const AMGameState* MGameState, int32 OutTeamCounts[2]){
	/** Counts the players who hold a slot in each team. */
	OutTeamCounts[0] = OutTeamCounts[1] = 0;
	for (const FPlayerSelection& PlayerSelection : MGameState->GetPlayerSelectionChange()){
		const uint8 Team = UMTeamStatics::GetTeamForSlot(PlayerSelection.GetSlot());
		OutTeamCounts[Team]++;
	}
}

int32 UMBotComponent::FindFreeSlotInTeam(const AMGameState* MGameState, uint8 Team){
	/** Returns the first free slot of the team, or INDEX_NONE when the team is full. */
	const uint8 FirstSlot = Team * UMTeamStatics::GetPlayerCountPerTeam();
	for (uint8 Slot = FirstSlot; Slot < FirstSlot + UMTeamStatics::GetPlayerCountPerTeam(); Slot++){
		if (!MGameState->IsSlotTaken(Slot)) return Slot;
	}
	return INDEX_NONE;
}

bool UMBotComponent::IsBotSelection(const AMGameState* MGameState, const FPlayerSelection& PlayerSelection){
	/** Returns true when the slot belongs to a bot; the bot flag is set by the server and replicated with the player state. */
	for (const APlayerState* PlayerState : MGameState->PlayerArray){
		if (PlayerSelection.BelongsToPlayer(PlayerState)) return PlayerState->IsABot();
	}
	return false;
}

void UMBotComponent::TickMatch(AMCharacter* Character){
	/** Walks towards the nearest living enemy and attacks it once it is in range and the reaction time has passed; movement and attacks go through the same input and RPCs as a human player. */
	const AMGameState* MGameState = GetWorld()->GetGameState<AMGameState>();
	if (Character->IsDead() || (MGameState && MGameState->IsMatchOver())) return;

	const float Now = GetWorld()->GetTimeSeconds();
	if (!Target.IsValid() || Target->IsDead() || Now - LastTargetSearchTime >= TargetSearchInterval){
		LastTargetSearchTime = Now;
		AMCharacter* NewTarget = FindNearestEnemy(Character);
		if (NewTarget != Target.Get()){
			Target = NewTarget;
			TargetAcquiredTime = Now;
			if (NewTarget) UE_LOG(LogMBot, Log, TEXT("[Bot] New target %s"), *NewTarget->GetName());
		}
	}
	if (!Target.IsValid()) return;

	const FVector ToTarget = Target->GetActorLocation() - Character->GetActorLocation();
	Character->AddMovementInput(ToTarget.GetSafeNormal2D());

	const bool bInRange = ToTarget.Size2D() <= Character->GetAttackRange() * AttackRangeFactor;
	if (bInRange && Now - TargetAcquiredTime >= ReactionTime) Character->RequestAttack();
}

AMCharacter* UMBotComponent::FindNearestEnemy(const AMCharacter* Character) const{
	/** Returns the closest living character from the other team, using only the state replicated to this client. */
	AMCharacter* NearestEnemy = nullptr;
	float NearestDistanceSquared = TNumericLimits<float>::Max();

	for (AMCharacter* Other : TActorRange<AMCharacter>(GetWorld())){
		if (Other == Character || Other->IsDead() || Other->GetGenericTeamId() == Character->GetGenericTeamId()) continue;

		const float DistanceSquared = FVector::DistSquared(Other->GetActorLocation(), Character->GetActorLocation());
		if (DistanceSquared < NearestDistanceSquared){
			NearestDistanceSquared = DistanceSquared;
			NearestEnemy = Other;
		}
	}
	return NearestEnemy;
}
