// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PacManPlayerController.generated.h"

class APacManCharacter;

UCLASS()
class PACMAN3D_API APacManPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	APacManPlayerController();

	virtual void GameHasEnded(class AActor *EndGameFocus = nullptr, bool bIsWinner = false) override;

	void CheckAuthentication();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game")
	int32 Score;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game")
	float Tiempo;

private:
	UPROPERTY(EditAnywhere)
	TSubclassOf<class UUserWidget> LoseScreenClass;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UUserWidget> WinScreenClass;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class UUserWidget> HUDClass;

	UPROPERTY(EditAnywhere)
	float restartDelay = 5;

	FTimerHandle RestartTimer;

	UPROPERTY(EditAnywhere)
	UUserWidget *HUD;

	FTimerHandle TimerHandle_CheckAuthentication;

protected:
	virtual void BeginPlay() override;

    void EndPlay(const EEndPlayReason::Type EndPlayReason);

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Authentication")
    class AAuthManagerActor* AuthManager;
	
	UPROPERTY()
    APacManCharacter* PacmanCharacter;
};
