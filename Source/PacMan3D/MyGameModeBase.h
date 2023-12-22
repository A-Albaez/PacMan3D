// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MyGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class PACMAN3D_API AMyGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public: 
	AMyGameModeBase();

	virtual void PawnKilled(APawn* Pawnkilled);
	
};
