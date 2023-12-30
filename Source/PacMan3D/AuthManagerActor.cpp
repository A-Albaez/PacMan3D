// Fill out your copyright notice in the Description page of Project Settings.

#include "AuthManagerActor.h"

// Sets default values
AAuthManagerActor::AAuthManagerActor()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    Username = TEXT("");
    UserEmail = TEXT("");
}

// Called when the game starts or when spawned
void AAuthManagerActor::BeginPlay()
{
    Super::BeginPlay();
}

void AAuthManagerActor::RegisterUser(const FString &InUsername, const FString &InEmail, const FString &InPassword)
{
    Username = InUsername;
    UserEmail = InEmail;

    // Construye la URL de la API de autenticación de Firebase para registrar un nuevo usuario
    FString FirebaseRegisterUrl = TEXT("https://identitytoolkit.googleapis.com/v1/accounts:signUp?key=AIzaSyDIx954E-qv68taPdWVEfV5qpXWAaElnSg");

    // Construye el cuerpo de la solicitud JSON
    FString JsonBody = FString::Printf(TEXT("{\"username\":\"%s\",\"email\":\"%s\",\"password\":\"%s\",\"returnSecureToken\":true}"), *InUsername, *InEmail, *InPassword);

    // Configura la solicitud HTTP
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
    HttpRequest->SetVerb("POST");
    HttpRequest->SetURL(FirebaseRegisterUrl);
    HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    HttpRequest->SetContentAsString(JsonBody);
    HttpRequest->OnProcessRequestComplete().BindUObject(this, &AAuthManagerActor::HandleRegisterUserResponse);
    HttpRequest->ProcessRequest();
}

void AAuthManagerActor::HandleRegisterUserResponse(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSuccess)
{
    if (bSuccess && HttpResponse.IsValid())
    {
        // Procesa la respuesta JSON aquí
        FString ResponseJson = HttpResponse->GetContentAsString();
        UE_LOG(LogTemp, Warning, TEXT("Datos recibidos del servidor: %s"), *ResponseJson);

        // Intenta convertir la respuesta JSON a un objeto JSON
        TSharedPtr<FJsonObject> JsonObject;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseJson);

        if (FJsonSerializer::Deserialize(Reader, JsonObject))
        {
            // Verifica si la respuesta contiene información sobre el éxito del registro
            if (JsonObject->HasField("kind") && JsonObject->GetStringField("kind") == "identitytoolkit#SignupNewUserResponse")
            {
                IdToken = JsonObject->GetStringField("idToken");

                // El usuario se registró correctamente
                SaveUserDataToFirestore();
                UE_LOG(LogTemp, Warning, TEXT("Registro de usuario exitoso"));
            }
            else
            {
                // El usuario no se registró correctamente
                FString ErrorDetails = FString::Printf(TEXT("Error en el registro de usuario. Kind: %s, Message: %s"),
                                                      *JsonObject->GetStringField("kind"), *JsonObject->GetStringField("message"));
                UE_LOG(LogTemp, Error, TEXT("%s"), *ErrorDetails);
            }
        }
        else
        {
            // No se pudo analizar la respuesta JSON
            UE_LOG(LogTemp, Error, TEXT("Error al analizar la respuesta JSON del servidor"));
        }
    }
    else
    {
        // Maneja errores
        FString ErrorMessage = (HttpResponse.IsValid()) ? HttpResponse->GetContentAsString() : TEXT("HTTP Request Failed");
        UE_LOG(LogTemp, Error, TEXT("Error en el registro de usuario. %s"), *ErrorMessage);
    }
}

void AAuthManagerActor::SaveUserDataToFirestore()
{
    
    // Construye la URL de tu colección en Firestore
    FString FirestoreUrl = TEXT("https://firestore.googleapis.com/v1/projects/pac-man3d/databases/(default)/documents/users");

     // Construye el cuerpo de la solicitud JSON para guardar datos en Firestore
    FString JsonBody = FString::Printf(TEXT("{\"fields\":{\"username\":{\"stringValue\":\"%s\"},\"email\":{\"stringValue\":\"%s\"}}}"), *Username, *UserEmail);

    // Configura la solicitud HTTP
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
    HttpRequest->SetVerb("POST");
    HttpRequest->SetURL(FirestoreUrl);
    HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    HttpRequest->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *IdToken));
    HttpRequest->SetContentAsString(JsonBody);
    HttpRequest->OnProcessRequestComplete().BindUObject(this, &AAuthManagerActor::HandleSaveUserDataResponse);
    HttpRequest->ProcessRequest();
}

// Maneja la respuesta de la solicitud HTTP
void AAuthManagerActor::HandleSaveUserDataResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    // Verifica si la solicitud fue exitosa y la respuesta es válida
    if (bWasSuccessful && Response.IsValid())
    {
        // Obtiene el código de estado de la respuesta
        int32 StatusCode = Response->GetResponseCode();

        // Verifica si el código de estado es 200 (OK)
        if (StatusCode == 200)
        {
            // Obtiene el contenido de la respuesta como un objeto JSON
            TSharedPtr<FJsonObject> JsonObject;
            TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
            if (FJsonSerializer::Deserialize(JsonReader, JsonObject))
            {
                // Obtiene el nombre del documento creado en Firestore
                FString DocumentName = JsonObject->GetStringField("name");

                // Muestra un mensaje de éxito en el log
                UE_LOG(LogTemp, Log, TEXT("User data saved successfully in Firestore: %s"), *DocumentName);
            }
            else
            {
                // No se pudo analizar la respuesta JSON
                UE_LOG(LogTemp, Error, TEXT("Error al analizar la respuesta JSON de Firestore"));
            }
        }
        else
        {
            // Obtiene el mensaje de error de la respuesta
            FString ErrorMessage = Response->GetContentAsString();

            // Muestra un mensaje de error en el log con el código de estado
            UE_LOG(LogTemp, Error, TEXT("Failed to save user data in Firestore. Status Code: %d, Error Message: %s"), StatusCode, *ErrorMessage);
        }
    }
    else
    {
        // Muestra un mensaje de error en el log
        FString ErrorMessage = (Response.IsValid()) ? Response->GetContentAsString() : TEXT("HTTP Request Failed");
        UE_LOG(LogTemp, Error, TEXT("Failed to send HTTP request to Firestore. %s"), *ErrorMessage);
    }
}
