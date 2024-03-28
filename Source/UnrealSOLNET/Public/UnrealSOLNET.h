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

// @third party code - BEGIN CoreCLR
#include "../../Dependencies/CoreCLR/includes/coreclr_delegates.h"
#include "../../Dependencies/CoreCLR/includes/hostfxr.h"
// @third party code - END CoreCLR


#include "DrawDebugHelpers.h"

#include "Engine/GameEngine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Misc/CommandLine.h"
#include "Misc/DefaultValueHelper.h"
#include "Misc/OutputDeviceNull.h"
#include "Modules/ModuleManager.h"

#include "PhysicsEngine/RadialForceComponent.h"
#include "UnrealEngine.h"

#include "UnrealSOLNET_Framework.h"
#include "UnrealSOLNET_Library.h"

#if WITH_EDITOR
	#include "Editor.h"
	#include "Framework/Notifications/NotificationManager.h"
	#include "Widgets/Notifications/SNotificationList.h"
#endif

#ifdef _WIN32
	#define UNREALSOLNET_WINDOWS 1
#elif defined(__unix__)
	#define UNREALSOLNET_UNIX 2
#elif defined(__APPLE__)
	#define UNREALSOLNET_MAC 3
#endif

#define UNREALSOLNET_NONE
#define UNREALSOLNET_BRACKET_LEFT (
#define UNREALSOLNET_BRACKET_RIGHT )

UNREALSOLNET_API DECLARE_LOG_CATEGORY_EXTERN(LogUnrealSOLNET, Log, All);

namespace UnrealSOLNET {
	enum struct StatusType : int32 {
		Stopped,
		Idle,
		Running
	};

	enum struct TickState : int32 {
		Stopped,
		Registered,
		Started
	};

	enum struct LogLevel : int32 {
		Display,
		Warning,
		Error,
		Fatal
	};

	enum struct CallbackType : int32 {
		None,
	};

	enum struct ArgumentType : int32 {
		None,
		Single,
		Integer,
		Pointer,
		Callback
	};

	enum struct CommandType : int32 {
		Initialize = 1,
		LoadAssemblies = 2,
		UnloadAssemblies = 3,
		Find = 4,
		Execute = 5
	};

	enum {
		OnWorldBegin,
		OnWorldPostBegin,
		OnWorldPrePhysicsTick,
		OnWorldDuringPhysicsTick,
		OnWorldPostPhysicsTick,
		OnWorldPostUpdateTick,
		OnWorldEnd
		
	};

	struct Callback {
		void** Parameters;
		CallbackType Type;

		FORCEINLINE Callback(void** Parameters, CallbackType Type) {
			this->Parameters = Parameters;
			this->Type = Type;
		}
	};

	struct Argument {
		union {
			float Single;
			uint32_t Integer;
			void* Pointer;
			Callback Callback;
		};
		ArgumentType Type;

		FORCEINLINE Argument(float Value) {
			this->Single = Value;
			this->Type = ArgumentType::Single;
		}

		FORCEINLINE Argument(uint32_t Value) {
			this->Integer = Value;
			this->Type = ArgumentType::Integer;
		}

		FORCEINLINE Argument(void* Value) {
			this->Pointer = Value;
			this->Type = !Value ? ArgumentType::None : ArgumentType::Pointer;
		}

		FORCEINLINE Argument(UnrealSOLNET::Callback Value) {
			this->Callback = Value;
			this->Type = ArgumentType::Callback;
		}
	};

	struct Command {
		union {
			struct {
				void* Buffer;
				int32 Checksum;
			};
			struct {
				char* Method;
				int32 Optional;
			};
			struct {
				void* Function;
				Argument Value;
			};
		};
		CommandType Type;

		FORCEINLINE Command(void* const Functions[3], int32 Checksum) {
			this->Buffer = (void*)Functions;
			this->Checksum = Checksum;
			this->Type = CommandType::Initialize;
		}

		FORCEINLINE Command(CommandType Type) {
			this->Type = Type;
		}

		FORCEINLINE Command(const char* Method, bool Optional) {
			this->Method = (char*)Method;
			this->Optional = Optional;
			this->Type = CommandType::Find;
		}

		FORCEINLINE Command(void* Function) {
			this->Function = Function;
			this->Value = nullptr;
			this->Type = CommandType::Execute;
		}

		FORCEINLINE Command(void* Function, Argument Value) {
			this->Function = Function;
			this->Value = Value;
			this->Type = CommandType::Execute;
		}
	};

	static_assert(sizeof(Callback) == 16, "Invalid size of the [Callback] structure");
	static_assert(sizeof(Argument) == 24, "Invalid size of the [Argument] structure");
	static_assert(sizeof(Command) == 40, "Invalid size of the [Command] structure");

	static void* (*ManagedCommand)(Command);

	static FString ProjectPath;
	static FString UserAssembliesPath;

	static StatusType Status = StatusType::Stopped;
	static TickState WorldTickState = TickState::Stopped;

	struct PrePhysicsTickFunction : public FTickFunction {
		virtual void ExecuteTick(float DeltaTime, enum ELevelTick TickType, ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent) override;
		virtual FString DiagnosticMessage() override;
	};

	struct DuringPhysicsTickFunction : public FTickFunction {
		virtual void ExecuteTick(float DeltaTime, enum ELevelTick TickType, ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent) override;
		virtual FString DiagnosticMessage() override;
	};

	struct PostPhysicsTickFunction : public FTickFunction {
		virtual void ExecuteTick(float DeltaTime, enum ELevelTick TickType, ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent) override;
		virtual FString DiagnosticMessage() override;
	};

	struct PostUpdateTickFunction : public FTickFunction {
		virtual void ExecuteTick(float DeltaTime, enum ELevelTick TickType, ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent) override;
		virtual FString DiagnosticMessage() override;
	};

	class Module : public IModuleInterface {
		protected:

		virtual void StartupModule() override;
		virtual void ShutdownModule() override;

		private:

		void OnWorldPostInitialization(UWorld* World, const UWorld::InitializationValues InitializationValues);
		void OnWorldCleanup(UWorld* World, bool SessionEnded, bool CleanupResources);

		static void RegisterTickFunction(FTickFunction& TickFunction, ETickingGroup TickGroup, AWorldSettings* LevelActor);
		static void HostError(const char_t* Message);
		static void Exception(const char* Message);
		static void Log(UnrealSOLNET::LogLevel Level, const char* Message);

		FDelegateHandle OnWorldPostInitializationHandle;
		FDelegateHandle OnWorldCleanupHandle;

		PrePhysicsTickFunction OnPrePhysicsTickFunction;
		DuringPhysicsTickFunction OnDuringPhysicsTickFunction;
		PostPhysicsTickFunction OnPostPhysicsTickFunction;
		PostUpdateTickFunction OnPostUpdateTickFunction;

		void* HostfxrLibrary;
	};

	namespace Engine {
		static UWorld* World;
	}

	namespace Shared {
		static constexpr int32 storageSize = 128;

		// Non-instantiable
		static void* AssertFunctions[storageSize];
		static void* DebugFunctions[storageSize];
		static void* ObjectFunctions[storageSize];


		static void* RuntimeFunctions[2];
		static void* Events[128];
		static void* Functions[128];
	}

	namespace Utility {
		FORCEINLINE static size_t Strcpy(char* Destination, const char* Source, size_t Length);
		FORCEINLINE static size_t Strlen(const char* Source);
	}
}