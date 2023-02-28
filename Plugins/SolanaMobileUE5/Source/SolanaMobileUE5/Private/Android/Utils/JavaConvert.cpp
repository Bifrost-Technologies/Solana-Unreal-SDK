#include "JavaConvert.h"

jlong JavaConvert::GetJavaLong(long l)
{
	jlong jl = static_cast<jlong>(l);
  return jl;
}

jstring JavaConvert::GetJavaString(const FString& string)
{
	return GetJavaString(TCHAR_TO_UTF8(*string));
}

jstring JavaConvert::GetJavaString(const string& str)
{
	return GetJavaString(str.c_str());
}

jstring JavaConvert::GetJavaString(const char* str)
{
	JNIEnv* JEnv = AndroidJavaEnv::GetJavaEnv();
	return JEnv->NewStringUTF(str);
}

FString JavaConvert::FromJavaFString(jstring javaString)
{
	JNIEnv* Env = AndroidJavaEnv::GetJavaEnv();
	const char* UTFString = Env->GetStringUTFChars(javaString, nullptr);
	FString Result(UTF8_TO_TCHAR(UTFString));
	Env->ReleaseStringUTFChars(javaString, UTFString);
	Env->DeleteLocalRef(javaString);

	return Result;
}

string JavaConvert::FromJavaString(jstring javaString)
{
	JNIEnv* Env = AndroidJavaEnv::GetJavaEnv();
	const char* UTFString = Env->GetStringUTFChars(javaString, nullptr);
	FString Result(UTF8_TO_TCHAR(UTFString));
	Env->ReleaseStringUTFChars(javaString, UTFString);
	Env->DeleteLocalRef(javaString);

	return string(TCHAR_TO_UTF8(*Result));
}

jobjectArray JavaConvert::ConvertToJStringArray(const TArray<FString>& stringArray)
{
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	jobjectArray javaStringArray = (jobjectArray)Env->NewObjectArray(stringArray.Num(), FJavaWrapper::JavaStringClass, nullptr);
	for (int i = 0; i < stringArray.Num(); i++) {
		Env->SetObjectArrayElement(javaStringArray, i, JavaConvert::GetJavaString(stringArray[i]));
	}
	return javaStringArray;
}

jbooleanArray JavaConvert::ConvertToJBooleanArray(const TArray<bool>& boolArray)
{
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	jbooleanArray javaBooleanArray = (jbooleanArray)Env->NewBooleanArray(boolArray.Num());
	jboolean* javaBooleanArrayPtr = Env->GetBooleanArrayElements(javaBooleanArray, 0);
	for (int i = 0; i < boolArray.Num(); i++) {
		javaBooleanArrayPtr[i] = boolArray[i];
	}
	return javaBooleanArray;
}

jintArray JavaConvert::ConvertToJIntArray(const TArray<int>& intArray)
{
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	jintArray javaIntArray = (jintArray)Env->NewIntArray(intArray.Num());
	jint* javaIntArrayPtr = (jint*)malloc(intArray.Num() * sizeof(jint));
	for (int i = 0; i < intArray.Num(); i++) {
		javaIntArrayPtr[i] = (jint)intArray[i];
	}
	Env->SetIntArrayRegion(javaIntArray, 0, intArray.Num(), javaIntArrayPtr);
	free(javaIntArrayPtr);
	return javaIntArray;
}

jbyteArray JavaConvert::ConvertToJByteArray(const TArray<uint8>& byteArray)
{
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	jbyteArray javaByteArray = (jbyteArray)Env->NewByteArray(byteArray.Num());
	jbyte* javaByteArrayPtr = (jbyte*)malloc(byteArray.Num() * sizeof(jbyte));
	for (int i = 0; i < byteArray.Num(); i++) {
		javaByteArrayPtr[i] = byteArray[i];
	}
	Env->SetByteArrayRegion(javaByteArray, 0, byteArray.Num(), javaByteArrayPtr);
	free(javaByteArrayPtr);
	return javaByteArray;
}

jlongArray JavaConvert::ConvertToJLongArray(const TArray<long>& longArray)
{
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	jlongArray javaLongArray = (jlongArray)Env->NewLongArray(longArray.Num());
	jlong* javaLongArrayPtr = (jlong*)malloc(longArray.Num() * sizeof(jlong));
	for (int i = 0; i < longArray.Num(); i++) {
		javaLongArrayPtr[i] = longArray[i];
	}
	Env->SetLongArrayRegion(javaLongArray, 0, longArray.Num(), javaLongArrayPtr);
	free(javaLongArrayPtr);
	return javaLongArray;
}

jfloatArray JavaConvert::ConvertToJFloatArray(const TArray<float>& floatArray)
{
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	jfloatArray javaFloatArray = (jfloatArray)Env->NewFloatArray(floatArray.Num());
	jfloat* javaFloatArrayPtr = (jfloat*)malloc(floatArray.Num() * sizeof(jfloat));
	for (int i = 0; i < floatArray.Num(); i++) {
		javaFloatArrayPtr[i] = floatArray[i];
	}
	Env->SetFloatArrayRegion(javaFloatArray, 0, floatArray.Num(), javaFloatArrayPtr);
	free(javaFloatArrayPtr);
	return javaFloatArray;
}

TArray<uint8> JavaConvert::ConvertToByteArray(jbyteArray javaArray)
{
	TArray<uint8> byteArray;
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	jbyte* javaByte = Env->GetByteArrayElements(javaArray, 0);
	int length = Env->GetArrayLength(javaArray);
	for (int i = 0; i < length; i++) {
		byteArray.Add(javaByte[i]);
	}
	return byteArray;
}


TArray<float> JavaConvert::ConvertToFloatArray(jfloatArray javaArray)
{
	TArray<float> floatArray;
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	jfloat* javaFloat = Env->GetFloatArrayElements(javaArray, 0);
	int length = Env->GetArrayLength(javaArray);
	for (int i = 0; i < length; i++)
	{
		floatArray.Add((float)javaFloat[i]);
	}
	return floatArray;
}

TArray<int> JavaConvert::ConvertToIntArray(jintArray javaArray)
{
	TArray<int> numArray;
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	jint* javaNum = Env->GetIntArrayElements(javaArray, 0);
	int length = Env->GetArrayLength(javaArray);
	for (int i = 0; i < length; i++)
	{
		numArray.Add((int)javaNum[i]);
	}
	return numArray;
}

TArray<long> JavaConvert::ConvertToLongArray(jlongArray javaArray)
{
	TArray<long> longArray;
	JNIEnv* Env = FAndroidApplication::GetJavaEnv();
	jlong* javaLong = Env->GetLongArrayElements(javaArray, 0);
	int length = Env->GetArrayLength(javaArray);
	for (int i = 0; i < length; i++)
	{
		longArray.Add((long)javaLong[i]);
	}
	return longArray;
}

TArray<FString> JavaConvert::ConvertToStringArray(jobjectArray javaStringArray)
{
	TArray<FString> stringArray;

	JNIEnv* Env = FAndroidApplication::GetJavaEnv();

	int length = Env->GetArrayLength(javaStringArray);

	for (int i = 0; i < length; i++)
	{
		jstring javaString = static_cast<jstring>(Env->GetObjectArrayElement(javaStringArray, i));
		stringArray.Add(JavaConvert::FromJavaFString(javaString));
	}

	return stringArray;
}



