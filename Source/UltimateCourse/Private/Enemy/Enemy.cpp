// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Enemy.h"

#include "Components/AttributeComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HUD/HealthBarComponent.h"
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
	PrimaryActorTick.bCanEverTick = false;

	EnemyState = EEnemyState::EES_Dead;

}

void AEnemy::Attack()
{
	Super::Attack();
	PlayAttackMontage();
}

void AEnemy::PlayAttackMontage()
{
	Super::PlayAttackMontage();

	if (const TObjectPtr<UAnimInstance> AnimInstance = GetMesh()->GetAnimInstance(); AnimInstance && AttackMontage) {

		AnimInstance->Montage_Play(AttackMontage, 1.3f);

		FName AttackToUse = FName("Attack 1");

		switch (FMath::RandRange(0, 2)) {
		case 0:
			break;
		case 1:
			AttackToUse = FName("Attack 2");
			break;
		case 2:
			AttackToUse = FName("Attack 3");
			break;
		default:
			AttackToUse = FName("Attack 1");
			break;

		}

		AnimInstance->Montage_JumpToSection(AttackToUse);

	}
}

bool AEnemy::IsDead() const
{
	return EnemyState == EEnemyState::EES_Dead;
}

bool AEnemy::CanAttack() const
{
	return !IsOutsideAttackRadius() && !IsAttacking() && !IsDead();
}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	UpdatePatrolPoints();

	CheckCombatTarget();
}

void AEnemy::GetHit_Implementation(const FVector& ImpactPoint)
{
	if(Attributes && IsAlive())
	{
		SetHealthBarVisibility(true);
		PlayHitReactMontage(GetDirectionFromHitPoint(ImpactPoint));
	}
	else Die();


	PlayHitSound(ImpactPoint);
	PlayHitParticle(ImpactPoint);
}

float AEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
	AActor* DamageCauser)
{
	CombatTarget = EventInstigator->GetPawn();
	ChaseTarget();
	HandleDamage(DamageAmount);
	return  DamageAmount;
}

void AEnemy::Destroyed()
{
	Super::Destroyed();
	if(EquippedWeapon)
	{
		EquippedWeapon->Destroy();
	}
}

void AEnemy::HandleDamage(float DamageAmount)
{
	Super::HandleDamage(DamageAmount);
	HealthBarComponent->SetHealthPercent(Attributes->GetHealthPercent());
}

void AEnemy::SetHealthBarVisibility(bool Visibility) const
{
	if(HealthBarComponent)
	{
		if(HealthBarComponent)
			HealthBarComponent->SetVisibility(Visibility);
	}
}

void AEnemy::LoseInterest()
{
	CombatTarget = nullptr;
	SetHealthBarVisibility(false);
}

void AEnemy::StartPatrolling()
{
	EnemyState = EEnemyState::EES_Patrolling;
	GetCharacterMovement()->MaxWalkSpeed = PatrollingSpeed;
	CurrentPatrolTarget = this;
}

void AEnemy::ChaseTarget()
{
	EnemyState = EEnemyState::EES_Chasing;
	GetCharacterMovement()->MaxWalkSpeed = ChasingSpeed;
	MoveToTarget(CombatTarget);
}

bool AEnemy::IsOutsideAttackRadius() const
{
	return !InTargetRange(CombatTarget, AttackRadius);
}

bool AEnemy::IsChasing() const
{
	return EnemyState == EEnemyState::EES_Chasing;
}

bool AEnemy::IsAttacking() const
{
	return EnemyState == EEnemyState::EES_Attacking;
}

void AEnemy::CheckCombatTarget()
{
	if(EnemyState <= EEnemyState::EES_Patrolling )return;
	if(!CombatTarget)return;
	
	if(!InTargetRange(CombatTarget, CombatRadius))
	{
		ClearAttackTimer();
		LoseInterest();
		if(EnemyState == EEnemyState::EES_Engaged)StartPatrolling();
		
	}
	else if(IsOutsideAttackRadius() &&  !IsChasing())
	{
		ClearAttackTimer();
		if(EnemyState == EEnemyState::EES_Engaged) ChaseTarget();
	}
	else if(CanAttack())
	{
		ClearAttackTimer();
		StartAttackTimer();
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
		MoveRequest.SetAcceptanceRadius(60.f);

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
	const bool bShouldChaseTarger = EnemyState < EEnemyState::EES_Chasing &&
		!IsDead() && Pawn->ActorHasTag(FName("SlashCharacter"));
	
	if(!bShouldChaseTarger) return;
	
	GetWorldTimerManager().ClearTimer(PatrolTimer);
	CombatTarget = Pawn;
	ChaseTarget();
}


void AEnemy::StartAttackTimer()
{
	EnemyState = EEnemyState::EES_Attacking;

	const float AttackTime = FMath::RandRange(AttackMin, AttackMax);
	GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemy::Attack, AttackTime);
}

void AEnemy::ClearAttackTimer()
{
	GetWorldTimerManager().ClearTimer(AttackTimer);
}

