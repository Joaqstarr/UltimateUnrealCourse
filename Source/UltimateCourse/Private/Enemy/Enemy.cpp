// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Enemy.h"

#include "Components/AttributeComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "UltimateCourse/DebugMacros.h"
#include "Kismet/KismetSystemLibrary.h"
#include "HUD/HealthBarComponent.h"
#include "Kismet/GameplayStatics.h"
#include "AIController.h"

// Sets default values
AEnemy::AEnemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GetMesh()->SetCollisionObjectType(ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetGenerateOverlapEvents(true);

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	Attributes = CreateDefaultSubobject<UAttributeComponent>(FName("Attributes"));

	HealthBarComponent = CreateDefaultSubobject<UHealthBarComponent>(FName("HealthBar"));
	HealthBarComponent->SetupAttachment(GetRootComponent());
	HealthBarComponent->SetAbsolute(false, false, false);

	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationYaw = false;

}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	if(HealthBarComponent)
	{
		HealthBarComponent->SetVisibility(false);
		HealthBarComponent->SetHealthPercent(Attributes->GetHealthPercent());
	}
	AiController = Cast<AAIController>(GetController());

	if(AiController && CurrentPatrolTarget)
	{
		FAIMoveRequest MoveRequest;
		MoveRequest.SetGoalActor(CurrentPatrolTarget);
		MoveRequest.SetAcceptanceRadius(15.f);

		FNavPathSharedPtr NavPath;
		AiController->MoveTo(MoveRequest, &NavPath);
		TArray<FNavPathPoint>& PathPoints = NavPath->GetPathPoints();
		for(FNavPathPoint& Point : PathPoints)
		{
			const FVector& Location = Point.Location;
			DrawDebugSphere(GetWorld(), Location, 12.f, 12, FColor::Green, false, 10.f);
		}
	}
}

void AEnemy::PlayHitReactMontage(const FName& Section) const
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(!AnimInstance || !ReactMontage)return;

	AnimInstance->Montage_Play(ReactMontage);
	AnimInstance->Montage_JumpToSection(Section);
}

void AEnemy::Die()
{
	if(HealthBarComponent)
	{
		HealthBarComponent->SetVisibility(false);
	}
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	
	SetLifeSpan(5.f);
	if(!AnimInstance || !DeathMontage) return;
	FString SectionName = FString();
	const uint8 Selection = FMath::RandRange(1, 5);
	SectionName = FString("Death" + FString::FromInt(Selection));
	DeathPose = static_cast<EDeathPose>(Selection);
	
	AnimInstance->Montage_Play(DeathMontage);

	AnimInstance->Montage_JumpToSection(FName(SectionName));
}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if(Damager)
	{
		if(FVector::Distance( Damager->GetActorLocation(), GetActorLocation()) > MaxHealthBarDistance)
		{
			Damager = nullptr;
			if(HealthBarComponent)
			{
				HealthBarComponent->SetVisibility(false);
			}
		}
	}
}

// Called to bind functionality to input
void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AEnemy::GetHit_Implementation(const FVector& ImpactPoint)
{
	//DRAW_SPHERE_WITH_DETAILS(ImpactPoint, FColor::Green, 10, 4.f);
	if(Attributes && Attributes->IsAlive())
	{
		if(HealthBarComponent)
		{
			HealthBarComponent->SetVisibility(true);
		}
		PlayHitReactMontage(GetDirectionFromHitPoint(ImpactPoint));
	}else
	{
		
		Die();
	}

	if(HitSound)
		UGameplayStatics::PlaySoundAtLocation(this, HitSound, ImpactPoint);

	if(HitEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(this, HitEffect, ImpactPoint, FRotator::ZeroRotator, FVector(1), true, EPSCPoolMethod::None, true);
	}
}

float AEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
	AActor* DamageCauser)
{
	Damager = EventInstigator->GetPawn();
	if(Attributes)
	{
		Attributes->ReceiveDamage(DamageAmount);

		if(HealthBarComponent)
		{
			HealthBarComponent->SetHealthPercent(Attributes->GetHealthPercent());
		}
		return DamageAmount;

	}	
	
	return 0;
}

FName AEnemy::GetDirectionFromHitPoint(const FVector& HitPoint) const
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
	
	/*
	UKismetSystemLibrary::DrawDebugArrow(this, GetActorLocation(), GetActorLocation() + Forward*60.0f, 5.f, FColor::Red, 5.f);
	UKismetSystemLibrary::DrawDebugArrow(this, GetActorLocation(), GetActorLocation() + ToHit*60.0f, 5.f, FColor::Magenta, 5.f);
	if(GEngine)
	{
		GEngine->AddOnScreenDebugMessage(1, 5.f, FColor::Green, FString::Printf(TEXT("Theta: %f"), Angle));
	}
	*/
	
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

