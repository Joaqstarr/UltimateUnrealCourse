// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/IHitInterface.h"
#include "Characters/CharacterTypes.h"
#include "Enemy.generated.h"

class UPawnSensingComponent;
class UHealthBarComponent;
class UAnimMontage;
class USoundBase;
class UParticleSystem;
class UAttributeComponent;
class AAIController;

UCLASS()
class ULTIMATECOURSE_API AEnemy : public ACharacter, public IIHitInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemy();
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void GetHit_Implementation(const FVector& ImpactPoint) override;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	void PlayHitReactMontage(const FName& Section) const;
	void Die();
	UPROPERTY(BlueprintReadOnly)
	EDeathPose DeathPose = EDeathPose::EDP_Alive;
private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UAttributeComponent> Attributes;
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UHealthBarComponent> HealthBarComponent;
	
	UPROPERTY(EditDefaultsOnly, Category=Montages)
	TObjectPtr<UAnimMontage> ReactMontage;
	UPROPERTY(EditDefaultsOnly, Category=Montages)
	TObjectPtr<UAnimMontage> DeathMontage;
	FName GetDirectionFromHitPoint(const FVector& HitPoint) const;

	UPROPERTY(EditAnywhere)
	TObjectPtr<USoundBase> HitSound;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UParticleSystem> HitEffect;
	TObjectPtr<APawn> Damager;
	UPROPERTY(EditDefaultsOnly)
	float MaxHealthBarDistance = 1000;
	void CheckMaxHealthBarDist();
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

	/*
	 * Ai Aggro
	 */
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UPawnSensingComponent> PawnSensingComponent;
	UFUNCTION()
	void OnPawnSpotted( APawn* Pawn);
	
	EEnemyState EnemyState = EEnemyState::EES_Patrolling;
};
