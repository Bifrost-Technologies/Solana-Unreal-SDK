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


#include "UnrealSOLNET_Framework.h"

DEFINE_LOG_CATEGORY(LogUnrealManaged);

namespace UnrealSOLNETFramework {
	
	#define UNREALSOLNET_GET_PROPERTY_VALUE(Type, Object, Name, Value)\
		FName name(UTF8_TO_TCHAR(Name));\
		for (TFieldIterator<Type> currentProperty(Object->GetClass()); currentProperty; ++currentProperty) {\
			Type* property = *currentProperty;\
			if (property->GetFName() == name) {\
				*Value = property->GetPropertyValue_InContainer(Object);\
				return true;\
			}\
		}\
		return false;

	#define UNREALSOLNET_SET_PROPERTY_VALUE(Type, Object, Name, Value)\
		FName name(UTF8_TO_TCHAR(Name));\
		for (TFieldIterator<Type> currentProperty(Object->GetClass()); currentProperty; ++currentProperty) {\
			Type* property = *currentProperty;\
			if (property->GetFName() == name) {\
				property->SetPropertyValue_InContainer(Object, Value);\
				return true;\
			}\
		}\
		return false;

	
	
		#define UNREALSOLNET_PIXEL_FORMAT 72
		#define UNREALSOLNET_BLEND_TYPE 6


	namespace Assert {
		void OutputMessage(const char* Message) {
			FString message(UTF8_TO_TCHAR(Message));

			UE_LOG(LogUnrealManaged, Error, TEXT("%s: %s"), ANSI_TO_TCHAR(__FUNCTION__), *message);

			GEngine->AddOnScreenDebugMessage((uint64)-1, 60.0f, FColor::Red, *message);
		}
	}

	namespace Debug {
		void Log(LogLevel Level, const char* Message) {
			#define UNREALSOLNET_FRAMEWORK_LOG(Verbosity) UE_LOG(LogUnrealManaged, Verbosity, TEXT("%s: %s"), ANSI_TO_TCHAR(__FUNCTION__), *FString(UTF8_TO_TCHAR(Message)));

			if (Level == LogLevel::Display) {
				UNREALSOLNET_FRAMEWORK_LOG(Display);
			} else if (Level == LogLevel::Warning) {
				UNREALSOLNET_FRAMEWORK_LOG(Warning);
			} else if (Level == LogLevel::Error) {
				UNREALSOLNET_FRAMEWORK_LOG(Error);
			} else if (Level == LogLevel::Fatal) {
				UNREALSOLNET_FRAMEWORK_LOG(Fatal);
			}
		}

		void Exception(const char* Message) {
			GEngine->AddOnScreenDebugMessage((uint64)-1, 10.0f, FColor::Red, *FString(UTF8_TO_TCHAR(Message)));
		}

		void AddOnScreenMessage(int32 Key, float TimeToDisplay, Color DisplayColor, const char* Message) {
			GEngine->AddOnScreenDebugMessage((uint64)Key, TimeToDisplay, DisplayColor, *FString(UTF8_TO_TCHAR(Message)));
		}

		void ClearOnScreenMessages() {
			GEngine->ClearOnScreenDebugMessages();
		}

		void DrawBox(const Vector3* Center, const Vector3* Extent, const Quaternion* Rotation, Color Color, bool PersistentLines, float LifeTime, uint8 DepthPriority, float Thickness) {
			DrawDebugBox(UnrealSOLNET::Engine::World, *Center, *Extent, *Rotation, Color, PersistentLines, LifeTime, DepthPriority, Thickness);
		}

		void DrawCapsule(const Vector3* Center, float HalfHeight, float Radius, const Quaternion* Rotation, Color Color, bool PersistentLines, float LifeTime, uint8 DepthPriority, float Thickness) {
			DrawDebugCapsule(UnrealSOLNET::Engine::World, *Center, HalfHeight, Radius, *Rotation, Color, PersistentLines, LifeTime, DepthPriority, Thickness);
		}

		void DrawCone(const Vector3* Origin, const Vector3* Direction, float Length, float AngleWidth, float AngleHeight, int32 Sides, Color Color, bool PersistentLines, float LifeTime, uint8 DepthPriority, float Thickness) {
			DrawDebugCone(UnrealSOLNET::Engine::World, *Origin, *Direction, Length, AngleWidth, AngleHeight, Sides, Color, PersistentLines, LifeTime, DepthPriority, Thickness);
		}

		void DrawCylinder(const Vector3* Start, const Vector3* End, float Radius, int32 Segments, Color Color, bool PersistentLines, float LifeTime, uint8 DepthPriority, float Thickness) {
			DrawDebugCylinder(UnrealSOLNET::Engine::World, *Start, *End, Radius, Segments, Color, PersistentLines, LifeTime, DepthPriority, Thickness);
		}

		void DrawSphere(const Vector3* Center, float Radius, int32 Segments, Color Color, bool PersistentLines, float LifeTime, uint8 DepthPriority, float Thickness) {
			DrawDebugSphere(UnrealSOLNET::Engine::World, *Center, Radius, Segments, Color, PersistentLines, LifeTime, DepthPriority, Thickness);
		}

		void DrawLine(const Vector3* Start, const Vector3* End, Color Color, bool PersistentLines, float LifeTime, uint8 DepthPriority, float Thickness) {
			DrawDebugLine(UnrealSOLNET::Engine::World, *Start, *End, Color, PersistentLines, LifeTime, DepthPriority, Thickness);
		}

		void DrawPoint(const Vector3* Location, float Size, Color Color, bool PersistentLines, float LifeTime, uint8 DepthPriority) {
			DrawDebugPoint(UnrealSOLNET::Engine::World, *Location, Size, Color, PersistentLines, LifeTime, DepthPriority);
		}

		void FlushPersistentLines() {
			FlushPersistentDebugLines(UnrealSOLNET::Engine::World);
		}
	}

	namespace Object {
		


		void GetName(UObject* Object, char* Name) {
			const char* name = TCHAR_TO_UTF8(*Object->GetName());

			UnrealSOLNET::Utility::Strcpy(Name, name, UnrealSOLNET::Utility::Strlen(name));
		}

		bool GetBool(UObject* Object, const char* Name, bool* Value) {
			UNREALSOLNET_GET_PROPERTY_VALUE(FBoolProperty, Object, Name, Value);
		}

		bool GetByte(UObject* Object, const char* Name, uint8* Value) {
			UNREALSOLNET_GET_PROPERTY_VALUE(FByteProperty, Object, Name, Value);
		}

		bool GetShort(UObject* Object, const char* Name, int16* Value) {
			UNREALSOLNET_GET_PROPERTY_VALUE(FInt16Property, Object, Name, Value);
		}

		bool GetInt(UObject* Object, const char* Name, int32* Value) {
			UNREALSOLNET_GET_PROPERTY_VALUE(FIntProperty, Object, Name, Value);
		}

		bool GetLong(UObject* Object, const char* Name, int64* Value) {
			UNREALSOLNET_GET_PROPERTY_VALUE(FInt64Property, Object, Name, Value);
		}

		bool GetUShort(UObject* Object, const char* Name, uint16* Value) {
			UNREALSOLNET_GET_PROPERTY_VALUE(FUInt16Property, Object, Name, Value);
		}

		bool GetUInt(UObject* Object, const char* Name, uint32* Value) {
			UNREALSOLNET_GET_PROPERTY_VALUE(FUInt32Property, Object, Name, Value);
		}

		bool GetULong(UObject* Object, const char* Name, uint64* Value) {
			UNREALSOLNET_GET_PROPERTY_VALUE(FUInt64Property, Object, Name, Value);
		}

		bool GetFloat(UObject* Object, const char* Name, float* Value) {
			UNREALSOLNET_GET_PROPERTY_VALUE(FFloatProperty, Object, Name, Value);
		}

		bool GetDouble(UObject* Object, const char* Name, double* Value) {
			UNREALSOLNET_GET_PROPERTY_VALUE(FDoubleProperty, Object, Name, Value);
		}

		bool GetEnum(UObject* Object, const char* Name, int32* Value) {
			FName name(UTF8_TO_TCHAR(Name));

			for (TFieldIterator<FNumericProperty> currentProperty(Object->GetClass()); currentProperty; ++currentProperty) {
				FNumericProperty* property = *currentProperty;

				if (property->GetFName() == name) {
					*Value = static_cast<int32>(property->GetSignedIntPropertyValue(property->ContainerPtrToValuePtr<int32>(Object)));

					return true;
				}
			}

			return false;
		}

		bool GetString(UObject* Object, const char* Name, char* Value) {
			FName name(UTF8_TO_TCHAR(Name));

			for (TFieldIterator<FStrProperty> currentProperty(Object->GetClass()); currentProperty; ++currentProperty) {
				FStrProperty* property = *currentProperty;

				if (property->GetFName() == name) {
					const char* string = TCHAR_TO_UTF8(*property->GetPropertyValue_InContainer(Object));

					UnrealSOLNET::Utility::Strcpy((char*)Value, string, UnrealSOLNET::Utility::Strlen(string));

					return true;
				}
			}

			return false;
		}

		bool GetText(UObject* Object, const char* Name, char* Value) {
			FName name(UTF8_TO_TCHAR(Name));

			for (TFieldIterator<FTextProperty> currentProperty(Object->GetClass()); currentProperty; ++currentProperty) {
				FTextProperty* property = *currentProperty;

				if (property->GetFName() == name) {
					const char* string = TCHAR_TO_UTF8(*property->GetPropertyValue_InContainer(Object).ToString());

					UnrealSOLNET::Utility::Strcpy(Value, string, UnrealSOLNET::Utility::Strlen(string));

					return true;
				}
			}

			return false;
		}

		bool SetBool(UObject* Object, const char* Name, bool Value) {
			UNREALSOLNET_SET_PROPERTY_VALUE(FBoolProperty, Object, Name, Value);
		}

		bool SetByte(UObject* Object, const char* Name, uint8 Value) {
			UNREALSOLNET_SET_PROPERTY_VALUE(FByteProperty, Object, Name, Value);
		}

		bool SetShort(UObject* Object, const char* Name, int16 Value) {
			UNREALSOLNET_SET_PROPERTY_VALUE(FInt16Property, Object, Name, Value);
		}

		bool SetInt(UObject* Object, const char* Name, int32 Value) {
			UNREALSOLNET_SET_PROPERTY_VALUE(FIntProperty, Object, Name, Value);
		}

		bool SetLong(UObject* Object, const char* Name, int64 Value) {
			UNREALSOLNET_SET_PROPERTY_VALUE(FInt64Property, Object, Name, Value);
		}

		bool SetUShort(UObject* Object, const char* Name, uint16 Value) {
			UNREALSOLNET_SET_PROPERTY_VALUE(FUInt16Property, Object, Name, Value);
		}

		bool SetUInt(UObject* Object, const char* Name, uint32 Value) {
			UNREALSOLNET_SET_PROPERTY_VALUE(FUInt32Property, Object, Name, Value);
		}

		bool SetULong(UObject* Object, const char* Name, uint64 Value) {
			UNREALSOLNET_SET_PROPERTY_VALUE(FUInt64Property, Object, Name, Value);
		}

		bool SetFloat(UObject* Object, const char* Name, float Value) {
			UNREALSOLNET_SET_PROPERTY_VALUE(FFloatProperty, Object, Name, Value);
		}

		bool SetDouble(UObject* Object, const char* Name, double Value) {
			UNREALSOLNET_SET_PROPERTY_VALUE(FDoubleProperty, Object, Name, Value);
		}

		bool SetEnum(UObject* Object, const char* Name, int32 Value) {
			FName name(UTF8_TO_TCHAR(Name));

			for (TFieldIterator<FNumericProperty> currentProperty(Object->GetClass()); currentProperty; ++currentProperty) {
				FNumericProperty* property = *currentProperty;

				if (property->GetFName() == name) {
					property->SetIntPropertyValue(property->ContainerPtrToValuePtr<int32>(Object), static_cast<int64>(Value));

					return true;
				}
			}

			return false;
		}

		bool SetString(UObject* Object, const char* Name, const char* Value) {
			FName name(UTF8_TO_TCHAR(Name));

			for (TFieldIterator<FStrProperty> currentProperty(Object->GetClass()); currentProperty; ++currentProperty) {
				FStrProperty* property = *currentProperty;

				if (property->GetFName() == name) {
					property->SetPropertyValue_InContainer(Object, FString(UTF8_TO_TCHAR(Value)));

					return true;
				}
			}

			return false;
		}

		bool SetText(UObject* Object, const char* Name, const char* Value) {
			FName name(UTF8_TO_TCHAR(Name));

			for (TFieldIterator<FTextProperty> currentProperty(Object->GetClass()); currentProperty; ++currentProperty) {
				FTextProperty* property = *currentProperty;

				if (property->GetFName() == name) {
					property->SetPropertyValue_InContainer(Object, FText::FromString(FString(UTF8_TO_TCHAR(Value))));

					return true;
				}
			}

			return false;
		}
	}
}
