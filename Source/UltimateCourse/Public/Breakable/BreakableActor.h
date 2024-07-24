// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interfaces/IHitInterface.h"
#include "Chaos/ChaosGameplayEventDispatcher.h"
#include "BreakableActor.generated.h"

class UCapsuleComponent;
class UGeometryCollectionComponent;
class AItem;

UCLASS()
class ULTIMATECOURSE_API ABreakableActor : public AActor, public IIHitInterface
{
	GENERATED_BODY()
	
public:	
	ABreakableActor();
	virtual void Tick(float DeltaTime) override;
	virtual void GetHit_Implementation(const FVector& ImpactPoint, const AActor* Hitter) override;


protected:
	virtual void BeginPlay() override;
	UPROPERTY(EditAnywhere)
	TObjectPtr<UCapsuleComponent> CapsuleCollision;
private:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UGeometryCollectionComponent> GeometryCollection;
	UFUNCTION()
	void OnBroken(const FChaosBreakEvent& BreakEvent);
	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<AItem>> ItemDrops;
	bool bHasBroken;

	TSubclassOf<AItem> GetRandomDrop();
	
	
};
