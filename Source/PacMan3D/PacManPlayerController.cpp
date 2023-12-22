// Fill out your copyright notice in the Description page of Project Settings.


#include "PacManPlayerController.h"
#include "TimerManager.h"
#include "Blueprint/UserWidget.h"

void APacManPlayerController::GameHasEnded(AActor *EndGameFocus, bool bIsWinner)
{
    Super::GameHasEnded(EndGameFocus, bIsWinner);

    HUD->RemoveFromViewport();

    UUserWidget* ResultScreen = nullptr;

    if (bIsWinner)
    {
        // Crear la pantalla de victoria
        ResultScreen = CreateWidget(this, WinScreenClass);
    }
    else
    {
        // Crear la pantalla de derrota
        ResultScreen = CreateWidget(this, LoseScreenClass);
    }

    if (ResultScreen != nullptr)
    {
        ResultScreen->AddToViewport();
        GetWorldTimerManager().SetTimer(RestartTimer, this, &APlayerController::RestartLevel, restartDelay);

    }    
}

void APacManPlayerController::BeginPlay() 
{
    Super::BeginPlay();

    HUD = CreateWidget(this, HUDClass);

    if(HUD != nullptr)
    {
        HUD->AddToViewport();
    }
}
