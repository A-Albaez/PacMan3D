// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Http.h"
#include "AuthManagerActor.generated.h"

UCLASS()
class PACMAN3D_API AAuthManagerActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAuthManagerActor();

	UFUNCTION(BlueprintCallable, Category = "Firebase")
    void RegisterUser(const FString& InUsername,const FString& InEmail, const FString& InPassword);

    

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Firebase")
    FString Username;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Firebase")
    FString UserEmail;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Firebase")
    FString UserPassword;
    FString IdToken;

private:
    void HandleRegisterUserResponse(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSuccess);
    void SaveUserDataToFirestore();
    TSharedPtr<FJsonObject> GetUserJsonFields() const;
    void HandleSaveUserDataResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

};
