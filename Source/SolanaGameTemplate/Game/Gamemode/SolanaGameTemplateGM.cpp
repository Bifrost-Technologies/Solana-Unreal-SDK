#include "SolanaGameTemplateGM.h"
#include "UObject/ConstructorHelpers.h"
#include "Json.h"

ASolanaGameTemplateGameModeBase::ASolanaGameTemplateGameModeBase()
{
	//static ConstructorHelpers::FClassFinder<APawn> PlayerCharacter (TEXT("/Game/Action_Male_and_Female/Blueprints/Male/BP_ThirdPersonCharacter_Male"));
	//if (PlayerCharacter.Class != NULL)
	//{
	//	DefaultPawnClass = PlayerCharacter.Class;
	//}
}

 UFUNCTION(BlueprintCallable)
 void ASolanaGameTemplateGameModeBase::LoginPlayer(FString username, FString password)
 {
	 FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
	 TSharedRef<FJsonObject> RequestObj = MakeShared<FJsonObject>();
	 RequestObj->SetStringField("username", username);
	 RequestObj->SetStringField("password", password);

	 FString RequestBody;
	 TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
	 FJsonSerializer::Serialize(RequestObj, Writer);

	 Request->OnProcessRequestComplete().BindUObject(this, &ASolanaGameTemplateGameModeBase::OnResponseReceived);
	 Request->SetURL("http://127.0.0.1:5073/login");
	 Request->SetVerb("POST");
	 Request->SetHeader("Content-Type", "application/json");
	 Request->SetContentAsString(RequestBody);
	 Request->ProcessRequest();
 }

 UFUNCTION(BlueprintCallable)
 void ASolanaGameTemplateGameModeBase::RequestStorePurchase(FString usertoken, FString storeitemID)
 {
	 FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
	 TSharedRef<FJsonObject> RequestObj = MakeShared<FJsonObject>();
	 RequestObj->SetStringField("UserToken", usertoken);
	 RequestObj->SetStringField("StoreItemID", storeitemID);

	 FString RequestBody;
	 TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
	 FJsonSerializer::Serialize(RequestObj, Writer);

	 Request->OnProcessRequestComplete().BindUObject(this, &ASolanaGameTemplateGameModeBase::OnResponseReceived);
	 Request->SetURL("http://127.0.0.1:5073/store/mintpurchase");
	 Request->SetVerb("POST");
	 Request->SetHeader("Content-Type", "application/json");
	 Request->SetContentAsString(RequestBody);
	 Request->ProcessRequest();
 }

 UFUNCTION(BlueprintCallable)
 void ASolanaGameTemplateGameModeBase::RequestRecyclerTransaction(FString usertoken, FString inventoryitemID)
 {
	 FHttpRequestRef Request = FHttpModule::Get().CreateRequest();
	 TSharedRef<FJsonObject> RequestObj = MakeShared<FJsonObject>();
	 RequestObj->SetStringField("UserToken", usertoken);
	 RequestObj->SetStringField("InventoryItemID", inventoryitemID);

	 FString RequestBody;
	 TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
	 FJsonSerializer::Serialize(RequestObj, Writer);

	 Request->OnProcessRequestComplete().BindUObject(this, &ASolanaGameTemplateGameModeBase::OnResponseReceived);
	 Request->SetURL("http://127.0.0.1:5073/store/purchase");
	 Request->SetVerb("POST");
	 Request->SetHeader("Content-Type", "application/json");
	 Request->SetContentAsString(RequestBody);
	 Request->ProcessRequest();
 }

 UFUNCTION(BlueprintCallable)
 void ASolanaGameTemplateGameModeBase::OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfull)
 {
	 TSharedPtr<FJsonObject> ResponseObj;
	 TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());
	 FJsonSerializer::Deserialize(Reader, ResponseObj);

	 UE_LOG(LogTemp, Display, TEXT("Response %s"), *Response->GetContentAsString());
	 UE_LOG(LogTemp, Display, TEXT("Login Key: %s"), *ResponseObj->GetStringField("token"));
 }

