// Fill out your copyright notice in the Description page of Project Settings.
#include "Characters/BaseCharacter.h"
#include "Items/Weapons/Weapon.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/CapsuleComponent.h"
#include "Components/AttributeComponent.h"

ABaseCharacter::ABaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	Attributes = CreateDefaultSubobject<UAttributeComponent>(FName("Attributes"));
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
}

void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

int32 ABaseCharacter::PlayAttackMontage()
{
	return PlayRandomMontageSection(AttackMontage, AttackMontageSections);
}

int32 ABaseCharacter::PlayDeathMontage()
{
	return PlayRandomMontageSection(DeathMontage, DeathMontageSections);
}

void ABaseCharacter::DisableCapsule()
{
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

}

void ABaseCharacter::PlayHitReactMontage(const FName& Section) const
{
	if(!ReactMontage)return;

	PlayMontageSection(ReactMontage, Section);

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
	if(!EquippedWeapon)return;
	
	EquippedWeapon->UpdateWeaponCollision(collisionTo);
	
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

bool ABaseCharacter::IsAlive() const
{
	if(!Attributes)return true;
	return Attributes->IsAlive();
}

void ABaseCharacter::HandleDamage(float DamageAmount)
{
	if(Attributes)
	{
		Attributes->ReceiveDamage(DamageAmount);
	}
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



void ABaseCharacter::SetAttackMontage(TObjectPtr<UAnimMontage> newMontage)
{
	AttackMontage = newMontage;
}

void ABaseCharacter::PlayHitSound(const FVector& Location) const
{
	if(!HitSound)return;
	
	UGameplayStatics::PlaySoundAtLocation(this, HitSound, Location);
	
}

void ABaseCharacter::PlayHitParticle(const FVector& Location) const
{
	if(!HitEffect)return;
	
	UGameplayStatics::SpawnEmitterAtLocation(this, HitEffect, Location, FRotator::ZeroRotator, FVector(1), true, EPSCPoolMethod::None, true);
	
}

void ABaseCharacter::PlayMontageSection(TObjectPtr<UAnimMontage> Montage, const FName& SectionName) const
{
	if (const TObjectPtr<UAnimInstance> AnimInstance = GetMesh()->GetAnimInstance(); AnimInstance && Montage)
	{
		AnimInstance->Montage_Play(Montage);
		AnimInstance->Montage_JumpToSection(SectionName, Montage);
	}
}

int32 ABaseCharacter::PlayRandomMontageSection(TObjectPtr<UAnimMontage> Montage, const TArray<FName>& SectionNames) const
{
	if(SectionNames.Num() <= 0) return -1;
	
	
	const uint32 MaxSectionIndex = SectionNames.Num()-1;
	const uint32 Selection = FMath::RandRange(0, MaxSectionIndex);
	
	PlayMontageSection(Montage, SectionNames[Selection]);
	return Selection;
}


