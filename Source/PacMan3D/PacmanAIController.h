// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "PacmanAIController.generated.h"

/**
 * 
 */
UCLASS()
class PACMAN3D_API APacmanAIController : public AAIController
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;


public:
	virtual void Tick(float DeltaTime) override;
};
