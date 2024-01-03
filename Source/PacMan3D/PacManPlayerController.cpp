// APacManPlayerController.cpp

#include "PacManPlayerController.h"
#include "TimerManager.h"
#include "Blueprint/UserWidget.h"
#include "AuthManagerActor.h"
#include "Kismet/GameplayStatics.h"

APacManPlayerController::APacManPlayerController()
{
}

void APacManPlayerController::BeginPlay()
{
    Super::BeginPlay();

    HUD = CreateWidget(this, HUDClass);

    if (HUD != nullptr)
    {
        HUD->AddToViewport();
    }
    AuthManager = AAuthManagerActor::GetInstance(); // Usa el Singleton aquí


    if (AuthManager == nullptr)
    {
        AuthManager = GetWorld()->SpawnActor<AAuthManagerActor>(AAuthManagerActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
    }
}

void APacManPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    GetWorldTimerManager().ClearTimer(TimerHandle_CheckAuthentication);
}

void APacManPlayerController::GameHasEnded(AActor* EndGameFocus, bool bIsWinner)
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
        UE_LOG(LogTemp, Error, TEXT("Pantalla final"));
        ResultScreen->AddToViewport();

        // Log detallado para la depuración
        UE_LOG(LogTemp, Warning, TEXT("AuthManager: %s"), *GetNameSafe(AuthManager));

        // Verifica la autenticación después de un retardo
        GetWorldTimerManager().SetTimer(TimerHandle_CheckAuthentication, this, &APacManPlayerController::CheckAuthentication, 1.0f, false);
    }
}

void APacManPlayerController::CheckAuthentication()
{
    if (AuthManager != nullptr && AuthManager->LoginSuccess())
    {
        UE_LOG(LogTemp, Warning, TEXT("AuthManager no es nulo y el inicio de sesión fue exitoso. Token de autenticación actual: %s"), *AuthManager->GetIdToken());

        // Llama a RegisterScore
        AuthManager->RegisterScore(1, 1.2);
    }
    else if (AuthManager != nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("AuthManager no es nulo, pero el inicio de sesión no fue exitoso. Token de autenticación actual: %s"), *AuthManager->GetIdToken());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("AuthManager es nulo."));
    }
}
