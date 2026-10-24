
#include "MPlayerCharacter.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"


AMPlayerCharacter::AMPlayerCharacter(){
	/** Creates the camera on a spring arm and makes the character turn towards its movement. */
	SpringArm = CreateDefaultSubobject<USpringArmComponent>("Spring Arm");
	SpringArm->SetupAttachment(GetRootComponent());
	SpringArm->bUsePawnControlRotation = true;
	
	Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0, 720.0, 0);
}

void AMPlayerCharacter::PawnClientRestart(){
	/** Adds the movement input mapping for the local player. */
	Super::PawnClientRestart();
	APlayerController* PlayerController = GetController<APlayerController>();
	if (PlayerController){
		UEnhancedInputLocalPlayerSubsystem* InputSubsystem = PlayerController->GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
		if (InputSubsystem){
			InputSubsystem->RemoveMappingContext(InputMappingContext);
			InputSubsystem->AddMappingContext(InputMappingContext, 0);
		}
	}
	
}

void AMPlayerCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent){
	/** Binds the jump, look, move and attack input actions. */
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (EnhancedInputComponent){
		EnhancedInputComponent->BindAction(JumpInputAction, ETriggerEvent::Triggered, this, &AMPlayerCharacter::Jump);
		EnhancedInputComponent->BindAction(LookInputAction, ETriggerEvent::Triggered, this, &AMPlayerCharacter::HandleLookInput);
		EnhancedInputComponent->BindAction(MoveInputAction, ETriggerEvent::Triggered, this, &AMPlayerCharacter::HandleMoveInput);
		EnhancedInputComponent->BindAction(AttackInputAction, ETriggerEvent::Started, this, &AMPlayerCharacter::RequestAttack);
	}
}

void AMPlayerCharacter::HandleLookInput(const FInputActionValue& Input){
	/** Rotates the camera from mouse or stick input. */
	FVector2D InputValue = Input.Get<FVector2D>();
	
	AddControllerPitchInput(InputValue.Y);
	AddControllerYawInput(InputValue.X);
}

void AMPlayerCharacter::HandleMoveInput(const FInputActionValue& Input){
	/** Moves the character relative to the camera direction. */
	FVector2D InputValue = Input.Get<FVector2D>();
	InputValue.Normalize();

	AddMovementInput(GetMoveForwardDirection()*InputValue.Y + GetLookRightDirection()*InputValue.X);
}

FVector AMPlayerCharacter::GetLookRightDirection(){
	/** Returns the right direction of the camera. */
	return Camera->GetRightVector();
}

FVector AMPlayerCharacter::GetLookForwardDirection(){
	/** Returns the forward direction of the camera. */
	return Camera->GetForwardVector();
}

FVector AMPlayerCharacter::GetMoveForwardDirection(){
	/** Returns the direction used for forward movement, built from the camera vectors. */
	return FVector::CrossProduct(GetLookRightDirection(), GetLookForwardDirection());
}
