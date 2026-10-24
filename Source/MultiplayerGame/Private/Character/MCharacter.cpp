// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/MCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Kismet/GameplayStatics.h"
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

void AMCharacter::RequestAttack(){
	/** Starts an attack: the owning client plays the animation at once (prediction) and asks the server to resolve the hit. */
	if (!IsAttackReady()) return;

	LastAttackTime = GetWorld()->GetTimeSeconds();
	PlayAttackMontage();

	if (HasAuthority()) PerformAttack();
	else Server_Attack();
	
}

void AMCharacter::Server_Attack_Implementation(){
	/** Runs on the server when the owning client attacks; the cooldown is checked again because the client cannot be trusted. */
	if (!IsAttackReady(AttackCooldownTolerance)){
		UE_LOG(LogMCombat, Warning, TEXT("[Server] Rejected attack from %s: cooldown not finished (%.2fs left)"),
			*GetOwnerName(), AttackCooldown - (GetWorld()->GetTimeSeconds() - LastAttackTime));
		return;
	}

	LastAttackTime = GetWorld()->GetTimeSeconds();
	PerformAttack();
}

void AMCharacter::Multicast_PlayAttack_Implementation(){
	/** Plays the attack animation on all machines except the one that already played it. */
	if (IsLocallyControlled()) return;
	PlayAttackMontage();
}

bool AMCharacter::IsAttackReady(float Tolerance) const{
	/** Returns true when the attack cooldown has passed; the tolerance absorbs network jitter between requests. */
	return GetWorld()->GetTimeSeconds() - LastAttackTime >= AttackCooldown - Tolerance;
}

void AMCharacter::PerformAttack(){
	/** Sweeps a sphere in front of the character and damages every enemy character it hits. Server only. */
	Multicast_PlayAttack();

	const FVector Start = GetActorLocation();
	const FVector End = Start + GetActorForwardVector() * AttackRange;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	TArray<FHitResult> Hits;
	GetWorld()->SweepMultiByObjectType(Hits, Start, End, FQuat::Identity,
		FCollisionObjectQueryParams(ECC_Pawn), FCollisionShape::MakeSphere(AttackRadius), QueryParams);

	TSet<AActor*> DamagedActors;
	for (const FHitResult& Hit : Hits){
		AMCharacter* Target = Cast<AMCharacter>(Hit.GetActor());
		if (!Target || DamagedActors.Contains(Target) || Target->GetGenericTeamId() == TeamID) continue;

		DamagedActors.Add(Target);
		UGameplayStatics::ApplyDamage(Target, AttackDamage, GetController(), this, UDamageType::StaticClass());
		UE_LOG(LogMCombat, Log, TEXT("[Server] %s hit %s for %.0f damage"), *GetOwnerName(), *Target->GetOwnerName(), AttackDamage);
	}

	if (DamagedActors.IsEmpty()){
		UE_LOG(LogMCombat, Log, TEXT("[Server] %s attacked and missed"), *GetOwnerName());
	}
}

void AMCharacter::PlayAttackMontage(){
	/** Plays the attack montage on the character mesh; skipped on a dedicated server where nobody sees it. */
	if (GetNetMode() == NM_DedicatedServer) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && AttackMontage) AnimInstance->Montage_Play(AttackMontage);
	
}
