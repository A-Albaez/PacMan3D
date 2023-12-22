#include "KillEmAllGameModeBase.h"
#include "EngineUtils.h"

void AKillEmAllGameModeBase::BeginPlay()
{
    Super::BeginPlay();

    CountInitialCoins();
}

void AKillEmAllGameModeBase::CountInitialCoins()
{
    TotalCoins = 0;

    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor->ActorHasTag("coin"))
        {
            TotalCoins++;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Monedas iniciales: %d"), TotalCoins);
}

void AKillEmAllGameModeBase::PawnKilled(APawn* Pawnkilled)
{
    Super::PawnKilled(Pawnkilled);

    APlayerController* PlayerController = Cast<APlayerController>(Pawnkilled->GetController());

    if (PlayerController != nullptr)
    {
        //PlayerController->GameHasEnded(nullptr, false);
        EndGame(false);
    }
}

void AKillEmAllGameModeBase::EndGame(bool bIsPlayerWinner)
{
    for (AController* Controller : TActorRange<AController>(GetWorld()))
    {
        bool bIsWinner = Controller->IsPlayerController() == bIsPlayerWinner;
        Controller->GameHasEnded(Controller->GetPawn(), bIsWinner);
        
    }

}

void AKillEmAllGameModeBase::CheckWinCondition()
{
    int32 RemainingCoins = 0;

    for (TActorIterator<AActor> ActorItr(GetWorld()); ActorItr; ++ActorItr)
    {
        AActor* Actor = *ActorItr;
        if (Actor->ActorHasTag("coin"))
        {
            RemainingCoins++;
        }
    }

    if (RemainingCoins == 0)
    {
        // Nuevo: Mostrar la cantidad restante en pantalla
        UE_LOG(LogTemp, Warning, TEXT("¡Has recogido todas las monedas!"));

        // Llamar al método que maneja la victoria
        EndGame(true);
    }
}