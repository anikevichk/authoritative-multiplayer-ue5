// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/MCharacter.h"
#include "Components/SkeletalMeshComponent.h"

AMCharacter::AMCharacter(){
	/** Enables ticking, turns off mesh collision and puts the character in team 0. */
	PrimaryActorTick.bCanEverTick = true;
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	TeamID = FGenericTeamId(0);
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


void AMCharacter::SetGenericTeamId(const FGenericTeamId& NewTeamID){
	/** Sets the team of the character; called by an AI controller on possess. */
	TeamID = NewTeamID;
}

FGenericTeamId AMCharacter::GetGenericTeamId() const{
	/** Returns the team of the character used by AI perception (team 0 by default). */
	return TeamID;
}
