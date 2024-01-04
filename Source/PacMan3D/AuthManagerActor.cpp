#include "AuthManagerActor.h"
#include "Json.h"
#include "Serialization/JsonSerializer.h"


AAuthManagerActor* AAuthManagerActor::Instance = nullptr;

// Sets default values
AAuthManagerActor::AAuthManagerActor()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    ApiKey = TEXT("AIzaSyDIx954E-qv68taPdWVEfV5qpXWAaElnSg");
    FirebaseSignUpUrl = TEXT("https://identitytoolkit.googleapis.com/v1/accounts:signUp?key=AIzaSyDIx954E-qv68taPdWVEfV5qpXWAaElnSg");
    FirebaseLoginUrl = TEXT("https://identitytoolkit.googleapis.com/v1/accounts:signInWithPassword?key=AIzaSyDIx954E-qv68taPdWVEfV5qpXWAaElnSg");

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

    // Construye el cuerpo de la solicitud JSON
    FString JsonBody = FString::Printf(TEXT("{\"username\":\"%s\",\"email\":\"%s\",\"password\":\"%s\",\"returnSecureToken\":true}"), *InUsername, *InEmail, *InPassword);

    // Configura la solicitud HTTP
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
    HttpRequest->SetVerb("POST");
    HttpRequest->SetURL(FirebaseSignUpUrl);
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

void AAuthManagerActor::LoginUser(const FString &InEmail, const FString &InPassword)
{
    // Construir el cuerpo JSON de manera segura utilizando la función de escape JSON
    FString JsonBody = FString::Printf(TEXT("{\"email\":\"%s\",\"password\":\"%s\",\"returnSecureToken\":true}"), *InEmail, *InPassword);

    // Log de la solicitud antes de enviarla
    UE_LOG(LogTemp, Warning, TEXT("Solicitud HTTP enviada al servidor: %s"), *JsonBody);

    // Configurar la solicitud HTTP
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
    HttpRequest->SetVerb("POST");
    HttpRequest->SetURL(FirebaseLoginUrl);
    HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    HttpRequest->SetContentAsString(JsonBody);
    HttpRequest->OnProcessRequestComplete().BindUObject(this, &AAuthManagerActor::HandleLoginUserResponse);
    HttpRequest->ProcessRequest();
}

void AAuthManagerActor::HandleLoginUserResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bSuccess)
{
    if (bSuccess && Response.IsValid())
    {
        bLoginSuccess = true;
        // Procesa la respuesta JSON aquí
        FString ResponseJson = Response->GetContentAsString();
        UE_LOG(LogTemp, Warning, TEXT("Respuesta del servidor: %s"), *ResponseJson);

        // Intenta convertir la respuesta JSON a un objeto JSON
        TSharedPtr<FJsonObject> JsonObject;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseJson);

        if (FJsonSerializer::Deserialize(Reader, JsonObject))
        {
            // Verifica si la respuesta contiene información sobre el éxito del inicio de sesión
            if (JsonObject->HasField("kind") && JsonObject->GetStringField("kind") == "identitytoolkit#VerifyPasswordResponse")
            {
                // Extrae el ID del usuario de la respuesta JSON y almacénalo
                IdToken = JsonObject->GetStringField("idToken");

                SaveIdToken(IdToken, bLoginSuccess);
                GetUserIdFromFirestore(UserEmail);
            }
            else
            {
                // El inicio de sesión no fue exitoso
                FString ErrorDetails = FString::Printf(TEXT("Error en el inicio de sesión. Kind: %s, Message: %s"),
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
        bLoginSuccess = false;
        // Maneja errores
        FString ErrorMessage = (Response.IsValid()) ? Response->GetContentAsString() : TEXT("HTTP Request Failed");
        UE_LOG(LogTemp, Error, TEXT("Error en el inicio de sesión. %s"), *ErrorMessage);
    }
}

bool AAuthManagerActor::LoginSuccess() const
{
    return bLoginSuccess;
}

void AAuthManagerActor::RegisterScore(int32 HighScore, float GameTime)
{
    // Verificar si el usuario está autenticado
    if (IdToken.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("El usuario no está autenticado. No se puede registrar la puntuación."));
        return;
    }

    // Construir la URL para registrar la puntuación
    FString ScoreUrl = TEXT("https://firestore.googleapis.com/v1/projects/pac-man3d/databases/(default)/documents/scores");

    // Construir el cuerpo de la solicitud JSON para registrar la puntuación
    FString JsonBody = FString::Printf(TEXT("{\"fields\":{\"user_id\":{\"stringValue\":\"%s\"},\"high_score\":{\"integerValue\":%d},\"game_time\":{\"doubleValue\":%f}}}"), *CurrentUserId, HighScore, GameTime);

    // Configurar la solicitud HTTP
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
    HttpRequest->SetVerb("POST");
    HttpRequest->SetURL(ScoreUrl);
    HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    HttpRequest->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *IdToken));
    HttpRequest->SetContentAsString(JsonBody);
    HttpRequest->OnProcessRequestComplete().BindUObject(this, &AAuthManagerActor::HandleRegisterScoreResponse);
    HttpRequest->ProcessRequest();
}

void AAuthManagerActor::HandleRegisterScoreResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (bWasSuccessful && Response.IsValid())
    {
        // Manejar la respuesta del servidor al registrar la puntuación
        FString ResponseContent = Response->GetContentAsString();
        UE_LOG(LogTemp, Warning, TEXT("Respuesta del servidor al registrar la puntuación: %s"), *ResponseContent);
    }
    else
    {
        // Manejar el error de la solicitud HTTP al registrar la puntuación
        FString ErrorMessage = (Response.IsValid()) ? Response->GetContentAsString() : TEXT("HTTP Request Failed");
        UE_LOG(LogTemp, Error, TEXT("Error al registrar la puntuación. %s"), *ErrorMessage);
    }
}

FString AAuthManagerActor::GetIdToken() const
{
    return IdToken;
}

void AAuthManagerActor::GetRanking() {}

void AAuthManagerActor::SaveIdToken(FString id, bool res)
{
    IdToken = id;
    bLoginSuccess = res;
}

AAuthManagerActor* AAuthManagerActor::GetInstance()
{
    if (Instance == nullptr)
    {
        // Si no hay una instancia existente, crea una nueva
        Instance = NewObject<AAuthManagerActor>();
        Instance->AddToRoot(); // Añadir a la raíz para evitar que se elimine automáticamente
    }

    return Instance;
}

AAuthManagerActor* AAuthManagerActor::GetAuthManagerInstanceBP()
{
    return GetInstance();
}

void AAuthManagerActor::GetUserIdFromFirestore(const FString& userEmail)
{
    // Construye la URL de la colección en Firestore y configura la consulta para buscar por email
    FString FirestoreQueryUrl = FString::Printf(TEXT("https://firestore.googleapis.com/v1/projects/pac-man3d/databases/(default)/documents:runQuery"));

    // Configura el cuerpo de la solicitud JSON con la consulta
       FString QueryBody = TEXT("{\"structuredQuery\": {\"from\": [{\"collectionId\": \"users\"}]}}");

    
    // Configura la solicitud HTTP para la consulta
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
    HttpRequest->SetVerb("POST");
    HttpRequest->SetURL(FirestoreQueryUrl);
    HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    HttpRequest->SetContentAsString(QueryBody);
    HttpRequest->OnProcessRequestComplete().BindUObject(this, &AAuthManagerActor::HandleGetUserIdResponse);
    HttpRequest->ProcessRequest();
}

void AAuthManagerActor::HandleGetUserIdResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    if (bWasSuccessful && Response.IsValid())
    {
        FString ResponseJson = Response->GetContentAsString();
        UE_LOG(LogTemp, Warning, TEXT("Datos recibidos del servidor para obtener todos los usuarios: %s"), *ResponseJson);

        // Intenta convertir la respuesta JSON a una matriz de objetos JSON
        TArray<TSharedPtr<FJsonValue>> JsonArray;
        TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseJson);

        if (FJsonSerializer::Deserialize(Reader, JsonArray))
        {
            // Itera sobre los documentos en la respuesta
            for (const TSharedPtr<FJsonValue>& DocumentValue : JsonArray)
            {
                // Verifica si el valor es un objeto
                if (DocumentValue->Type == EJson::Object)
                {
                    // Obtiene el objeto "document"
                    TSharedPtr<FJsonObject> DocumentObject = DocumentValue->AsObject()->GetObjectField("document");

                    // Verifica si el objeto contiene el campo "fields"
                    if (DocumentObject->HasField("fields"))
                    {
                        TSharedPtr<FJsonObject> FieldsObject = DocumentObject->GetObjectField("fields");

                        // Verifica si el campo "email" existe y su valor coincide con UserEmail
                        if (FieldsObject->HasField("email") && FieldsObject->GetStringField("email") == UserEmail)
                        {
                            // Verifica si el campo "username" existe y obtiene su valor
                            if (FieldsObject->HasField("username"))
                            {
                                FString UserN;
                                if (FieldsObject->GetObjectField("username")->TryGetStringField("stringValue", UserN))
                                {
                                    // Muestra el username en el log
                                    UE_LOG(LogTemp, Log, TEXT("Username del usuario con email %s: %s"), *UserEmail, *UserN);
                                    CurrentUserId = UserN;
                                }
                                else
                                {
                                    // Muestra un mensaje indicando que no se pudo obtener el campo "username"
                                    UE_LOG(LogTemp, Warning, TEXT("No se pudo obtener el campo 'username' para el usuario con email %s"), *UserEmail);
                                }
                            }
                            else
                            {
                                // Muestra un mensaje indicando que no se encontró el campo "username"
                                UE_LOG(LogTemp, Warning, TEXT("El usuario con email %s no tiene un campo 'username'"), *UserEmail);
                            }

                            // Puedes salir del bucle si ya encontraste el usuario
                            break;
                        }
                    }
                }
            }
        }
        else
        {
            // No se pudo analizar la respuesta JSON
            UE_LOG(LogTemp, Error, TEXT("Error al analizar la respuesta JSON del servidor para obtener todos los usuarios"));
        }
    }
    else
    {
        // Maneja errores
        FString ErrorMessage = (Response.IsValid()) ? Response->GetContentAsString() : TEXT("HTTP Request Failed");
        UE_LOG(LogTemp, Error, TEXT("Error al obtener todos los usuarios. %s"), *ErrorMessage);
    }
}
