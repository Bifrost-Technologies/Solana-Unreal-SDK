#include "SolanaMobileUE5Blueprint.h"
#include "SolanaMobileUE5.h"
#include <Async/Async.h>
#include <Engine.h>

#include <iostream>
#include <string>
#include <sstream>

using namespace std;

// All Java classes are on the path: "SolanaMobileUE5\Source\SolanaMobileUE5\Private\Android\Java\"
#include "Android/Utils/AndroidUtils.h"


FString USolanaMobileUE5Blueprint::HelloWorld(FString MyStr )
{

  return AndroidUtils::CallJavaCode<FString>(
    "com/Plugins/SolanaMobileUE5/HelloWorldClass",     // package (used by com/Plugins/SolanaMobileUE5) and the name of your Java class.
    "HelloWorldOnAndroid",                              //Name of your Java function.
    "",                                                 //Set your own signature instead of an automatic one (Send an empty one if you need an automatic one).
    false,                                              //Determines whether to pass Activity UE5 to Java.
    MyStr                                               //A list of your parameters in the Java function.
  );

  
}

FTypeDispacth USolanaMobileUE5Blueprint::StaticValueDispatch;

void USolanaMobileUE5Blueprint::asyncHelloWorld(const FTypeDispacth& CallBackPlatform, FString MyStr /*= "async Hello World"*/)
{
  USolanaMobileUE5Blueprint::StaticValueDispatch = CallBackPlatform;

  AndroidUtils::CallJavaCode<void>(
    "com/Plugins/SolanaMobileUE5/asyncHelloWorldClass",
    "asyncHelloWorldOnAndroid",
    "",
    true,
    MyStr
  );


}

void USolanaMobileUE5Blueprint::StaticFunctDispatch(const FString& ReturnValue)
{  
  //Lambda function for the dispatcher
  AsyncTask(ENamedThreads::GameThread, [=]() {
    StaticValueDispatch.ExecuteIfBound(ReturnValue);
    });
}


JNI_METHOD void Java_com_Plugins_SolanaMobileUE5_asyncHelloWorldClass_CallBackCppAndroid(JNIEnv* env, jclass clazz, jstring returnStr)
{
  FString result = JavaConvert::FromJavaFString(returnStr);
  UE_LOG(LogTemp, Warning, TEXT("asyncHelloWorld callback caught in C++! - [%s]"), *FString(result)); //Debug log for UE4
  USolanaMobileUE5Blueprint::StaticFunctDispatch(result);// Call Dispatcher
}


void USolanaMobileUE5Blueprint::ShowToastMobile(FString Message, EToastLengthMessage Length)
{

  AndroidUtils::CallJavaCode<void>("com/Plugins/SolanaMobileUE5/NativeUI", "showToast", "", true, Message, (int)Length);

}

