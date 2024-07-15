// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/Item.h"
#include "Characters/CharacterTypes.h"
#include "Weapon.generated.h"

/**
 * 
 */
class USoundBase;
class UBoxComponent;
class UAnimMontage;

UCLASS()
class ULTIMATECOURSE_API AWeapon : public AItem
{
	GENERATED_BODY()
	
public:
	AWeapon();
	void Equip(TObjectPtr<USceneComponent> parent,FName socketName, TObjectPtr<AActor> newOwner, TObjectPtr<APawn> newInstigator);
	void DropWeapon(const FVector& Location);
	void AttachMeshToSocket(TObjectPtr<USceneComponent> parent, const FName& socketName) const;
	void UpdateWeaponCollision(bool collisionTo);
	TObjectPtr<UAnimMontage> GetAttackMontage() const;
	TObjectPtr<UAnimMontage> GetEquipMontage() const;
	EWeaponType GetWeaponType() const;

protected:
	virtual void BeginPlay() override;
	void PlayEquipSound();
	void DisableSphereCollision();
	void DeactivateEmbers();
	UFUNCTION(BlueprintCallable)
	void OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION(BlueprintImplementableEvent)
	void CreateFields(const FVector& FieldLocation);
	
private:
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	TObjectPtr<USoundBase> EquipSound;
	UPROPERTY(VisibleAnywhere, Category="Weapon Properties")
	TObjectPtr<UBoxComponent> WeaponBox;
	UPROPERTY(VisibleAnywhere, Category="Weapon Properties")
	TObjectPtr<USceneComponent> TraceStart;
	UPROPERTY(VisibleAnywhere, Category="Weapon Properties")
	TObjectPtr<USceneComponent> TraceEnd;
	
	void EnableAttackCollision() const;
	void DisableAttackCollision();
	TArray<TObjectPtr<AActor>> IgnoreActors;

	UPROPERTY(EditAnywhere, Category="Weapon Properties")
	float Damage = 20;

	UPROPERTY(EditAnywhere, Category="Animations")
	TObjectPtr<UAnimMontage> AttackMontage;
	UPROPERTY(EditAnywhere, Category="Animations")
	TObjectPtr<UAnimMontage> EquipMontage;

	UPROPERTY(EditAnywhere, Category="Weapon Properties")
	EWeaponType WeaponClass;

	FRotator DefaultRotation;

	void BoxTrace(FHitResult& HitResult);
	void ExecuteGetHit(FHitResult HitResult);
	bool ActorIsSameType(AActor* OtherActor);
	UPROPERTY(EditAnywhere, Category="Weapon Properties")
	FVector BoxTraceExtent = FVector(0.5f);
	UPROPERTY(EditAnywhere, Category="Weapon Properties")
	bool bShowBoxDebug = false;
};
