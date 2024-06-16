// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/BaseCharacter.h"
#include "Characters/CharacterTypes.h"
#include "Enemy.generated.h"

class UPawnSensingComponent;
class UHealthBarComponent;
class AAIController;
class AWeapon;

UCLASS()
class ULTIMATECOURSE_API AEnemy : public ABaseCharacter
{
	GENERATED_BODY()

public:
	AEnemy();
	virtual void Tick(float DeltaTime) override;

	virtual void GetHit_Implementation(const FVector& ImpactPoint) override;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	virtual void Destroyed() override;
	virtual void HandleDamage(float DamageAmount) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Die() override;
	UPROPERTY(BlueprintReadOnly)
	EDeathPose DeathPose;
	UPROPERTY(BlueprintReadOnly)
	EEnemyState EnemyState = EEnemyState::EES_Patrolling;
	virtual void Attack() override;
	virtual void PlayAttackMontage() override;
	bool IsDead() const;
	virtual  bool CanAttack() const override;
private:

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UHealthBarComponent> HealthBarComponent;


	TObjectPtr<APawn> CombatTarget;
	UPROPERTY(EditDefaultsOnly, Category=Combat)
	float CombatRadius = 1000;
	UPROPERTY(EditDefaultsOnly, Category=Combat)
	float AttackRadius = 150;
	void CheckCombatTarget();

	bool InTargetRange(const AActor* Target, const float Radius) const;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AWeapon> WeaponClass;

	void SetHealthBarVisibility(bool Visibility) const;

	/*
	 *Navigation
	 */


	UPROPERTY(EditInstanceOnly, Category="AI Navigation")
	TObjectPtr<AActor> CurrentPatrolTarget;
	UPROPERTY(EditInstanceOnly, Category="AI Navigation")
	TArray<AActor*> PatrolTargets;
	TObjectPtr<AAIController> AiController;
	UPROPERTY(EditDefaultsOnly, Category="AI Navigation")
	float AcceptableNavPointDistance = 15.f;

	int PointPosition = 0;
	void UpdatePatrolPoints();
	void MoveToTarget(TObjectPtr<AActor> Target) const;

	UPROPERTY(EditAnywhere, Category="AI Navigation")
	float PatrolWaitTime = 3.f;
	FTimerHandle PatrolTimer;
	void PatrolTimerFinished() const;
	void LoseInterest();
	void StartPatrolling();
	void ChaseTarget();
	bool IsOutsideAttackRadius() const;
	bool IsChasing() const;
	bool IsAttacking() const;

	/*
	 * Ai Aggro
	 */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UPawnSensingComponent> PawnSensingComponent;
	UFUNCTION()
	void OnPawnSpotted( APawn* Pawn);
	/*
	 *Combat
	 */

	void StartAttackTimer();
	void ClearAttackTimer();

	UPROPERTY(EditAnywhere, Category="Combat")
	float PatrollingSpeed = 125.f;
	UPROPERTY(EditAnywhere, Category="Combat")
	float ChasingSpeed = 300.f;

	FTimerHandle AttackTimer;
	UPROPERTY(EditAnywhere, Category="Combat")
	float AttackMin = 0.5f;
	UPROPERTY(EditAnywhere, Category="Combat")
	float AttackMax = 1.f;
	

};
