// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/MCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Player/MPlayerState.h"
#include "Framework/MGameMode.h"
#include "Widgets/HealthBarWidget.h"
#include "MLogCategories.h"

AMCharacter::AMCharacter(){
	/** Enables ticking, turns off mesh collision and adds the health bar above the head; the team is set on possess. */
	PrimaryActorTick.bCanEverTick = true;
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	HealthBarComponent = CreateDefaultSubobject<UWidgetComponent>("Health Bar");
	HealthBarComponent->SetupAttachment(GetRootComponent());
	HealthBarComponent->SetWidgetSpace(EWidgetSpace::Screen);
	HealthBarComponent->SetDrawSize(FVector2D(120.f, 12.f));
	HealthBarComponent->SetRelativeLocation(FVector(0.f, 0.f, 110.f));
}

void AMCharacter::BeginPlay(){
	/** Starts with full health on the server and shows the current health on every machine. */
	Super::BeginPlay();

	if (HasAuthority()) Health = MaxHealth;
	UpdateHealthBar();
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
	/** Replicates the team so that clients know which characters are enemies, and the health for the health bars. */
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMCharacter, TeamID);
	DOREPLIFETIME(AMCharacter, Health);
	DOREPLIFETIME(AMCharacter, LastHitDirection);
}

float AMCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser){
	/** Reduces the health on the server only and remembers the direction of the hit; clients get both values through replication. */
	const float AppliedDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	if (!HasAuthority() || IsDead() || AppliedDamage <= 0.f) return 0.f;

	if (DamageCauser) LastHitDirection = (GetActorLocation() - DamageCauser->GetActorLocation()).GetSafeNormal2D();
	Health = FMath::Clamp(Health - AppliedDamage, 0.f, MaxHealth);
	UpdateHealthBar();

	UE_LOG(LogMCombat, Log, TEXT("[Server] %s has %.0f/%.0f health"), *GetOwnerName(), Health, MaxHealth);
	if (IsDead()){
		UE_LOG(LogMCombat, Log, TEXT("[Server] %s died"), *GetOwnerName());
		HandleDeath();
		if (AMGameMode* GameMode = GetWorld()->GetAuthGameMode<AMGameMode>()) GameMode->CharacterKilled(this, EventInstigator);
	}
	else{
		Multicast_PlayHitReact(GetHitDirection(DamageCauser));
	}

	return AppliedDamage;
}

void AMCharacter::OnRep_Health(){
	/** Updates the health bar on a client when the server changes the health, and handles the death once it reaches zero. */
	UpdateHealthBar();
	if (IsDead()) HandleDeath();
}

bool AMCharacter::IsDead() const{
	/** Returns true when the health has dropped to zero. */
	return Health <= 0.f;
}

void AMCharacter::HandleDeath(){
	/** Stops all movement and player input (the camera stays still), turns off the capsule, hides the health bar and lets the body fall as a ragdoll pushed away from the last hit, with the animation paused, so bones without physics bodies (eyelids, weapon) stop moving. Runs once on every machine; the ragdoll is only visual, so a dedicated server skips it. */
	if (bDeathHandled) return;
	bDeathHandled = true;

	GetCharacterMovement()->DisableMovement();
	if (APlayerController* PlayerController = GetController<APlayerController>()){
		if (PlayerController->IsLocalController()) DisableInput(PlayerController);
	}
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HealthBarComponent->SetVisibility(false);

	if (GetNetMode() != NM_DedicatedServer){
		GetMesh()->bPauseAnims = true;
		GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
		GetMesh()->SetSimulatePhysics(true);
		GetMesh()->AddImpulseToAllBodiesBelow(LastHitDirection * DeathImpulseStrength, NAME_None, true);
	}
}

void AMCharacter::UpdateHealthBar(){
	/** Shows the current share of health in the bar; there is no widget on a dedicated server. */
	if (UHealthBarWidget* HealthBarWidget = Cast<UHealthBarWidget>(HealthBarComponent->GetUserWidgetObject())){
		HealthBarWidget->SetHealthPercent(MaxHealth > 0.f ? Health / MaxHealth : 0.f);
	}
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
	if (IsDead() || !IsAttackReady()) return;

	LastAttackTime = GetWorld()->GetTimeSeconds();
	PlayMontage(AttackMontage);

	if (HasAuthority()) PerformAttack();
	else Server_Attack();
	
}

void AMCharacter::CheatSpamAttack(int32 Count){
	/** Test console command: sends several attack requests at once, skipping the client cooldown, to show that the server rejects them. */
	if (HasAuthority()) return;

	UE_LOG(LogMCombat, Warning, TEXT("[Client] Sending %d attack requests without the cooldown check"), Count);
	for (int32 i = 0; i < Count; i++) Server_Attack();
}

void AMCharacter::Server_Attack_Implementation(){
	/** Runs on the server when the owning client attacks; death and the cooldown are checked again because the client cannot be trusted. */
	if (IsDead()){
		UE_LOG(LogMCombat, Warning, TEXT("[Server] Rejected attack from %s: character is dead"), *GetOwnerName());
		return;
	}
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
	PlayMontage(AttackMontage);
}

void AMCharacter::Multicast_PlayHitReact_Implementation(EHitDirection Direction){
	/** Plays the hit reaction for the given side on all machines, including the owner of the hit character, since only the server knows about the hit. */
	if (UAnimMontage* const* Montage = HitReactMontages.Find(Direction)) PlayMontage(*Montage);
}

EHitDirection AMCharacter::GetHitDirection(const AActor* DamageCauser) const{
	/** Returns the side of the character the attacker stands on, comparing the direction to the attacker with the forward and right vectors. */
	if (!DamageCauser) return EHitDirection::Front;

	const FVector ToCauser = (DamageCauser->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
	const float ForwardDot = FVector::DotProduct(GetActorForwardVector(), ToCauser);
	const float RightDot = FVector::DotProduct(GetActorRightVector(), ToCauser);

	if (FMath::Abs(ForwardDot) >= FMath::Abs(RightDot)) return ForwardDot >= 0.f ? EHitDirection::Front : EHitDirection::Back;
	return RightDot >= 0.f ? EHitDirection::Right : EHitDirection::Left;
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
		if (!Target || DamagedActors.Contains(Target) || Target->IsDead() || Target->GetGenericTeamId() == TeamID) continue;

		DamagedActors.Add(Target);
		UGameplayStatics::ApplyDamage(Target, AttackDamage, GetController(), this, UDamageType::StaticClass());
		UE_LOG(LogMCombat, Log, TEXT("[Server] %s hit %s for %.0f damage"), *GetOwnerName(), *Target->GetOwnerName(), AttackDamage);
	}

	if (DamagedActors.IsEmpty()){
		UE_LOG(LogMCombat, Log, TEXT("[Server] %s attacked and missed"), *GetOwnerName());
	}
}

void AMCharacter::PlayMontage(UAnimMontage* Montage){
	/** Plays the given montage on the character mesh; skipped on a dedicated server where nobody sees it. */
	if (GetNetMode() == NM_DedicatedServer) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && Montage) AnimInstance->Montage_Play(Montage);
}
