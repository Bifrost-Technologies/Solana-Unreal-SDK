#pragma once

#include <Android/AndroidJNI.h>
#include <Android/AndroidApplication.h>
#include <Android/AndroidJava.h>

#include <iostream>
#include <string>

using namespace std;

class JavaConvert
{
public:
	
	static jlong GetJavaLong(long l);

	static jstring GetJavaString(const FString& string);

	static jstring GetJavaString(const string& str);

	static jstring GetJavaString(const char* str);

	static FString FromJavaFString(jstring javaString);

	static string FromJavaString(jstring javaString);

	static jobjectArray ConvertToJStringArray(const TArray<FString>& stringArray);

	static jbooleanArray ConvertToJBooleanArray(const TArray<bool>& boolArray);

	static jintArray ConvertToJIntArray(const TArray<int>& intArray);

	static jbyteArray ConvertToJByteArray(const TArray<uint8>& byteArray);

	static jlongArray ConvertToJLongArray(const TArray<long>& longArray);
	
	static jfloatArray ConvertToJFloatArray(const TArray<float>& floatArray);

	static TArray<uint8> ConvertToByteArray(jbyteArray javaArray);
	
	static TArray<float> ConvertToFloatArray(jfloatArray javaArray);
	
	static TArray<int> ConvertToIntArray(jintArray javaArray);

	static TArray<long> ConvertToLongArray(jlongArray javaArray);

	static TArray<FString> ConvertToStringArray(jobjectArray javaStringArray);

};
