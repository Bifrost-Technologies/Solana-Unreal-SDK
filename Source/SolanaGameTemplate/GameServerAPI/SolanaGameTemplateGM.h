// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include <Http.h>
#include "SolanaGameTemplateGM.generated.h"

/**
 * 
 */
 // Event that will be the 'Completed' exec wire in the blueprint node along with all parameters as output pins.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHttpRequestCompleted, const FString&, MOTD, bool, bSuccess);


UCLASS()
class USolanaGameTemplateGameModeBase : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	USolanaGameTemplateGameModeBase();


	UFUNCTION(BlueprintCallable)
		void LoginPlayer(FString username, FString password);

private:
	void OnResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bConnectedSuccessfull);
};
