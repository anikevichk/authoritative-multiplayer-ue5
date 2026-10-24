// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GenericTeamAgentInterface.h"
#include "MCharacter.generated.h"

UCLASS()
class AMCharacter : public ACharacter, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	AMCharacter();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void SetGenericTeamId(const FGenericTeamId& NewTeamID) override;
	virtual FGenericTeamId GetGenericTeamId() const override;

	void RequestAttack();

private:
	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	class UAnimMontage* AttackMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	float AttackDamage = 20.f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	float AttackRange = 150.f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	float AttackRadius = 60.f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	float AttackCooldown = 0.8f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat")
	float AttackCooldownTolerance = 0.1f;

	float LastAttackTime = -1000.f;

	UFUNCTION(Server, Reliable)
	void Server_Attack();

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayAttack();

	bool IsAttackReady(float Tolerance = 0.f) const;
	void PerformAttack();
	void PlayAttackMontage();

	UPROPERTY(ReplicatedUsing = OnRep_TeamID)
	FGenericTeamId TeamID;

	UFUNCTION()
	void OnRep_TeamID();

	FGenericTeamId LoggedTeamID;
	void LogReceivedTeam();
	FString GetOwnerName() const;
};
