// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyGameModeBase.h"
#include "KillEmAllGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class PACMAN3D_API AKillEmAllGameModeBase : public AMyGameModeBase
{
	GENERATED_BODY()

public:
    void BeginPlay();
    void CountInitialCoins();
    virtual void PawnKilled(APawn *Pawnkilled) override;

    UPROPERTY(BlueprintReadOnly, Category = "Game")
    int32 TotalCoins;

	void CheckWinCondition();

protected:
	void EndGame(bool bIsPlayerWinner);
	
};
