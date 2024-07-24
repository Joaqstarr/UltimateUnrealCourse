// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/BaseCharacter.h"
#include "InputActionValue.h"
#include "Characters/CharacterTypes.h"
#include "SlashCharacter.generated.h"

class UInputMappingContext;
class UInputAction;
class UCameraComponent;
class USpringArmComponent;
class UGroomComponent;
class AItem;
class UAnimMontage;

UCLASS()
class ULTIMATECOURSE_API ASlashCharacter : public ABaseCharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASlashCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void ResetAttackState() override;
	virtual void GetHit_Implementation(const FVector& ImpactPoint) override;

	/* Input Callbacks */
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void Jumping(const FInputActionValue& Value);
	void SheathWeapon();
	void UnsheathWeapon();
	void ToggleArmWeapon();
	void EKeyPressed(const FInputActionValue& Value);
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void EquipWeapon(AWeapon* Weapon);

	virtual void Attack() override;
	
	/* Input Callbacks */
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputMappingContext* SlashContext;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* MovementAction;
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* LookingAction;
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* JumpAction;
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* EquipAction;
	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* AttackAction;

	/*
	*	Play Montage Functions
	*/
	void PlayEquipMontage(const FName& Section) const;
	UFUNCTION(BlueprintCallable)
	void AttachWeaponToBack() const;
	UFUNCTION(BlueprintCallable)
	void AttachWeaponToHand();
	UFUNCTION(BlueprintCallable)
	void ResetEquipState();

	virtual bool CanAttack() const override;

private:

	bool CanDisarm() const;
	bool CanArm() const;

	ECharacterState CharacterState = ECharacterState::ECS_Unequipped;
	UPROPERTY(VisibleInstanceOnly)
	EActionState ActionState = EActionState::EAS_Unoccupied;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCameraComponent> ViewCamera;

	UPROPERTY(VisibleAnywhere, Category = Hair)
	TObjectPtr<UGroomComponent> Hair;

	UPROPERTY(VisibleAnywhere, Category = Hair)
	TObjectPtr<UGroomComponent> Eyebrows;

	UPROPERTY(VisibleInstanceOnly)
	TObjectPtr<AItem> OverlappingItem;

	void DropWeapon();
	

	/*
	* ANIMATION MONTAGES
	*/

	UPROPERTY()
	TObjectPtr <UAnimMontage> EquipMontage;

	UFUNCTION(BlueprintCallable)
	void ResetHitReactState();
	
public:
	FORCEINLINE void SetOverlappingItem(AItem* Item) { OverlappingItem = Item; }
	FORCEINLINE TObjectPtr<AItem> GetOverlappingItem() const { return OverlappingItem; }
	FORCEINLINE ECharacterState GetCharacterState() const { return CharacterState; }
};
