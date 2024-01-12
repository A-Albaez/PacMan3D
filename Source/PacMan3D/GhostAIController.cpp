// Fill out your copyright notice in the Description page of Project Settings.
#include "GhostAIController.h"
#include "Kismet/GameplayStatics.h"
#include "BehaviorTree/BlackboardComponent.h"

void AGhostAIController::BeginPlay()
{
    Super::BeginPlay();
    FVector Patrol (3150.0, 1209.999987, 327.639551);


    if (AIBehavior != nullptr)
    {
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

        RunBehaviorTree(AIBehavior);
        GetBlackboardComponent()->SetValueAsVector(TEXT("StartLocation"), GetPawn()->GetActorLocation());

        GetBlackboardComponent()->SetValueAsVector(TEXT("PatrolRoute"), Patrol);

    }
    
}

void AGhostAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);    
}