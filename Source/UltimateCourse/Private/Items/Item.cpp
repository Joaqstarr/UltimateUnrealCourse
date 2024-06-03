// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Item.h"
#include "NiagaraComponent.h"
#include "UltimateCourse/DebugMacros.h"
#include "Components/SphereComponent.h"
#include "Characters/SlashCharacter.h"


AItem::AItem()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	TotalTime = 0;
	Amplitude = 0.2f;
	SinSpeed = 1.f;

	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMeshComponent"));
	RootComponent = ItemMesh;
	

	Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	Sphere->SetupAttachment(GetRootComponent());
	Sphere->SetSphereRadius(300.f);


	NiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(FName("FloatingParticle"));
	NiagaraComponent->SetupAttachment(GetRootComponent());

}

// Called when the game starts or when spawned
void AItem::BeginPlay()
{
	Super::BeginPlay();
	//SetActorLocation(FVector(-150.f, 200.f, 100.f));
	//SetActorRotation(FRotator(120.f, 45.f, 60.f));

	Sphere->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnSphereOverlap);
	Sphere->OnComponentEndOverlap.AddDynamic(this, &AItem::OnSphereEndOverlap);
	if(FloatingParticle)
	{
		NiagaraComponent->SetAsset(FloatingParticle);
	}
	NiagaraComponent->Activate();
}

float AItem::TransformedSin()
{
	return FMath::Sin(TotalTime * SinSpeed) * Amplitude;
}

float AItem::TransformedCos()
{
	return FMath::Cos(TotalTime * SinSpeed) * Amplitude;
}

void AItem::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (ItemState == EItemState::EIS_Equipped) return;
	ASlashCharacter* slashCharacter = Cast<ASlashCharacter>(OtherActor);
	if (slashCharacter) {
		slashCharacter->SetOverlappingItem(this);
	}


}

void AItem::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{

	if (ItemState == EItemState::EIS_Equipped) return;

	ASlashCharacter* slashCharacter = Cast<ASlashCharacter>(OtherActor);
	if (slashCharacter) {
		if (slashCharacter->GetOverlappingItem() == this) {
			slashCharacter->SetOverlappingItem(nullptr);
		}
	}


}



// Called every frame
void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TotalTime += DeltaTime;
	
	if (ItemState == EItemState::EIS_Hovering) {
		AddActorWorldOffset(FVector(0.f, 0.f, TransformedSin()));
	}

}

