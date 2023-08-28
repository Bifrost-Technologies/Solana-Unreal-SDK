/*
 *  UnrealSOLNET
 *  Copyright (c) 2023 Bifrost Inc.
 *  Author: Nathan Martell
 *  Forked from: Stanislav Denisov's UnrealCLR
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 */


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
