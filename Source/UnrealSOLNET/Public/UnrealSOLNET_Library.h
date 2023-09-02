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

#include "Kismet/BlueprintFunctionLibrary.h"
#include "UnrealSOLNET_Library.generated.h"

USTRUCT(BlueprintType)
struct UNREALSOLNET_API FManagedFunction {
	GENERATED_BODY()

	public:

	void* Pointer;

	FManagedFunction();
};

UCLASS()
class UNREALSOLNET_API UUnrealSOLNETLibrary : public UBlueprintFunctionLibrary {
	GENERATED_UCLASS_BODY()

	public:

	
	UFUNCTION(BlueprintCallable, BlueprintPure = false, Category = "Solana SDK", meta = (ToolTip = "Finds the Solana SDK method from Unreal SOLNET, optional parameter suppresses errors if the function was not found"))
	static void ExecuteSDKFunction(FString Method, bool Optional, bool& Result, UObject* Object);
};

UCLASS()
class UNREALSOLNET_API UUnrealSOLNETCharacter : public UObject {
	GENERATED_UCLASS_BODY()

	public:

	void* LandedCallback;

	UFUNCTION()
	void Landed(const FHitResult& Hit);
};