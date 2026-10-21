// Fill out your copyright notice in the Description page of Project Settings.


#include "Animations/MAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UMAnimInstance::NativeInitializeAnimation() {
	/** Caches the owning character and its movement component. */
	Super::NativeInitializeAnimation();
	OwnerCharacter = Cast<ACharacter>(TryGetPawnOwner());
	if (OwnerCharacter) OwnerMovementComponent = OwnerCharacter->GetCharacterMovement();
}

void UMAnimInstance::NativeUpdateAnimation(float DeltaSeconds) {
	/** Updates speed, turn rate, jump state and look offset used by the animation blueprint. */
	Super::NativeUpdateAnimation(DeltaSeconds);
	if (OwnerCharacter) {
		MovementSpeed = OwnerCharacter->GetVelocity().Length();
		FRotator BodyRotatation = OwnerCharacter->GetActorRotation();
		FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(BodyRotatation, PreviousRotationSpeed);
		PreviousRotationSpeed = BodyRotatation;

		YawSpeed = Delta.Yaw/DeltaSeconds;
		SmoothedYawSpeed = UKismetMathLibrary::FInterpTo(SmoothedYawSpeed, YawSpeed, DeltaSeconds, SmoothedYawLerpSpeed);

		FRotator ControlRotation = OwnerCharacter->GetBaseAimRotation();
		LookRotationOffset = UKismetMathLibrary::NormalizedDeltaRotator(ControlRotation, BodyRotatation);
	}

	if (OwnerMovementComponent) bIsJumping = OwnerMovementComponent->IsFalling();
}

void UMAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds) {
	/** Thread-safe animation update. */
	Super::NativeThreadSafeUpdateAnimation(DeltaSeconds);
}
