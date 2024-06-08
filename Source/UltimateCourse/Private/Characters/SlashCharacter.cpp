// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/SlashCharacter.h"
#include "EnhancedInputComponent.h"
#include "InputAction.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GroomComponent.h"
#include "Items/Weapons/Weapon.h"
#include "Animation/AnimInstance.h"

// Sets default values
ASlashCharacter::ASlashCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("Camera Boom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 300.0f;
	CameraBoom->bUsePawnControlRotation = true;

	ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewCamera"));
	ViewCamera->SetupAttachment(CameraBoom);

	Hair = CreateDefaultSubobject<UGroomComponent>(TEXT("Hair"));
	Hair->SetupAttachment(GetMesh());
	Hair->AttachmentName = FString("head");

	Eyebrows = CreateDefaultSubobject<UGroomComponent>(TEXT("Eyebrows"));
	Eyebrows->SetupAttachment(GetMesh());
	Eyebrows->AttachmentName = FString("head");

	GetCharacterMovement()->bOrientRotationToMovement = true;

}

// Called when the game starts or when spawned
void ASlashCharacter::BeginPlay()
{
	Super::BeginPlay();

	Tags.Add(FName("SlashCharacter"));
	
	if (TObjectPtr <APlayerController> PlayerController = Cast<APlayerController>(GetController())) {
		if (TObjectPtr<UEnhancedInputLocalPlayerSubsystem> InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer())) {

			FModifyContextOptions Options;
			Options.bIgnoreAllPressedKeysUntilRelease = true;
			Options.bForceImmediately = false;
			InputSubsystem->AddMappingContext(SlashContext, 0, Options);
		}
	}
}


// Called every frame
void ASlashCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ASlashCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (TObjectPtr<UEnhancedInputComponent> EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {
		EnhancedInputComponent->BindAction(MovementAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Move);
		EnhancedInputComponent->BindAction(LookingAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Look);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ASlashCharacter::Jumping);
		EnhancedInputComponent->BindAction(EquipAction, ETriggerEvent::Started, this, &ASlashCharacter::Equip);
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &ASlashCharacter::Attack);
	}

}

void ASlashCharacter::Move(const FInputActionValue& Value)
{
	if (ActionState != EActionState::EAS_Unoccupied)return;

	if (!GetController())return;
	const FVector CurVal = Value.Get<FVector>();
	  
	const FRotator YawRotation(0.f, GetControlRotation().Yaw, 0.f);

	const FVector Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector Right = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);


	AddMovementInput(Forward, CurVal.Y);
	AddMovementInput(Right, CurVal.X);

}

void ASlashCharacter::Look(const FInputActionValue& Value)
{
	if (!GetController())return;
	const FVector CurVal = Value.Get<FVector>();

	AddControllerYawInput(CurVal.X);
	AddControllerPitchInput(CurVal.Y);

}

void ASlashCharacter::Jumping(const FInputActionValue& Value)
{
	if (ActionState == EActionState::EAS_Attacking)return;

	if (!GetController())return;
	Jump();
	

}

void ASlashCharacter::Equip(const FInputActionValue& Value)
{
	if (ActionState == EActionState::EAS_Attacking)return;

	if (OverlappingItem) {
		if (TObjectPtr<AWeapon> asWeapon = Cast<AWeapon>(OverlappingItem)) {
			asWeapon->Equip(GetMesh(), FName("RightHandSocket"), this, this);
			OverlappingItem = nullptr;
			if(EquippedWeapon)
			{
				DropWeapon();
			}

			switch(asWeapon->GetWeaponType())
			{
			case EWeaponType::EWT_OneHanded:
				CharacterState = ECharacterState::ECS_EquippedOneHandedWeapon;
				break;
			case EWeaponType::EWT_TwoHanded:
				CharacterState = ECharacterState::ECS_EquippedTwoHandedWeapon;
				break;
			default:
				CharacterState = ECharacterState::ECS_EquippedOneHandedWeapon;
				break;
			}
			
			EquippedWeapon = asWeapon;
			
			AttackMontage = EquippedWeapon->GetAttackMontage();
			EquipMontage = EquippedWeapon->GetEquipMontage();

		}
	}
	else {
		if (CanDisarm()) {

			PlayEquipMontage(FName("Unequip"));
			CharacterState = ECharacterState::ECS_Unequipped;
			ActionState = EActionState::EAS_EquippingWeapon;
		}else
		if (CanArm()) {
			PlayEquipMontage(FName("Equip"));
			CharacterState = ECharacterState::ECS_EquippedOneHandedWeapon;
			ActionState = EActionState::EAS_EquippingWeapon;
		}
	}

}
void ASlashCharacter::PlayEquipMontage(const FName& Section) const
{
	TObjectPtr<UAnimInstance> AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && EquipMontage) {
		AnimInstance->Montage_Play(EquipMontage);
		AnimInstance->Montage_JumpToSection(Section);
	}

}

void ASlashCharacter::Disarm() const
{
	if (EquippedWeapon) {
		EquippedWeapon->AttachMeshToSocket(GetMesh(), FName("SpineSocket"));
	}

}

void ASlashCharacter::Arm()
{
	if (EquippedWeapon) {
		EquippedWeapon->AttachMeshToSocket(GetMesh(), FName("RightHandSocket"));
	}
}

void ASlashCharacter::ResetEquipState()
{
	ActionState = EActionState::EAS_Unoccupied;
}



void ASlashCharacter::Attack()
{
	if (!CanAttack())return;


	PlayAttackMontage();
	ActionState = EActionState::EAS_Attacking;
	

}

void ASlashCharacter::ResetAttackState()
{
	ActionState = EActionState::EAS_Unoccupied;
}

void ASlashCharacter::GetHit_Implementation(const FVector& ImpactPoint)
{
	Super::GetHit_Implementation(ImpactPoint);
}

void ASlashCharacter::PlayAttackMontage()
{
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



bool ASlashCharacter::CanAttack() const
{
	return CharacterState != ECharacterState::ECS_Unequipped && ActionState == EActionState::EAS_Unoccupied;
}

bool ASlashCharacter::CanDisarm() const
{
	return CharacterState != ECharacterState::ECS_Unequipped && EquippedWeapon;
}

bool ASlashCharacter::CanArm() const
{
	return CharacterState == ECharacterState::ECS_Unequipped && ActionState == EActionState::EAS_Unoccupied && EquippedWeapon;
}

void ASlashCharacter::DropWeapon()
{
	EquippedWeapon->DropWeapon(GetActorLocation());
	EquippedWeapon = nullptr;
}
