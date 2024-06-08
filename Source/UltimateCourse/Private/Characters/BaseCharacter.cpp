// Fill out your copyright notice in the Description page of Project Settings.
#include "Characters/BaseCharacter.h"
#include "Items/Weapons/Weapon.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/AttributeComponent.h"

ABaseCharacter::ABaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	Attributes = CreateDefaultSubobject<UAttributeComponent>(FName("Attributes"));

}

void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABaseCharacter::PlayAttackMontage()
{
}
void ABaseCharacter::PlayHitReactMontage(const FName& Section) const
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(!AnimInstance || !ReactMontage)return;

	AnimInstance->Montage_Play(ReactMontage);
	AnimInstance->Montage_JumpToSection(Section);
}

bool ABaseCharacter::CanAttack() const
{
	return true;
}

void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ABaseCharacter::UpdateWeaponCollision(bool collisionTo)
{
	if(EquippedWeapon)
	{
		EquippedWeapon->UpdateWeaponCollision(collisionTo);
	}
}

void ABaseCharacter::Attack()
{
}

void ABaseCharacter::Die()
{
}

void ABaseCharacter::ResetAttackState()
{
}

FName ABaseCharacter::GetDirectionFromHitPoint(const FVector& HitPoint) const
{
	const FVector Forward = GetActorForwardVector();
	const FVector ImpactLowered(HitPoint.X, HitPoint.Y, GetActorLocation().Z);
	const FVector ToHit = (ImpactLowered - GetActorLocation()).GetSafeNormal();
	
	float Angle = UKismetMathLibrary::Acos(FVector::DotProduct(Forward, ToHit));
	Angle = UKismetMathLibrary::RadiansToDegrees(Angle);
	const float AngleAbs = UKismetMathLibrary::Abs(Angle);

	const FVector CrossProduct = FVector::CrossProduct(Forward, ToHit);
	if(CrossProduct.Z < 0)
	{
		Angle *= -1;
	}
	
	if(AngleAbs <45)
	{
		return FName("Front");
	}
	if(AngleAbs < 135)
	{
		if(Angle > 0)
		{
			return FName("Right");
		}else
		{
			return FName("Left");
		}
	}
	return FName("Back");
}


