#pragma once

#include <Kismet/BlueprintFunctionLibrary.h>
#include <Runtime/Launch/Resources/Version.h>
#include <Async/Async.h>
#include <Engine.h>

#include "NativeUI/Enums/ToastLengthMessage.h"

#include "SolanaMobileUE5Blueprint.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FTypeDispacth, const FString&, ReturnValue);   

UCLASS()
class SolanaMobileUE5_API USolanaMobileUE5Blueprint : public UBlueprintFunctionLibrary
{
  GENERATED_BODY()
public:
  USolanaMobileUE5Blueprint(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {};

  static FTypeDispacth StaticValueDispatch;
  static void StaticFunctDispatch(const FString& ReturnValue);

  // - Solana Mobile Wrapper -
  // 
  // Come back soon!
  // 
  //Test Android JNI methods
  UFUNCTION(BlueprintCallable, Category = "SolanaMobileUE5 Category")
  static FString HelloWorld(FString MyStr = "Hello World");

  UFUNCTION(BlueprintCallable, Category = "SolanaMobileUE5 Category")
  static void asyncHelloWorld(const FTypeDispacth& CallBackPlatform, FString MyStr = "async Hello World");

  UFUNCTION(BlueprintCallable, Category = "SolanaMobileUE5 Category")
  static void ShowToastMobile(FString Message, EToastLengthMessage Length);
  //
};
