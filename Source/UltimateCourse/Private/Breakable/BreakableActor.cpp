// Fill out your copyright notice in the Descrition page of Project Settings.

#include "Breakable/BreakableActor.h"

#include "Components/CapsuleComponent.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Engine/World.h"
#include "Items/Treasure.h"

// Sets default values
ABreakableActor::ABreakableActor()
{
	PrimaryActorTick.bCanEverTick = false;

	GeometryCollection = CreateDefaultSubobject<UGeometryCollectionComponent>(TEXT("GeometryCollection"));
	SetRootComponent(GeometryCollection);

	GeometryCollection->SetGenerateOverlapEvents(true);
	GeometryCollection->SetCollisionObjectType(ECC_Destructible);
	GeometryCollection->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GeometryCollection->SetNotifyBreaks(true);
	
	GeometryCollection->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	CapsuleCollision = CreateDefaultSubobject<UCapsuleComponent>(FName("Capsule"));
	CapsuleCollision->SetupAttachment(GetRootComponent());
	CapsuleCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	CapsuleCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	bHasBroken = false;
}

void ABreakableActor::BeginPlay()
{
	Super::BeginPlay();
	GeometryCollection->OnChaosBreakEvent.AddDynamic(this, &ABreakableActor::OnBroken);
}

 void ABreakableActor::OnBroken(const FChaosBreakEvent& BreakEvent)
{
	if(bHasBroken)return;
	bHasBroken = true;

	
	if(GetWorld() && ItemDrops.Num() > 0)
	{
		GetWorld()->SpawnActor(GetRandomDrop(), &GetActorTransform());
	}
	SetLifeSpan(3.f);
	CapsuleCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
}

TSubclassOf<AItem> ABreakableActor::GetRandomDrop()
{

	const int32 Ran = FMath::RandRange(0, ItemDrops.Num()-1);
	return ItemDrops[Ran];
}

void ABreakableActor::GetHit_Implementation(const FVector& ImpactPoint, const AActor* Hitter)
{
	
}

void ABreakableActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

