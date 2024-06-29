// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/IHitInterface.h"
#include "BaseCharacter.generated.h"

class AWeapon;
class UAttributeComponent;
class UAnimMontage;
class USoundBase;
class UParticleSystem;

UCLASS()
class ULTIMATECOURSE_API ABaseCharacter : public ACharacter, public IIHitInterface
{
	GENERATED_BODY()

public:
	ABaseCharacter();
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable)
	void UpdateWeaponCollision(bool collisionTo);

	UFUNCTION(BlueprintCallable)
	virtual void ResetAttackState();
	bool IsAlive() const;
	virtual void HandleDamage( float DamageAmount);

protected:
	virtual void BeginPlay() override;
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UAttributeComponent> Attributes;

	
	UPROPERTY(VisibleAnywhere, Category = "Weapon")
	TObjectPtr<AWeapon> EquippedWeapon;

	virtual bool CanAttack() const;
	
	virtual void Attack();
	virtual void Die();
	void PlayHitReactMontage(const FName& Section) const;


	FName GetDirectionFromHitPoint(const FVector& HitPoint) const;

	/*
	* ANIMATION MONTAGES
	*/
	UPROPERTY(EditDefaultsOnly, Category=Montages)
	TObjectPtr<UAnimMontage> AttackMontage;
	UPROPERTY(EditDefaultsOnly, Category=Montages)
	TObjectPtr<UAnimMontage> ReactMontage;
	UPROPERTY(EditDefaultsOnly, Category=Montages)
	TObjectPtr<UAnimMontage> DeathMontage;

	UPROPERTY(EditAnywhere, Category=Combat)
	TArray<FName> AttackMontageSections;
	UPROPERTY(EditAnywhere, Category=Combat)
	TArray<FName> DeathMontageSections;
	
	void PlayHitSound(const FVector& Location) const;
	void PlayHitParticle(const FVector& Location) const;
	void PlayMontageSection(TObjectPtr<UAnimMontage> Montage, const FName& SectionName) const;
	int32 PlayRandomMontageSection(TObjectPtr<UAnimMontage> Montage, const TArray<FName>& SectionNames) const;

	int32 PlayAttackMontage();
	virtual int32 PlayDeathMontage();
	void DisableCapsule();

private:
	/*
	 *Effects
	 */
	UPROPERTY(EditAnywhere)
	TObjectPtr<USoundBase> HitSound;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UParticleSystem> HitEffect;
};
