// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Weapons/Weapon.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "Interfaces/IHitInterface.h"
#include "NiagaraComponent.h"
#include "Animation/AnimMontage.h"
#include "UltimateCourse/DebugMacros.h"

AWeapon::AWeapon()
{
	WeaponBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Weapon Box"));
	WeaponBox->SetupAttachment(GetRootComponent());
	WeaponBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	WeaponBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

	TraceStart = CreateDefaultSubobject<USceneComponent>(TEXT("StartSweep"));
	TraceStart->SetupAttachment(GetRootComponent());
	
	TraceEnd = CreateDefaultSubobject<USceneComponent>(TEXT("EndSweep"));
	TraceEnd->SetupAttachment(GetRootComponent());

}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	DisableAttackCollision();
	WeaponBox->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnBoxOverlap);
	DefaultRotation = GetActorRotation();
}

void AWeapon::Equip(TObjectPtr<USceneComponent> parent, FName socketName, TObjectPtr<AActor> newOwner, TObjectPtr<APawn> newInstigator)
{
	AttachMeshToSocket(parent, socketName);
	ItemState = EItemState::EIS_Equipped;

	SetOwner(newOwner);
	SetInstigator(newInstigator);
	
	if (EquipSound) {
		UGameplayStatics::PlaySoundAtLocation(this, EquipSound, GetActorLocation());
	}
	if (Sphere) {
		Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	if(NiagaraComponent)
	{
		if(NiagaraComponent->IsActive())
		{
			NiagaraComponent->Deactivate();
		}
	}
}

void AWeapon::DropWeapon(const FVector& Location)
{
	ItemState = EItemState::EIS_Hovering;
	SetOwner(nullptr);
	SetInstigator(nullptr);
	ItemMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	//SetActorLocation(Location);
	if (Sphere) {
		Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	if(NiagaraComponent)
	{
		if(!NiagaraComponent->IsActive())
		{
			NiagaraComponent->Activate();
		}
	}
	SetActorRotation(DefaultRotation);
}

void AWeapon::AttachMeshToSocket(TObjectPtr<USceneComponent> parent, const FName& socketName) const
{
	FAttachmentTransformRules TransformRules(EAttachmentRule::SnapToTarget, true);
	ItemMesh->AttachToComponent(parent, TransformRules, socketName);
}

void AWeapon::UpdateWeaponCollision(bool collisionTo)
{
	if(collisionTo)
	{
		EnableAttackCollision();
	}else
	{
		DisableAttackCollision();
	}
}

TObjectPtr<UAnimMontage> AWeapon::GetAttackMontage() const
{
	return AttackMontage;
}

TObjectPtr<UAnimMontage> AWeapon::GetEquipMontage() const
{
	return EquipMontage;
}

EWeaponType AWeapon::GetWeaponType() const
{
	return WeaponClass;
}

void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

}

void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	Super::OnSphereEndOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);
}

void AWeapon::OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	FHitResult HitResult;

	IgnoreActors.AddUnique(this);
	
	UKismetSystemLibrary::BoxTraceSingle(this, TraceStart->GetComponentLocation(), TraceEnd->GetComponentLocation(),
		FVector(2.5f, 2.5f, 2.5f), FRotator(0, 0, 0),TraceTypeQuery1,
		false, IgnoreActors, EDrawDebugTrace::None, HitResult, true);
	//DRAW_SPHERE(HitResult.ImpactPoint);
	if(HitResult.GetActor())
	{
		if(IIHitInterface* Hittable = Cast<IIHitInterface>(HitResult.GetActor()); Hittable != nullptr)
		{
			UGameplayStatics::ApplyDamage(HitResult.GetActor(), Damage, GetInstigator()->GetController(), this, UDamageType::StaticClass());
			Hittable->Execute_GetHit(HitResult.GetActor(), HitResult.ImpactPoint);
		}
		IgnoreActors.AddUnique(HitResult.GetActor());
		CreateFields(HitResult.ImpactPoint);
	}
}

void AWeapon::EnableAttackCollision() const
{
	WeaponBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

}

void AWeapon::DisableAttackCollision() 
{
	WeaponBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	IgnoreActors.Empty();
}

