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



protected:
	virtual void BeginPlay() override;
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UAttributeComponent> Attributes;
	
	UFUNCTION(BlueprintCallable)
	void UpdateWeaponCollision(bool collisionTo);
	
	UPROPERTY(VisibleAnywhere, Category = "Weapon")
	TObjectPtr<AWeapon> EquippedWeapon;
	
	
	virtual void Attack();
	virtual void Die();
	virtual bool CanAttack() const;
	void PlayHitReactMontage(const FName& Section) const;
	FName GetDirectionFromHitPoint(const FVector& HitPoint) const;
	void SetAttackMontage(TObjectPtr<UAnimMontage> newMontage);
	void PlayHitSound(const FVector& Location) const;
	void PlayHitParticle(const FVector& Location) const;
	void DisableCapsule();
	UFUNCTION(BlueprintCallable)
    virtual void ResetAttackState();
    bool IsAlive() const;
    virtual void HandleDamage( float DamageAmount);
	

	int32 PlayAttackMontage();
	virtual int32 PlayDeathMontage();
	

	

private:
	/*
	 *Effects
	 */
	UPROPERTY(EditAnywhere)
	TObjectPtr<USoundBase> HitSound;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UParticleSystem> HitEffect;

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

	void PlayMontageSection(TObjectPtr<UAnimMontage> Montage, const FName& SectionName) const;
	int32 PlayRandomMontageSection(TObjectPtr<UAnimMontage> Montage, const TArray<FName>& SectionNames) const;
};
