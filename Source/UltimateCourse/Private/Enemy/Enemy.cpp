// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Enemy.h"

#include "Components/AttributeComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UltimateCourse/DebugMacros.h"
#include "Kismet/KismetSystemLibrary.h"
#include "HUD/HealthBarComponent.h"
#include "Kismet/GameplayStatics.h"
#include "AIController.h"
#include "Characters/SlashCharacter.h"
#include "Items/Weapons/Weapon.h"
#include "Perception/PawnSensingComponent.h"

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

	HealthBarComponent = CreateDefaultSubobject<UHealthBarComponent>(FName("HealthBar"));
	HealthBarComponent->SetupAttachment(GetRootComponent());
	HealthBarComponent->SetAbsolute(false, false, false);
	
	GetCharacterMovement()->MaxWalkSpeed = 125.f;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationYaw = false;

	PawnSensingComponent = CreateDefaultSubobject<UPawnSensingComponent>(FName("PawnSensing"));
	PawnSensingComponent->SetPeripheralVisionAngle(45.f);
	PawnSensingComponent->SightRadius = 4000;

	

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

	MoveToTarget(CurrentPatrolTarget);

	
	PawnSensingComponent->OnSeePawn.AddDynamic(this, &AEnemy::OnPawnSpotted);
	if(const TObjectPtr<UWorld> World = GetWorld(); World && WeaponClass)
	{
		AWeapon* DefaultWeapon = World->SpawnActor<AWeapon>(WeaponClass);
		DefaultWeapon->Equip(GetMesh(), FName("RightHandSocket"), this, this);
		EquippedWeapon = DefaultWeapon;
		
	}
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
	
	UpdatePatrolPoints();

	CheckCombatTarget();
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
	CombatTarget = EventInstigator->GetPawn();
	if(EnemyState <= EEnemyState::EES_Patrolling)
	{
		EnemyState = EEnemyState::EES_Chasing;
		GetCharacterMovement()->MaxWalkSpeed = 300;
		MoveToTarget(CombatTarget);
	}

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

void AEnemy::Destroyed()
{
	Super::Destroyed();
	if(EquippedWeapon)
	{
		EquippedWeapon->Destroy();
	}
}

void AEnemy::CheckCombatTarget()
{
	if(EnemyState <= EEnemyState::EES_Patrolling )return;
	
	if(CombatTarget)
	{
		if(!InTargetRange(CombatTarget, CombatRadius))
         		{
			CombatTarget = nullptr;
			if(HealthBarComponent)
			{
				HealthBarComponent->SetVisibility(false);
			}
			EnemyState = EEnemyState::EES_Patrolling;
			GetCharacterMovement()->MaxWalkSpeed = 125.f;
			CurrentPatrolTarget = this;


		}else if(!InTargetRange(CombatTarget, AttackRadius) && EnemyState != EEnemyState::EES_Chasing)
		{
			//Outside Attack Range
			
			EnemyState = EEnemyState::EES_Chasing;
			GetCharacterMovement()->MaxWalkSpeed = 300;
			MoveToTarget(CombatTarget);

		}else if(InTargetRange(CombatTarget, AttackRadius) && EnemyState != EEnemyState::EES_Attacking)
		{
			EnemyState = EEnemyState::EES_Attacking;
			// TODO: AttackMontage
		}
	}
}

bool AEnemy::InTargetRange(const AActor* Target, const float Radius) const
{
	return FVector::Distance(GetActorLocation(), Target->GetActorLocation()) < Radius;
}

void AEnemy::UpdatePatrolPoints()
{
	if(!CurrentPatrolTarget)return;
	
	FVector NormalizedVerticalActorPos = GetActorLocation();
	NormalizedVerticalActorPos.Z = CurrentPatrolTarget->GetActorLocation().Z;
	if(FVector::Distance(CurrentPatrolTarget->GetActorLocation(), NormalizedVerticalActorPos) > AcceptableNavPointDistance)return;
	
	
	if(PatrolTargets.Num() > 0)
	{
		PointPosition++;
		if(PointPosition >= PatrolTargets.Num())
			PointPosition = 0;
		CurrentPatrolTarget = PatrolTargets[PointPosition];
	}

	const float RandomTimer = FMath::RandRange(PatrolWaitTime, PatrolWaitTime*2);
	GetWorldTimerManager().SetTimer(PatrolTimer, this, &AEnemy::PatrolTimerFinished, RandomTimer);
}

void AEnemy::MoveToTarget(TObjectPtr<AActor> Target) const
{
	if(AiController && Target)
	{
		FAIMoveRequest MoveRequest;
		MoveRequest.SetGoalActor(Target);
		MoveRequest.SetAcceptanceRadius(15.f);

		FNavPathSharedPtr NavPath;
		AiController->MoveTo(MoveRequest, &NavPath);
		if(NavPath){
			TArray<FNavPathPoint>& PathPoints = NavPath->GetPathPoints();
			for(FNavPathPoint& Point : PathPoints)
			{
				const FVector& Location = Point.Location;
				DrawDebugSphere(GetWorld(), Location, 12.f, 12, FColor::Green, false, 10.f);
			}
		}
	}
}

void AEnemy::PatrolTimerFinished() const
{
	MoveToTarget(CurrentPatrolTarget);
}

void AEnemy::OnPawnSpotted(APawn* Pawn)
{
	if(EnemyState != EEnemyState::EES_Patrolling) return;
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, Pawn->GetHumanReadableName());
	
	if(Pawn->ActorHasTag(FName("SlashCharacter")))
	{
		GetWorldTimerManager().ClearTimer(PatrolTimer);
		EnemyState = EEnemyState::EES_Chasing;
		GetCharacterMovement()->MaxWalkSpeed = 300.f;
		MoveToTarget(Pawn);
		CombatTarget = Pawn;
	}
}

