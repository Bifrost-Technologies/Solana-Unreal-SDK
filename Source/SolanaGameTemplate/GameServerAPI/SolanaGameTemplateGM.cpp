
#include "SolanaGameTemplateGM.h"
#include "UObject/ConstructorHelpers.h"
#include <Json.h>

 USolanaGameTemplateGameModeBase::USolanaGameTemplateGameModeBase()
{
	//static ConstructorHelpers::FClassFinder<APawn> PlayerCharacter (TEXT("/Game/Action_Male_and_Female/Blueprints/Male/BP_ThirdPersonCharacter_Male"));
	//if (PlayerCharacter.Class != NULL)
	//{
	//	DefaultPawnClass = PlayerCharacter.Class;
	//}
}


void USolanaGameTemplateGameModeBase::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfull)
{
	TSharedPtr<FJsonObject> ResponseObj;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	FJsonSerializer::Deserialize(Reader, ResponseObj);

	UE_LOG(LogTemp, Display, TEXT("Response %s"), *Response->GetContentAsString());
	UE_LOG(LogTemp, Display, TEXT("Login Key: %s"), *ResponseObj->GetStringField("token"));
}

UFUNCTION(BlueprintCallable)
void USolanaGameTemplateGameModeBase::LoginPlayer(FString username, FString password)
{
	FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
	TSharedRef<FJsonObject> RequestObj = MakeShared<FJsonObject>();
	RequestObj->SetStringField("username", username);
	RequestObj->SetStringField("password", password);

	FString RequestBody;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
	FJsonSerializer::Serialize(RequestObj, Writer);

	Request->OnProcessRequestComplete().BindUObject(this, &USolanaGameTemplateGameModeBase::OnResponseReceived);
	Request->SetURL("https://portal.vivaion.com/login");
	Request->SetVerb("POST");
	Request->SetHeader("Content-Type", "application/json");
	Request->SetContentAsString(RequestBody);
	Request->ProcessRequest();
}