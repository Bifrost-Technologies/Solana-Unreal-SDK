/*
 *  UnrealSOLNET
 *  Copyright (c) 2024 Bifrost Inc.
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

UNREALSOLNET_API DECLARE_LOG_CATEGORY_EXTERN(LogUnrealManaged, Log, All);

namespace UnrealSOLNETFramework {

	enum struct LogLevel : int32 {
		Display,
		Warning,
		Error,
		Fatal
	};

	typedef void (*ConsoleVariableDelegate)();

	typedef void (*ConsoleCommandDelegate)(float);

	struct Color {
		uint8 B;
		uint8 G;
		uint8 R;
		uint8 A;

		FORCEINLINE Color(FColor Value) {
			this->R = Value.R;
			this->G = Value.G;
			this->B = Value.B;
			this->A = Value.A;
		}

		FORCEINLINE operator FColor() const { return FColor(R, G, B, A); }
	};

	struct Vector2 {
		float X;
		float Y;

		FORCEINLINE Vector2(FVector2D Value) {
			this->X = Value.X;
			this->Y = Value.Y;
		}

		FORCEINLINE operator FVector2D() const { return FVector2D(X, Y); }
	};

	struct Vector3 {
		float X;
		float Y;
		float Z;

		FORCEINLINE Vector3(FVector Value) {
			this->X = Value.X;
			this->Y = Value.Y;
			this->Z = Value.Z;
		}

		FORCEINLINE operator FVector() const { return FVector(X, Y, Z); }
	};

	struct Quaternion {
		float X;
		float Y;
		float Z;
		float W;

		FORCEINLINE Quaternion(FQuat Value) {
			this->X = Value.X;
			this->Y = Value.Y;
			this->Z = Value.Z;
			this->W = Value.W;
		}

		FORCEINLINE operator FQuat() const { return FQuat(X, Y, Z, W); }
	};

	struct Transform {
		Vector3 Location;
		Quaternion Rotation;
		Vector3 Scale;

		FORCEINLINE Transform(const FTransform& Value) :
			Location(Value.GetTranslation()),
			Rotation(Value.GetRotation()),
			Scale(Value.GetScale3D()) { }

		FORCEINLINE operator FTransform() const { return FTransform(Rotation, Location, Scale); }
	};

	struct LinearColor {
		float R;
		float G;
		float B;
		float A;

		FORCEINLINE LinearColor(FLinearColor Value) {
			this->R = Value.R;
			this->G = Value.G;
			this->B = Value.B;
			this->A = Value.A;
		}

		FORCEINLINE operator FLinearColor() const { return FLinearColor(R, G, B, A); }
	};
	namespace Assert {
		static void OutputMessage(const char* Message);
	}

	namespace Debug {
		static void Log(LogLevel Level, const char* Message);
		static void Exception(const char* Exception);
		static void AddOnScreenMessage(int32 Key, float TimeToDisplay, Color DisplayColor, const char* Message);
		static void ClearOnScreenMessages();
		static void DrawBox(const Vector3* Center, const Vector3* Extent, const Quaternion* Rotation, Color Color, bool PersistentLines, float LifeTime, uint8 DepthPriority, float Thickness);
		static void DrawCapsule(const Vector3* Center, float HalfHeight, float Radius, const Quaternion* Rotation, Color Color, bool PersistentLines, float LifeTime, uint8 DepthPriority, float Thickness);
		static void DrawCone(const Vector3* Origin, const Vector3* Direction, float Length, float AngleWidth, float AngleHeight, int32 Sides, Color Color, bool PersistentLines, float LifeTime, uint8 DepthPriority, float Thickness);
		static void DrawCylinder(const Vector3* Start, const Vector3* End, float Radius, int32 Segments, Color Color, bool PersistentLines, float LifeTime, uint8 DepthPriority, float Thickness);
		static void DrawSphere(const Vector3* Center, float Radius, int32 Segments, Color Color, bool PersistentLines, float LifeTime, uint8 DepthPriority, float Thickness);
		static void DrawLine(const Vector3* Start, const Vector3* End, Color Color, bool PersistentLines, float LifeTime, uint8 DepthPriority, float Thickness);
		static void DrawPoint(const Vector3* Location, float Size, Color Color, bool PersistentLines, float LifeTime, uint8 DepthPriority);
		static void FlushPersistentLines();
	}

	namespace Object {

		static void GetName(UObject* Object, char* Name);
		static bool GetBool(UObject* Object, const char* Name, bool* value);
		static bool GetByte(UObject* Object, const char* Name, uint8* Value);
		static bool GetShort(UObject* Object, const char* Name, int16* Value);
		static bool GetInt(UObject* Object, const char* Name, int32* Value);
		static bool GetLong(UObject* Object, const char* Name, int64* Value);
		static bool GetUShort(UObject* Object, const char* Name, uint16* Value);
		static bool GetUInt(UObject* Object, const char* Name, uint32* Value);
		static bool GetULong(UObject* Object, const char* Name, uint64* Value);
		static bool GetFloat(UObject* Object, const char* Name, float* Value);
		static bool GetDouble(UObject* Object, const char* Name, double* Value);
		static bool GetEnum(UObject* Object, const char* Name, int32* Value);
		static bool GetString(UObject* Object, const char* Name, char* Value);
		static bool GetText(UObject* Object, const char* Name, char* Value);
		static bool SetBool(UObject* Object, const char* Name, bool value);
		static bool SetByte(UObject* Object, const char* Name, uint8 Value);
		static bool SetShort(UObject* Object, const char* Name, int16 Value);
		static bool SetInt(UObject* Object, const char* Name, int32 Value);
		static bool SetLong(UObject* Object, const char* Name, int64 Value);
		static bool SetUShort(UObject* Object, const char* Name, uint16 Value);
		static bool SetUInt(UObject* Object, const char* Name, uint32 Value);
		static bool SetULong(UObject* Object, const char* Name, uint64 Value);
		static bool SetFloat(UObject* Object, const char* Name, float Value);
		static bool SetDouble(UObject* Object, const char* Name, double Value);
		static bool SetEnum(UObject* Object, const char* Name, int32 Value);
		static bool SetString(UObject* Object, const char* Name, const char* Value);
		static bool SetText(UObject* Object, const char* Name, const char* Value);
	}
	
}