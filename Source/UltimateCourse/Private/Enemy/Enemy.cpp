// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Enemy.h"
#include "Components/AttributeComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "HUD/HealthBarComponent.h"
#include "AIController.h"
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

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	UpdatePatrolPoints();

	CheckCombatTarget();
}

float AEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
	AActor* DamageCauser)
{
	CombatTarget = EventInstigator->GetPawn();

	if(!IsOutsideAttackRadius())
	{
		EnemyState = EEnemyState::EES_Attacking;
	}else
	{
		ChaseTarget();
	}
	
	
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

void AEnemy::GetHit_Implementation(const FVector& ImpactPoint, const AActor* Hitter)
{
	if(IsAlive())
	{
		SetHealthBarVisibility(true);
		PlayHitReactMontage(GetDirectionFromHitPoint(Hitter->GetActorLocation()));
	}
	else Die();

	ClearPatrolTimer();
	ClearAttackTimer();
	StopAttackMontage();
	UpdateWeaponCollision(false);
	PlayHitSound(ImpactPoint);
	PlayHitParticle(ImpactPoint);
}

void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	Tags.Add(FName("Enemy"));

	if(PawnSensingComponent)	
		PawnSensingComponent->OnSeePawn.AddDynamic(this, &AEnemy::OnPawnSpotted);


	InitializeEnemy();
}

void AEnemy::Die()
{
	SetHealthBarVisibility(false);

	DisableCapsule();
	SetLifeSpan(DeathLifeSpan);
	ClearAttackTimer();
	PlayDeathMontage();
	
	UpdateWeaponCollision(false);
	PrimaryActorTick.bCanEverTick = false;
	EnemyState = EEnemyState::EES_Dead;

}

void AEnemy::Attack()
{
	EnemyState = EEnemyState::EES_Engaged;
	Super::Attack();
	PlayAttackMontage();
}

bool AEnemy::CanAttack() const
{
	return !IsOutsideAttackRadius() && !IsAttacking() && !IsDead() && !IsEngaged();
}

void AEnemy::ResetAttackState()
{
	EnemyState = EEnemyState::EES_NoState;
	CheckCombatTarget();
}

void AEnemy::HandleDamage(float DamageAmount)
{
	Super::HandleDamage(DamageAmount);
	HealthBarComponent->SetHealthPercent(Attributes->GetHealthPercent());
}

int32 AEnemy::PlayDeathMontage()
{
	const uint8 Selection = Super::PlayDeathMontage();

	const EDeathPose Pose = static_cast<EDeathPose>(Selection);
	if(Pose < EDeathPose::EDP_Max)
		DeathPose = Pose;
	
	return Selection;
}

void AEnemy::InitializeEnemy()
{
	AiController = Cast<AAIController>(GetController());

	SetHealthBarVisibility(false);
	MoveToTarget(CurrentPatrolTarget);
	SpawnDefaultWeapon();
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

void AEnemy::CheckCombatTarget()
{
	if(EnemyState <= EEnemyState::EES_Patrolling )return;
	if(!CombatTarget)
	{
		ClearAttackTimer();
		LoseInterest();
		if(!IsEngaged())StartPatrolling();
		return;
	}

	if(!InTargetRange(CombatTarget, CombatRadius))
	{
		ClearAttackTimer();
		LoseInterest();
		if(!IsEngaged())StartPatrolling();
		
	}
	else if(IsOutsideAttackRadius() &&  !IsChasing())
	{
		ClearAttackTimer();
		if(!IsEngaged()) ChaseTarget();
	}
	else if(CanAttack())
	{
		ClearAttackTimer();
		StartAttackTimer();
	}
}

void AEnemy::SetHealthBarVisibility(bool Visibility) const
{
	if(HealthBarComponent)
	{
		if(HealthBarComponent)
		{
			HealthBarComponent->SetHealthPercent(Attributes->GetHealthPercent());
			HealthBarComponent->SetVisibility(Visibility);
		}
		
	}
}

bool AEnemy::IsDead() const
{
	return EnemyState == EEnemyState::EES_Dead;
}

bool AEnemy::IsEngaged() const
{
	return EnemyState == EEnemyState::EES_Engaged;
}



void AEnemy::SpawnDefaultWeapon()
{
	if(const TObjectPtr<UWorld> World = GetWorld(); World && WeaponClass)
	{
		AWeapon* DefaultWeapon = World->SpawnActor<AWeapon>(WeaponClass);
		DefaultWeapon->Equip(GetMesh(), FName("RightHandSocket"), this, this);
		EquippedWeapon = DefaultWeapon;
		
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
	MoveToTarget(CurrentPatrolTarget);
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





bool AEnemy::InTargetRange(const AActor* Target, const float Radius) const
{
	return FVector::Distance(GetActorLocation(), Target->GetActorLocation()) < Radius;
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

void AEnemy::ClearPatrolTimer()
{
	GetWorldTimerManager().ClearTimer(PatrolTimer);
}

void AEnemy::OnPawnSpotted(APawn* Pawn)
{

	const bool bShouldChaseTarger = EnemyState < EEnemyState::EES_Chasing &&
		!IsDead() && Pawn->ActorHasTag(FName("EngageableTarget"));
	
	if(!bShouldChaseTarger) return;
	
	ClearPatrolTimer();
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

