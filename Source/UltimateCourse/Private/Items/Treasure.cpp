// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Treasure.h"
#include "Kismet/GameplayStatics.h"
#include "Characters/SlashCharacter.h"

ATreasure::ATreasure()
{
	ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ATreasure::BeginPlay()
{
	Super::BeginPlay();
	SetActorLocation(GetActorLocation() + SpawnOffset);

}

void ATreasure::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(ASlashCharacter* SlashCharacter = Cast<ASlashCharacter>(OtherActor))
	{
		if(CollectSound)
		{
			UGameplayStatics::PlaySoundAtLocation(this, CollectSound, GetActorLocation(), FRotator::ZeroRotator);
		}
		Destroy();
	}
	
}
