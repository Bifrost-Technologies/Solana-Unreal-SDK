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


#include "UnrealSOLNET.h"

#define LOCTEXT_NAMESPACE "UnrealSOLNET"

DEFINE_LOG_CATEGORY(LogUnrealSOLNET);

void UnrealSOLNET::Module::StartupModule() {
	#define HOSTFXR_VERSION "6.0.1"
	#define HOSTFXR_WINDOWS "hostfxr.dll"
	#define HOSTFXR_MAC "libhostfxr.dylib"
	#define HOSTFXR_LINUX "libhostfxr.so"

	#ifdef UNREALSOLNET_WINDOWS
		#define HOSTFXR_PATH "Plugins/Solana SDK/Source/ThirdParty/Runtime/Win64/host/fxr/" HOSTFXR_VERSION "/" HOSTFXR_WINDOWS
		#define UNREALSOLNET_PLATFORM_STRING(string) string
	#elif defined(UNREALSOLNET_MAC)
		#define HOSTFXR_PATH "Plugins/Solana SDK/Source/ThirdParty/Runtime/Mac/host/fxr/" HOSTFXR_VERSION "/" HOSTFXR_MAC
		#define UNREALSOLNET_PLATFORM_STRING(string) TCHAR_TO_ANSI(string)
	#elif defined(UNREALSOLNET_UNIX)
		#define HOSTFXR_PATH "Plugins/Solana SDK/Source/ThirdParty/Runtime/Linux/host/fxr/" HOSTFXR_VERSION "/" HOSTFXR_LINUX
		#define UNREALSOLNET_PLATFORM_STRING(string) TCHAR_TO_ANSI(string)
	#else
		#error "Unknown platform"
	#endif

	UnrealSOLNET::Status = UnrealSOLNET::StatusType::Stopped;
	UnrealSOLNET::ProjectPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
	UnrealSOLNET::UserAssembliesPath = UnrealSOLNET::ProjectPath + TEXT("Plugins/Solana SDK/Source/ThirdParty/Managed/");

	OnWorldPostInitializationHandle = FWorldDelegates::OnPostWorldInitialization.AddRaw(this, &UnrealSOLNET::Module::OnWorldPostInitialization);
	OnWorldCleanupHandle = FWorldDelegates::OnWorldCleanup.AddRaw(this, &UnrealSOLNET::Module::OnWorldCleanup);

	const FString hostfxrPath = UnrealSOLNET::ProjectPath + TEXT(HOSTFXR_PATH);
	const FString assembliesPath = UnrealSOLNET::ProjectPath + TEXT("Plugins/Solana SDK/Source/ThirdParty/PluginRuntime/");
	const FString runtimeConfigPath = assembliesPath + TEXT("UnrealEngine.Runtime.runtimeconfig.json");
	const FString runtimeAssemblyPath = assembliesPath + TEXT("UnrealEngine.Runtime.dll");
	const FString runtimeTypeName = TEXT("UnrealEngine.Runtime.Core, UnrealEngine.Runtime");
	const FString runtimeMethodName = TEXT("ManagedCommand");

	UE_LOG(LogUnrealSOLNET, Display, TEXT("%s: Host path set to \"%s\""), ANSI_TO_TCHAR(__FUNCTION__), *hostfxrPath);

	HostfxrLibrary = FPlatformProcess::GetDllHandle(*hostfxrPath);

	if (HostfxrLibrary) {
		UE_LOG(LogUnrealSOLNET, Display, TEXT("%s: Host library loaded successfuly!"), ANSI_TO_TCHAR(__FUNCTION__));

		hostfxr_set_error_writer_fn HostfxrSetErrorWriter = (hostfxr_set_error_writer_fn)FPlatformProcess::GetDllExport(HostfxrLibrary, TEXT("hostfxr_set_error_writer"));

		if (!HostfxrSetErrorWriter) {
			UE_LOG(LogUnrealSOLNET, Error, TEXT("%s: Unable to locate hostfxr_set_error_writer entry point!"), ANSI_TO_TCHAR(__FUNCTION__));

			return;
		}

		hostfxr_initialize_for_runtime_config_fn HostfxrInitializeForRuntimeConfig = (hostfxr_initialize_for_runtime_config_fn)FPlatformProcess::GetDllExport(HostfxrLibrary, TEXT("hostfxr_initialize_for_runtime_config"));

		if (!HostfxrInitializeForRuntimeConfig) {
			UE_LOG(LogUnrealSOLNET, Error, TEXT("%s: Unable to locate hostfxr_initialize_for_runtime_config entry point!"), ANSI_TO_TCHAR(__FUNCTION__));

			return;
		}

		hostfxr_get_runtime_delegate_fn HostfxrGetRuntimeDelegate = (hostfxr_get_runtime_delegate_fn)FPlatformProcess::GetDllExport(HostfxrLibrary, TEXT("hostfxr_get_runtime_delegate"));

		if (!HostfxrGetRuntimeDelegate) {
			UE_LOG(LogUnrealSOLNET, Error, TEXT("%s: Unable to locate hostfxr_get_runtime_delegate entry point!"), ANSI_TO_TCHAR(__FUNCTION__));

			return;
		}

		hostfxr_close_fn HostfxrClose = (hostfxr_close_fn)FPlatformProcess::GetDllExport(HostfxrLibrary, TEXT("hostfxr_close"));

		if (!HostfxrClose) {
			UE_LOG(LogUnrealSOLNET, Error, TEXT("%s: Unable to locate hostfxr_close entry point!"), ANSI_TO_TCHAR(__FUNCTION__));

			return;
		}

		HostfxrSetErrorWriter(&HostError);

		hostfxr_handle HostfxrContext = nullptr;

		if (HostfxrInitializeForRuntimeConfig(UNREALSOLNET_PLATFORM_STRING(*runtimeConfigPath), nullptr, &HostfxrContext) != 0 || !HostfxrContext) {
			UE_LOG(LogUnrealSOLNET, Error, TEXT("%s: Unable to initialize the host! Please, try to restart the engine."), ANSI_TO_TCHAR(__FUNCTION__));

			HostfxrClose(HostfxrContext);

			return;
		}

		void* hostfxrLoadAssemblyAndGetFunctionPointer = nullptr;

		if (HostfxrGetRuntimeDelegate(HostfxrContext, hdt_load_assembly_and_get_function_pointer, &hostfxrLoadAssemblyAndGetFunctionPointer) != 0 || !HostfxrGetRuntimeDelegate) {
			UE_LOG(LogUnrealSOLNET, Error, TEXT("%s: Unable to get hdt_load_assembly_and_get_function_pointer runtime delegate!"), ANSI_TO_TCHAR(__FUNCTION__));

			HostfxrClose(HostfxrContext);

			return;
		}

		HostfxrClose(HostfxrContext);

		UE_LOG(LogUnrealSOLNET, Display, TEXT("%s: Host functions loaded successfuly!"), ANSI_TO_TCHAR(__FUNCTION__));

		load_assembly_and_get_function_pointer_fn HostfxrLoadAssemblyAndGetFunctionPointer = (load_assembly_and_get_function_pointer_fn)hostfxrLoadAssemblyAndGetFunctionPointer;

		if (HostfxrLoadAssemblyAndGetFunctionPointer && HostfxrLoadAssemblyAndGetFunctionPointer(UNREALSOLNET_PLATFORM_STRING(*runtimeAssemblyPath), UNREALSOLNET_PLATFORM_STRING(*runtimeTypeName), UNREALSOLNET_PLATFORM_STRING(*runtimeMethodName), UNMANAGEDCALLERSONLY_METHOD, nullptr, (void**)&UnrealSOLNET::ManagedCommand) == 0) {
			UE_LOG(LogUnrealSOLNET, Display, TEXT("%s: Host runtime assembly loaded successfuly!"), ANSI_TO_TCHAR(__FUNCTION__));
		} else {
			UE_LOG(LogUnrealSOLNET, Error, TEXT("%s: Host runtime assembly loading failed!"), ANSI_TO_TCHAR(__FUNCTION__));

			return;
		}

		#if WITH_EDITOR
			IPlatformFile& platformFile = FPlatformFileManager::Get().GetPlatformFile();

			if (!platformFile.DirectoryExists(*UnrealSOLNET::UserAssembliesPath)) {
				platformFile.CreateDirectory(*UnrealSOLNET::UserAssembliesPath);

				if (!platformFile.DirectoryExists(*UnrealSOLNET::UserAssembliesPath))
					UE_LOG(LogUnrealSOLNET, Warning, TEXT("%s: Unable to create a folder for managed assemblies at %s."), ANSI_TO_TCHAR(__FUNCTION__), *UnrealSOLNET::UserAssembliesPath);
			}
		#endif

		if (UnrealSOLNET::ManagedCommand) {
			// Framework pointers

			int32 position = 0;
			int32 checksum = 0;

			{
				int32 head = 0;
				Shared::Functions[position++] = Shared::AssertFunctions;

				Shared::AssertFunctions[head++] = (void*)&UnrealSOLNETFramework::Assert::OutputMessage;

				checksum += head;
			}

			{
				int32 head = 0;
				Shared::Functions[position++] = Shared::DebugFunctions;

				Shared::DebugFunctions[head++] = (void*)&UnrealSOLNETFramework::Debug::Log;
				Shared::DebugFunctions[head++] = (void*)&UnrealSOLNETFramework::Debug::Exception;
				Shared::DebugFunctions[head++] = (void*)&UnrealSOLNETFramework::Debug::AddOnScreenMessage;
				Shared::DebugFunctions[head++] = (void*)&UnrealSOLNETFramework::Debug::ClearOnScreenMessages;
				Shared::DebugFunctions[head++] = (void*)&UnrealSOLNETFramework::Debug::DrawBox;
				Shared::DebugFunctions[head++] = (void*)&UnrealSOLNETFramework::Debug::DrawCapsule;
				Shared::DebugFunctions[head++] = (void*)&UnrealSOLNETFramework::Debug::DrawCone;
				Shared::DebugFunctions[head++] = (void*)&UnrealSOLNETFramework::Debug::DrawCylinder;
				Shared::DebugFunctions[head++] = (void*)&UnrealSOLNETFramework::Debug::DrawSphere;
				Shared::DebugFunctions[head++] = (void*)&UnrealSOLNETFramework::Debug::DrawLine;
				Shared::DebugFunctions[head++] = (void*)&UnrealSOLNETFramework::Debug::DrawPoint;
				Shared::DebugFunctions[head++] = (void*)&UnrealSOLNETFramework::Debug::FlushPersistentLines;

				checksum += head;
			}

			{
				int32 head = 0;
				Shared::Functions[position++] = Shared::ObjectFunctions;

				Shared::ObjectFunctions[head++] = (void*)&UnrealSOLNETFramework::Object::GetName;
				Shared::ObjectFunctions[head++] = (void*)&UnrealSOLNETFramework::Object::GetBool;
				Shared::ObjectFunctions[head++] = (void*)&UnrealSOLNETFramework::Object::GetByte;
				Shared::ObjectFunctions[head++] = (void*)&UnrealSOLNETFramework::Object::GetShort;
				Shared::ObjectFunctions[head++] = (void*)&UnrealSOLNETFramework::Object::GetInt;
				Shared::ObjectFunctions[head++] = (void*)&UnrealSOLNETFramework::Object::GetLong;
				Shared::ObjectFunctions[head++] = (void*)&UnrealSOLNETFramework::Object::GetUShort;
				Shared::ObjectFunctions[head++] = (void*)&UnrealSOLNETFramework::Object::GetUInt;
				Shared::ObjectFunctions[head++] = (void*)&UnrealSOLNETFramework::Object::GetULong;
				Shared::ObjectFunctions[head++] = (void*)&UnrealSOLNETFramework::Object::GetFloat;
				Shared::ObjectFunctions[head++] = (void*)&UnrealSOLNETFramework::Object::GetDouble;
				Shared::ObjectFunctions[head++] = (void*)&UnrealSOLNETFramework::Object::GetEnum;
				Shared::ObjectFunctions[head++] = (void*)&UnrealSOLNETFramework::Object::GetString;
				Shared::ObjectFunctions[head++] = (void*)&UnrealSOLNETFramework::Object::GetText;
				Shared::ObjectFunctions[head++] = (void*)&UnrealSOLNETFramework::Object::SetBool;
				Shared::ObjectFunctions[head++] = (void*)&UnrealSOLNETFramework::Object::SetByte;
				Shared::ObjectFunctions[head++] = (void*)&UnrealSOLNETFramework::Object::SetShort;
				Shared::ObjectFunctions[head++] = (void*)&UnrealSOLNETFramework::Object::SetInt;
				Shared::ObjectFunctions[head++] = (void*)&UnrealSOLNETFramework::Object::SetLong;
				Shared::ObjectFunctions[head++] = (void*)&UnrealSOLNETFramework::Object::SetUShort;
				Shared::ObjectFunctions[head++] = (void*)&UnrealSOLNETFramework::Object::SetUInt;
				Shared::ObjectFunctions[head++] = (void*)&UnrealSOLNETFramework::Object::SetULong;
				Shared::ObjectFunctions[head++] = (void*)&UnrealSOLNETFramework::Object::SetFloat;
				Shared::ObjectFunctions[head++] = (void*)&UnrealSOLNETFramework::Object::SetDouble;
				Shared::ObjectFunctions[head++] = (void*)&UnrealSOLNETFramework::Object::SetEnum;
				Shared::ObjectFunctions[head++] = (void*)&UnrealSOLNETFramework::Object::SetString;
				Shared::ObjectFunctions[head++] = (void*)&UnrealSOLNETFramework::Object::SetText;

				checksum += head;
			}


			// Runtime pointers

			Shared::RuntimeFunctions[0] = (void*)&UnrealSOLNET::Module::Exception;
			Shared::RuntimeFunctions[1] = (void*)&UnrealSOLNET::Module::Log;

			constexpr void* functions[3] = {
				Shared::RuntimeFunctions,
				Shared::Events,
				Shared::Functions
			};

			if (reinterpret_cast<intptr_t>(UnrealSOLNET::ManagedCommand(UnrealSOLNET::Command(functions, checksum))) == 0xF) {
				UE_LOG(LogUnrealSOLNET, Display, TEXT("%s: Host runtime assembly initialized successfuly!"), ANSI_TO_TCHAR(__FUNCTION__));
			} else {
				UE_LOG(LogUnrealSOLNET, Error, TEXT("%s: Host runtime assembly initialization failed!"), ANSI_TO_TCHAR(__FUNCTION__));

				return;
			}

			UnrealSOLNET::Status = UnrealSOLNET::StatusType::Idle;

			UE_LOG(LogUnrealSOLNET, Display, TEXT("%s: Host loaded successfuly!"), ANSI_TO_TCHAR(__FUNCTION__));
		} else {
			UE_LOG(LogUnrealSOLNET, Error, TEXT("%s: Host runtime assembly unable to load the initialization function!"), ANSI_TO_TCHAR(__FUNCTION__));

			return;
		}
	} else {
		UE_LOG(LogUnrealSOLNET, Error, TEXT("%s: Host library loading failed!"), ANSI_TO_TCHAR(__FUNCTION__));
	}
}

void UnrealSOLNET::Module::ShutdownModule() {
	FWorldDelegates::OnPostWorldInitialization.Remove(OnWorldPostInitializationHandle);
	FWorldDelegates::OnWorldCleanup.Remove(OnWorldCleanupHandle);

	FPlatformProcess::FreeDllHandle(HostfxrLibrary);
}

void UnrealSOLNET::Module::OnWorldPostInitialization(UWorld* World, const UWorld::InitializationValues InitializationValues) {
	if (World->IsGameWorld()) {
		if (UnrealSOLNET::WorldTickState == TickState::Stopped) {
			UnrealSOLNET::Engine::World = World;

			if (UnrealSOLNET::Status != UnrealSOLNET::StatusType::Stopped) {
				UnrealSOLNET::ManagedCommand(UnrealSOLNET::Command(CommandType::LoadAssemblies));
				UnrealSOLNET::Status = UnrealSOLNET::StatusType::Running;

				for (TActorIterator<AWorldSettings> currentActor(UnrealSOLNET::Engine::World); currentActor; ++currentActor) {
					RegisterTickFunction(OnPrePhysicsTickFunction, TG_PrePhysics, *currentActor);
					RegisterTickFunction(OnDuringPhysicsTickFunction, TG_DuringPhysics, *currentActor);
					RegisterTickFunction(OnPostPhysicsTickFunction, TG_PostPhysics, *currentActor);
					RegisterTickFunction(OnPostUpdateTickFunction, TG_PostUpdateWork, *currentActor);

					UnrealSOLNET::WorldTickState = UnrealSOLNET::TickState::Registered;

					if (UnrealSOLNET::Shared::Events[OnWorldBegin])
						UnrealSOLNET::ManagedCommand(UnrealSOLNET::Command(UnrealSOLNET::Shared::Events[OnWorldBegin]));

					break;
				}
			} else {
				#if WITH_EDITOR
					FNotificationInfo notificationInfo(FText::FromString(TEXT("UnrealSOLNET host is not initialized! Please, check logs and try to restart the engine.")));

					notificationInfo.ExpireDuration = 5.0f;

					FSlateNotificationManager::Get().AddNotification(notificationInfo);
				#endif
			}
		}
	}
}

void UnrealSOLNET::Module::OnWorldCleanup(UWorld* World, bool SessionEnded, bool CleanupResources) {
	if (World->IsGameWorld() && World == UnrealSOLNET::Engine::World && UnrealSOLNET::WorldTickState != UnrealSOLNET::TickState::Stopped) {
		if (UnrealSOLNET::Status != UnrealSOLNET::StatusType::Stopped) {
			if (UnrealSOLNET::Shared::Events[OnWorldEnd])
				UnrealSOLNET::ManagedCommand(UnrealSOLNET::Command(UnrealSOLNET::Shared::Events[OnWorldEnd]));

			OnPrePhysicsTickFunction.UnRegisterTickFunction();
			OnDuringPhysicsTickFunction.UnRegisterTickFunction();
			OnPostPhysicsTickFunction.UnRegisterTickFunction();
			OnPostUpdateTickFunction.UnRegisterTickFunction();

			UnrealSOLNET::ManagedCommand(UnrealSOLNET::Command(CommandType::UnloadAssemblies));
			UnrealSOLNET::Status = UnrealSOLNET::StatusType::Idle;
		}

		UnrealSOLNET::Engine::World = nullptr;
		UnrealSOLNET::WorldTickState = UnrealSOLNET::TickState::Stopped;

		FMemory::Memset(UnrealSOLNET::Shared::Events, 0, sizeof(UnrealSOLNET::Shared::Events));
	}
}

void UnrealSOLNET::Module::RegisterTickFunction(FTickFunction& TickFunction, ETickingGroup TickGroup, AWorldSettings* LevelActor) {
	TickFunction.bCanEverTick = true;
	TickFunction.bTickEvenWhenPaused = false;
	TickFunction.bStartWithTickEnabled = true;
	TickFunction.bHighPriority = true;
	TickFunction.bAllowTickOnDedicatedServer = true;
	TickFunction.bRunOnAnyThread = false;
	TickFunction.TickGroup = TickGroup;
	TickFunction.RegisterTickFunction(UnrealSOLNET::Engine::World->PersistentLevel);
}

void UnrealSOLNET::Module::HostError(const char_t* Message) {
	UE_LOG(LogUnrealSOLNET, Error, TEXT("%s: %s"), ANSI_TO_TCHAR(__FUNCTION__), *FString(Message));
}

void UnrealSOLNET::Module::Exception(const char* Message) {
	const FString message(ANSI_TO_TCHAR(Message));

	FString OutputLog(message);

	OutputLog.ReplaceCharInline(TEXT('\n'), TEXT(' '));
	OutputLog.ReplaceCharInline(TEXT('\r'), TEXT(' '));
	OutputLog.ReplaceInline(TEXT("     "), TEXT(" "));

	UE_LOG(LogUnrealSOLNET, Error, TEXT("%s: %s"), ANSI_TO_TCHAR(__FUNCTION__), *OutputLog);

	GEngine->AddOnScreenDebugMessage((uint64)-1, 10.0f, FColor::Red, *message);
}

void UnrealSOLNET::Module::Log(UnrealSOLNET::LogLevel Level, const char* Message) {
	#define UNREALSOLNET_LOG(Verbosity) UE_LOG(LogUnrealSOLNET, Verbosity, TEXT("%s: %s"), ANSI_TO_TCHAR(__FUNCTION__), *message);

	FString message(ANSI_TO_TCHAR(Message));

	if (Level == UnrealSOLNET::LogLevel::Display) {
		UNREALSOLNET_LOG(Display);
	} else if (Level == UnrealSOLNET::LogLevel::Warning) {
		UNREALSOLNET_LOG(Warning);

		GEngine->AddOnScreenDebugMessage((uint64)-1, 60.0f, FColor::Yellow, *message);
	} else if (Level == UnrealSOLNET::LogLevel::Error) {
		UNREALSOLNET_LOG(Error);

		GEngine->AddOnScreenDebugMessage((uint64)-1, 60.0f, FColor::Red, *message);
	} else if (Level == UnrealSOLNET::LogLevel::Fatal) {
		UNREALSOLNET_LOG(Error);

		GEngine->AddOnScreenDebugMessage((uint64)-1, 60.0f, FColor::Red, *message);

		UnrealSOLNET::Status = UnrealSOLNET::StatusType::Idle;
	}
}

void UnrealSOLNET::PrePhysicsTickFunction::ExecuteTick(float DeltaTime, enum ELevelTick TickType, ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent) {
	if (UnrealSOLNET::WorldTickState != UnrealSOLNET::TickState::Started && UnrealSOLNET::Shared::Events[OnWorldPostBegin]) {
		UnrealSOLNET::ManagedCommand(UnrealSOLNET::Command(UnrealSOLNET::Shared::Events[OnWorldPostBegin]));
		UnrealSOLNET::WorldTickState = UnrealSOLNET::TickState::Started;
	}

	if (UnrealSOLNET::Shared::Events[OnWorldPrePhysicsTick])
		UnrealSOLNET::ManagedCommand(UnrealSOLNET::Command(UnrealSOLNET::Shared::Events[OnWorldPrePhysicsTick], DeltaTime));
}

void UnrealSOLNET::DuringPhysicsTickFunction::ExecuteTick(float DeltaTime, enum ELevelTick TickType, ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent) {
	if (UnrealSOLNET::Shared::Events[OnWorldDuringPhysicsTick])
		UnrealSOLNET::ManagedCommand(UnrealSOLNET::Command(UnrealSOLNET::Shared::Events[OnWorldDuringPhysicsTick], DeltaTime));
}

void UnrealSOLNET::PostPhysicsTickFunction::ExecuteTick(float DeltaTime, enum ELevelTick TickType, ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent) {
	if (UnrealSOLNET::Shared::Events[OnWorldPostPhysicsTick])
		UnrealSOLNET::ManagedCommand(UnrealSOLNET::Command(UnrealSOLNET::Shared::Events[OnWorldPostPhysicsTick], DeltaTime));
}

void UnrealSOLNET::PostUpdateTickFunction::ExecuteTick(float DeltaTime, enum ELevelTick TickType, ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent) {
	if (UnrealSOLNET::Shared::Events[OnWorldPostUpdateTick])
		UnrealSOLNET::ManagedCommand(UnrealSOLNET::Command(UnrealSOLNET::Shared::Events[OnWorldPostUpdateTick], DeltaTime));
}

FString UnrealSOLNET::PrePhysicsTickFunction::DiagnosticMessage() {
	return TEXT("PrePhysicsTickFunction");
}

FString UnrealSOLNET::DuringPhysicsTickFunction::DiagnosticMessage() {
	return TEXT("DuringPhysicsTickFunction");
}

FString UnrealSOLNET::PostPhysicsTickFunction::DiagnosticMessage() {
	return TEXT("PostPhysicsTickFunction");
}

FString UnrealSOLNET::PostUpdateTickFunction::DiagnosticMessage() {
	return TEXT("PostUpdateTickFunction");
}

size_t UnrealSOLNET::Utility::Strcpy(char* Destination, const char* Source, size_t Length) {
	char* destination = Destination;
	const char* source = Source;
	size_t length = Length;

	if (length != 0 && --length != 0) {
		do {
			if ((*destination++ = *source++) == 0)
				break;
		}

		while (--length != 0);
	}

	if (length == 0) {
		if (Length != 0)
			*destination = '\0';

		while (*source++);
	}

	return (source - Source - 1);
}

size_t UnrealSOLNET::Utility::Strlen(const char* Source) {
	return strlen(Source) + 1;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(UnrealSOLNET::Module, UnrealSOLNET)
