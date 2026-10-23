// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/MCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "Player/MPlayerState.h"
#include "MLogCategories.h"

AMCharacter::AMCharacter(){
	/** Enables ticking and turns off mesh collision; the team is set on possess. */
	PrimaryActorTick.bCanEverTick = true;
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AMCharacter::BeginPlay(){
	Super::BeginPlay();
}

void AMCharacter::Tick(float DeltaTime){
	Super::Tick(DeltaTime);

}

void AMCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent){
	/** Input is bound in AMPlayerCharacter. */
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AMCharacter::PossessedBy(AController* NewController){
	/** Takes the team chosen in the lobby from the player state of the new controller. Server only. */
	Super::PossessedBy(NewController);

	if (const AMPlayerState* MPlayerState = NewController->GetPlayerState<AMPlayerState>() ){
		SetGenericTeamId(MPlayerState->GetTeamID());
	}
	UE_LOG(LogMTeam, Log, TEXT("[Server] %s is in team %d (%s)"), *GetOwnerName(), TeamID.GetId(), *GetName());
}

void AMCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const{
	/** Replicates the team so that clients know which characters are enemies. */
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMCharacter, TeamID);
}

void AMCharacter::SetGenericTeamId(const FGenericTeamId& NewTeamID){
	/** Sets the team of the character; called on possess by a player or an AI controller. */
	TeamID = NewTeamID;
}

void AMCharacter::OnRep_PlayerState(){
	/** Logs the team on a client once the player state with the player name has arrived. */
	Super::OnRep_PlayerState();
	LogReceivedTeam();
}

void AMCharacter::OnRep_TeamID(){
	/** Logs the team received from the server on a client. */
	LogReceivedTeam();
}

void AMCharacter::LogReceivedTeam(){
	/** Logs the team once both the team and the player name are known, and only when the team changes. */
	if (!GetPlayerState() || TeamID == FGenericTeamId::NoTeam || TeamID == LoggedTeamID) return;

	LoggedTeamID = TeamID;
	const TCHAR* Ownership = GetLocalRole() == ROLE_AutonomousProxy ? TEXT("own") : TEXT("remote");
	UE_LOG(LogMTeam, Log, TEXT("[Client] %s is in team %d (%s, %s)"), *GetOwnerName(), TeamID.GetId(), *GetName(), Ownership);
}

FString AMCharacter::GetOwnerName() const{
	/** Returns the name of the player or bot that controls the character. */
	const APlayerState* OwnerPlayerState = GetPlayerState();
	return OwnerPlayerState ? OwnerPlayerState->GetPlayerName() : TEXT("uncontrolled character");
}

FGenericTeamId AMCharacter::GetGenericTeamId() const{
	/** Returns the team of the character used by AI perception. */
	return TeamID;
}
