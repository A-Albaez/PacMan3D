// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "PacManCharacter.generated.h"


class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class AKillEmAllGameModeBase;


DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS()
class PACMAN3D_API APacManCharacter : public ACharacter
{
	GENERATED_BODY()
	
	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enhanced Input", meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enhanced Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enhanced Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enhanced Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* PauseAction;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	APacManCharacter();

    UFUNCTION(BlueprintCallable, Category = "Gameplay")
    void LoseLife();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay")
	float Health = 3;

	float MaxHEalth = 3;

	UFUNCTION()
	void NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
	int32 Score;

	UFUNCTION(BlueprintPure)
	float GetHealtPercent() const;

	UFUNCTION(BlueprintPure)
	int GetScore() const;

	float ImmunityDuration = 3;
	bool bHasPlusPower = false;
	bool bIsImmune = false;
	FTimerHandle PlusPowerTimerHandle;
	FTimerHandle ImmunityTimerHandle;

	UCapsuleComponent* MyCapsuleComponent;

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	// void Pause(const FInputActionValue& Value);

	void DeactivatePlusPower();
	void ActivateImmunity();
	void DeactivateImmunity();
	void RespawnInSafeLocation();
	FVector FindSafeRespawnLocation();

    UPROPERTY(BlueprintReadOnly, Category = "Game")
    AKillEmAllGameModeBase* GameMode;

private:
	UPROPERTY(EditAnywhere, Category = "Sounds")
	USoundBase* PacmanEatingCoin;

	UPROPERTY(EditAnywhere, Category = "Sounds")
	USoundBase* PacmanEatingPower;

	UPROPERTY(EditAnywhere, Category = "Sounds")
	USoundBase* PacmanEatingGhost;

	UPROPERTY(EditAnywhere, Category = "Sounds")
	USoundBase* PacmanDies;

	// UPROPERTY(EditAnywhere)
	// TSubclassOf<class UUserWidget> PauseScreenClass;

};
