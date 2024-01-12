#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Http.h"
#include "AuthManagerActor.generated.h"

UCLASS() class PACMAN3D_API AAuthManagerActor : public AActor
{
    GENERATED_BODY()

public:
    // Sets default values for this actor's properties
    AAuthManagerActor();

    UFUNCTION(BlueprintCallable, Category = "Firebase")
    void RegisterUser(const FString &InUsername, const FString &InEmail, const FString &InPassword);

    UFUNCTION(BlueprintCallable, Category = "Firebase")
    void LoginUser(const FString &InEmail, const FString &InPassword);

    FString IdToken;
    
    UFUNCTION(BlueprintCallable, Category = "User")
    FString GetUsername();

    FString CurrentUserId;

    UFUNCTION(BlueprintCallable, Category = "Firebase")
    bool LoginSuccess() const;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firebase")
    bool bLoginSuccess;

    UFUNCTION(BlueprintCallable, Category = "Score")
    void RegisterScore(int32 HighScore, float GameTime);

    UFUNCTION(BlueprintPure, Category = "Firebase")
    FString GetIdToken() const;

    // Obtener ranking
    UFUNCTION(BlueprintCallable, Category = "Score")
    void GetRanking();

    static AAuthManagerActor *GetInstance();

    UFUNCTION(BlueprintPure, Category = "Singleton")
    static AAuthManagerActor *GetAuthManagerInstanceBP();

    void GetUserIdFromFirestore(const FString &userEmail);

    void HandleGetUserIdResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess);

protected:
    // Called when the game starts or when spawned
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Firebase")
    FString Username;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Firebase")
    FString UserEmail;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Firebase")
    FString UserPassword;

private:
    FString FirebaseSignUpUrl;
    FString FirebaseLoginUrl;
    FString ApiKey;

    void HandleRegisterUserResponse(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSuccess);
    void SaveUserDataToFirestore();
    TSharedPtr<FJsonObject> GetUserJsonFields() const;
    void HandleSaveUserDataResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
    void HandleLoginUserResponse(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSuccess);
    void HandleRegisterScoreResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

    void SaveIdToken(FString id, bool res);

    static AAuthManagerActor *Instance;
};