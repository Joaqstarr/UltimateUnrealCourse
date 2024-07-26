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

	/** <AActor> */
	virtual void Tick(float DeltaTime) override; 
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	virtual void Destroyed() override;

	/** </AActor> */

	/** <IHitInterface> */
	virtual void GetHit_Implementation(const FVector& ImpactPoint, const AActor* Hitter) override;

protected:
	// Called when the game starts or when spawned
	/** <AActor> */
	virtual void BeginPlay() override;
	/** </AActor> */
	/** <ABaseCharacter> */
	virtual void Die() override;
	virtual void Attack() override;
	virtual  bool CanAttack() const override;
	virtual void ResetAttackState() override;
	virtual void HandleDamage(float DamageAmount) override;
	virtual int32 PlayDeathMontage() override;

	/** </ABaseCharacter> */

	
	UPROPERTY(BlueprintReadOnly)
	EDeathPose DeathPose;
	
	UPROPERTY(BlueprintReadOnly)
	EEnemyState EnemyState = EEnemyState::EES_Patrolling;

	UPROPERTY(BlueprintReadOnly, Category=Combat)
    TObjectPtr<APawn> CombatTarget;

private:
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UHealthBarComponent> HealthBarComponent;
	void SetHealthBarVisibility(bool Visibility) const;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<AWeapon> WeaponClass;
	
	/** AI Behavior */


	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UPawnSensingComponent> PawnSensingComponent;

	UPROPERTY(EditDefaultsOnly, Category=Combat)
	float CombatRadius = 1000;
	UPROPERTY(EditDefaultsOnly, Category=Combat)
	float AttackRadius = 150;

	UPROPERTY(EditInstanceOnly, Category="AI Navigation")
	TObjectPtr<AActor> CurrentPatrolTarget;
	UPROPERTY(EditInstanceOnly, Category="AI Navigation")
	TArray<AActor*> PatrolTargets;
	TObjectPtr<AAIController> AiController;
	UPROPERTY(EditDefaultsOnly, Category="AI Navigation")
	float AcceptableNavPointDistance = 15.f;
	
	/*
	 *Navigation
	*/
		
	int PointPosition = 0;
	UPROPERTY(EditAnywhere, Category="AI Navigation")
	float PatrolWaitTime = 3.f;
	FTimerHandle PatrolTimer;

	
	void InitializeEnemy();
	
	void UpdatePatrolPoints();
	void CheckCombatTarget();
	

	void PatrolTimerFinished() const;
	void ClearPatrolTimer();
	void LoseInterest();
	void StartPatrolling();
	void ChaseTarget();
	bool IsOutsideAttackRadius() const;
	bool IsChasing() const;
	bool IsAttacking() const;
	bool InTargetRange(const AActor* Target, const float Radius) const;
	bool IsDead() const;
	bool IsEngaged() const;
	void SpawnDefaultWeapon();

	void MoveToTarget(TObjectPtr<AActor> Target) const;
	/*
	 *Combat
	 */

	UFUNCTION()
	void OnPawnSpotted( APawn* Pawn); //callback for pawn sensor
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
	UPROPERTY(EditAnywhere, Category=Combat)
	float DeathLifeSpan = 8.f;



};
