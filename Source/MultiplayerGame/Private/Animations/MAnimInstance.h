// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "MAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class UMAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	bool isMoving() const { return MovementSpeed != 0;}

	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	float GetSmoothedYawSpeed() const {return SmoothedYawSpeed;}

	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	bool GetIsJumping() const {return bIsJumping;}

	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	float GetLookYawOffset() const {return LookRotationOffset.Yaw;}

	UFUNCTION(BlueprintCallable, meta=(BlueprintThreadSafe))
	float GetLookPitchOffset() const {return LookRotationOffset.Pitch;}
	
private:
	UPROPERTY(EditAnywhere)
	class ACharacter* OwnerCharacter;

	UPROPERTY(EditAnywhere)
	class UCharacterMovementComponent* OwnerMovementComponent;

	UPROPERTY(EditAnywhere, Category = "Animation")
	float SmoothedYawLerpSpeed = 1.0f;
	
	float MovementSpeed;
	float YawSpeed;
	float SmoothedYawSpeed;

	bool bIsJumping;
	
	FRotator PreviousRotationSpeed;
	FRotator LookRotationOffset;
};
