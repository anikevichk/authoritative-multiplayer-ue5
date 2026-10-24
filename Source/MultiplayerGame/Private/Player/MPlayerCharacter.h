// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/MCharacter.h"
#include "InputActionValue.h"
#include "MPlayerCharacter.generated.h"
/**
 * 
 */
UCLASS()
class MULTIPLAYERGAME_API AMPlayerCharacter : public AMCharacter
{
	GENERATED_BODY()
public:
	AMPlayerCharacter();
	virtual void PawnClientRestart() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	UPROPERTY(VisibleAnywhere, Category = "View")
	class USpringArmComponent* SpringArm;
	
	UPROPERTY(VisibleAnywhere, Category = "View")
	class UCameraComponent* Camera;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* JumpInputAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* LookInputAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* MoveInputAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputAction* AttackInputAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	class UInputMappingContext* InputMappingContext;

	void HandleLookInput(const FInputActionValue& Input);
	void HandleMoveInput(const FInputActionValue& Input);

	FVector GetLookRightDirection();
	FVector GetLookForwardDirection();
	FVector GetMoveForwardDirection();
};
