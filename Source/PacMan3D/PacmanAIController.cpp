#include "PacmanAIController.h"
#include "Kismet/GameplayStatics.h"

void APacmanAIController::BeginPlay()
{
    Super::BeginPlay();

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

    SetFocus(PlayerPawn);
}

void APacmanAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    MoveToActor(PlayerPawn);

}