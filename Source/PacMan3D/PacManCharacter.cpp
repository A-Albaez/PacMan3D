// Fill out your copyright notice in the Description page of Project Settings.

#include "PacManCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "InputMappingContext.h"
#include "InputAction.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "MyGameModeBase.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "KillEmAllGameModeBase.h"
#include "Blueprint/UserWidget.h"


DEFINE_LOG_CATEGORY(LogTemplateCharacter);

// Sets default values
APacManCharacter::APacManCharacter()
{
	Score = 0;

	MyCapsuleComponent = GetCapsuleComponent();
	if (MyCapsuleComponent)
	{
		// Configurar la respuesta de colisión con el canal ECC_Pawn para permitir la superposición
		MyCapsuleComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

		UE_LOG(LogTemp, Warning, TEXT("Overlap handling configured in constructor."));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Capsule component is null. Check if it's properly initialized."));
	}

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;			 // Character moves in the direction of input...
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
}

// Called when the game starts or when spawned
void APacManCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Add Input Mapping Context
	if (APlayerController *PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem *Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			checkf(Subsystem, TEXT("Subsystem is null!"));
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
		else
		{
			UE_LOG(LogTemplateCharacter, Error, TEXT("Subsystem is null!"));
		}

		checkf(DefaultMappingContext, TEXT("DefaultMappingContext is null!"));
	}
}

void APacManCharacter::SetupPlayerInputComponent(UInputComponent *PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent *EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APacManCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APacManCharacter::Look);

		//Pause
		// EnhancedInputComponent->BindAction(PauseAction, ETriggerEvent::Triggered, this, &APacManCharacter::Pause);

	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void APacManCharacter::Move(const FInputActionValue &Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void APacManCharacter::Look(const FInputActionValue &Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

// void APacManCharacter::Pause(const FInputActionValue &Value)
// {
// 	UUserWidget* PauseScreen = CreateWidget(this, PauseScreenClass);

// 	if (UGameplayStatics::IsGamePaused(GetWorld()))
// 	{
// 		UGameplayStatics::SetGamePaused(GetWorld(), false);
// 	}
// 	else
// 	{
// 		UGameplayStatics::SetGamePaused(GetWorld(), true);
// 	}
	
// 	if(PauseScreen != nullptr)
// 	{
// 		PauseScreen->AddToViewport();
// 	}
// }

void APacManCharacter::LoseLife()
{
	if (Health > 0)
	{
		Health--;
		ActivateImmunity();
		RespawnInSafeLocation();
	}

	if (Health <= 0)
	{
		// AMyGameModeBase *GameMode = GetWorld()->GetAuthGameMode<AMyGameModeBase>();

		if (GameMode != nullptr)
		{
			GameMode->PawnKilled(this);
		}

		DetachFromControllerPendingDestroy();
		MyCapsuleComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		UGameplayStatics::SpawnSoundAttached(PacmanDies, GetMesh(), TEXT(""));
	}
}

void APacManCharacter::NotifyHit(UPrimitiveComponent *MyComp, AActor *Other, UPrimitiveComponent *OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult &Hit)
{
	TArray<FName> MyTags = this->Tags;
	TArray<FName> OtherTags = Other->Tags;

	if (!MyTags.IsEmpty() && !OtherTags.IsEmpty())
	{
		if (MyTags.Contains("Player") && OtherTags.Contains("Ghost"))
		{
			if (bHasPlusPower)
			{
				Other->Destroy();
				Other = nullptr;
				UGameplayStatics::SpawnSoundAttached(PacmanEatingGhost, GetMesh(), TEXT(""));
			}
			else if (bIsImmune == false)
				LoseLife();
		}
		else if (OtherTags.Contains("coin"))
		{
			UGameplayStatics::SpawnSoundAttached(PacmanEatingCoin, GetMesh(), TEXT(""));

			Score += 10;
			Other->Destroy();
			GameMode = Cast<AKillEmAllGameModeBase>(GetWorld()->GetAuthGameMode());

			if (GameMode)
			{
				GameMode->CheckWinCondition();
			}
		}
		else if (OtherTags.Contains("Plus"))
		{
			UGameplayStatics::SpawnSoundAttached(PacmanEatingPower, GetMesh(), TEXT(""));

			Score += 15;
			Other->Destroy();

			bHasPlusPower = true;
			GetWorldTimerManager().SetTimer(PlusPowerTimerHandle, this, &APacManCharacter::DeactivatePlusPower, 5.0f, false);
			UE_LOG(LogTemp, Warning, TEXT("Poder activado por 5 segundos"));
		}
	}
}

float APacManCharacter::GetHealtPercent() const
{
	return Health / MaxHEalth;
}

int APacManCharacter::GetScore() const
{
	return Score;
}

void APacManCharacter::DeactivatePlusPower()
{
	UE_LOG(LogTemp, Error, TEXT("Poder desactivado"));

	bHasPlusPower = false;
}

void APacManCharacter::ActivateImmunity()
{
	bIsImmune = true;
	GetWorldTimerManager().SetTimer(ImmunityTimerHandle, this, &APacManCharacter::DeactivateImmunity, ImmunityDuration, false);
}

void APacManCharacter::DeactivateImmunity()
{
	bIsImmune = false;
}

void APacManCharacter::RespawnInSafeLocation()
{
	FVector RespawnLocation = FindSafeRespawnLocation();
	SetActorLocation(RespawnLocation);
}

FVector APacManCharacter::FindSafeRespawnLocation()
{
	TArray<FVector> SafeLocations;

	// Safe locations.
	SafeLocations.Add(FVector(3840.0f, 1710.0f, 314.112232f));
	SafeLocations.Add(FVector(4817.0f, 1820.0f, 314.112232f));
	SafeLocations.Add(FVector(4100.0f, 470.0f, 314.112232f));

	//
	if (SafeLocations.Num() > 0)
	{
		int32 RandomIndex = FMath::RandRange(0, SafeLocations.Num() - 1);
		return SafeLocations[RandomIndex];
	}

	// Si no hay ubicaciones seguras, simplemente regresa la posición actual del personaje.
	return GetActorLocation();
}

void APacManCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}