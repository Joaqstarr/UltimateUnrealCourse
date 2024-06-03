// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/Item.h"
#include "Treasure.generated.h"

class USoundBase;
/**
 * 
 */
UCLASS()
class ULTIMATECOURSE_API ATreasure : public AItem
{
	GENERATED_BODY()
public:
	ATreasure();
protected:
	virtual void BeginPlay() override;
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
private:
	UPROPERTY(EditAnywhere, Category="Sounds")
	TObjectPtr<USoundBase> CollectSound;
	UPROPERTY(EditAnywhere, Category = "Treasure Properties")
	int32 Gold;
	UPROPERTY(EditAnywhere, Category="Treasure Properties")
	FVector SpawnOffset;
};
