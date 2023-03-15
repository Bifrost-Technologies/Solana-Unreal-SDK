// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Http.h"
#include "SolanaGameTemplateGM.generated.h"


UCLASS()
class SOLANAGAMETEMPLATE_API ASolanaGameTemplateGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	ASolanaGameTemplateGameModeBase();

	UFUNCTION(BlueprintCallable)
	void LoginPlayer(FString username, FString password);

	UFUNCTION(BlueprintCallable)
	void RequestStorePurchase(FString usertoken, FString storeitemID);

	UFUNCTION(BlueprintCallable)
	void RequestRecyclerTransaction(FString usertoken, FString inventoryitemID);

private:

	void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfull);
};
