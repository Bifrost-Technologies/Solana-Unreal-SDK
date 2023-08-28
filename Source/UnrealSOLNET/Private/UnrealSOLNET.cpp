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


#include "UnrealSOLNET.h"

#define LOCTEXT_NAMESPACE "UnrealSOLNET"

DEFINE_LOG_CATEGORY(LogUnrealSOLNET);

void UnrealSOLNET::Module::StartupModule() {
	#define HOSTFXR_VERSION "6.0.1"
	#define HOSTFXR_WINDOWS "hostfxr.dll"
	#define HOSTFXR_MAC "libhostfxr.dylib"
	#define HOSTFXR_LINUX "libhostfxr.so"

	#ifdef UNREALSOLNET_WINDOWS
		#define HOSTFXR_PATH "Plugins/Solana SDK/Runtime/Win64/host/fxr/" HOSTFXR_VERSION "/" HOSTFXR_WINDOWS
		#define UNREALSOLNET_PLATFORM_STRING(string) string
	#elif defined(UNREALSOLNET_MAC)
		#define HOSTFXR_PATH "Plugins/Solana SDK/Runtime/Mac/host/fxr/" HOSTFXR_VERSION "/" HOSTFXR_MAC
		#define UNREALSOLNET_PLATFORM_STRING(string) TCHAR_TO_ANSI(string)
	#elif defined(UNREALSOLNET_UNIX)
		#define HOSTFXR_PATH "Plugins/Solana SDK/Runtime/Linux/host/fxr/" HOSTFXR_VERSION "/" HOSTFXR_LINUX
		#define UNREALSOLNET_PLATFORM_STRING(string) TCHAR_TO_ANSI(string)
	#else
		#error "Unknown platform"
	#endif

	UnrealSOLNET::Status = UnrealSOLNET::StatusType::Stopped;
	UnrealSOLNET::ProjectPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectDir());
	UnrealSOLNET::UserAssembliesPath = UnrealSOLNET::ProjectPath + TEXT("Plugins/Solana SDK/SolanaKit/");

	OnWorldPostInitializationHandle = FWorldDelegates::OnPostWorldInitialization.AddRaw(this, &UnrealSOLNET::Module::OnWorldPostInitialization);
	OnWorldCleanupHandle = FWorldDelegates::OnWorldCleanup.AddRaw(this, &UnrealSOLNET::Module::OnWorldCleanup);

	const FString hostfxrPath = UnrealSOLNET::ProjectPath + TEXT(HOSTFXR_PATH);
	const FString assembliesPath = UnrealSOLNET::ProjectPath + TEXT("Plugins/Solana SDK/Managed/");
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
				Shared::Functions[position++] = Shared::CommandLineFunctions;

				Shared::CommandLineFunctions[head++] = (void*)&UnrealSOLNETFramework::CommandLine::Get;
				Shared::CommandLineFunctions[head++] = (void*)&UnrealSOLNETFramework::CommandLine::Set;
				Shared::CommandLineFunctions[head++] = (void*)&UnrealSOLNETFramework::CommandLine::Append;

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

				Shared::ObjectFunctions[head++] = (void*)&UnrealSOLNETFramework::Object::IsValid;
				Shared::ObjectFunctions[head++] = (void*)&UnrealSOLNETFramework::Object::Load;
				Shared::ObjectFunctions[head++] = (void*)&UnrealSOLNETFramework::Object::Rename;
				Shared::ObjectFunctions[head++] = (void*)&UnrealSOLNETFramework::Object::Invoke;
				Shared::ObjectFunctions[head++] = (void*)&UnrealSOLNETFramework::Object::ToActor;
				Shared::ObjectFunctions[head++] = (void*)&UnrealSOLNETFramework::Object::ToComponent;
				Shared::ObjectFunctions[head++] = (void*)&UnrealSOLNETFramework::Object::GetID;
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

			{
				int32 head = 0;
				Shared::Functions[position++] = Shared::ApplicationFunctions;

				Shared::ApplicationFunctions[head++] = (void*)&UnrealSOLNETFramework::Application::IsCanEverRender;
				Shared::ApplicationFunctions[head++] = (void*)&UnrealSOLNETFramework::Application::IsPackagedForDistribution;
				Shared::ApplicationFunctions[head++] = (void*)&UnrealSOLNETFramework::Application::IsPackagedForShipping;
				Shared::ApplicationFunctions[head++] = (void*)&UnrealSOLNETFramework::Application::GetProjectDirectory;
				Shared::ApplicationFunctions[head++] = (void*)&UnrealSOLNETFramework::Application::GetDefaultLanguage;
				Shared::ApplicationFunctions[head++] = (void*)&UnrealSOLNETFramework::Application::GetProjectName;
				Shared::ApplicationFunctions[head++] = (void*)&UnrealSOLNETFramework::Application::GetVolumeMultiplier;
				Shared::ApplicationFunctions[head++] = (void*)&UnrealSOLNETFramework::Application::SetProjectName;
				Shared::ApplicationFunctions[head++] = (void*)&UnrealSOLNETFramework::Application::SetVolumeMultiplier;
				Shared::ApplicationFunctions[head++] = (void*)&UnrealSOLNETFramework::Application::RequestExit;

				checksum += head;
			}

			{
				int32 head = 0;
				Shared::Functions[position++] = Shared::ConsoleManagerFunctions;

				Shared::ConsoleManagerFunctions[head++] = (void*)&UnrealSOLNETFramework::ConsoleManager::IsRegisteredVariable;
				Shared::ConsoleManagerFunctions[head++] = (void*)&UnrealSOLNETFramework::ConsoleManager::FindVariable;
				Shared::ConsoleManagerFunctions[head++] = (void*)&UnrealSOLNETFramework::ConsoleManager::RegisterVariableBool;
				Shared::ConsoleManagerFunctions[head++] = (void*)&UnrealSOLNETFramework::ConsoleManager::RegisterVariableInt;
				Shared::ConsoleManagerFunctions[head++] = (void*)&UnrealSOLNETFramework::ConsoleManager::RegisterVariableFloat;
				Shared::ConsoleManagerFunctions[head++] = (void*)&UnrealSOLNETFramework::ConsoleManager::RegisterVariableString;
				Shared::ConsoleManagerFunctions[head++] = (void*)&UnrealSOLNETFramework::ConsoleManager::RegisterCommand;
				Shared::ConsoleManagerFunctions[head++] = (void*)&UnrealSOLNETFramework::ConsoleManager::UnregisterObject;

				checksum += head;
			}

			{
				int32 head = 0;
				Shared::Functions[position++] = Shared::EngineFunctions;

				Shared::EngineFunctions[head++] = (void*)&UnrealSOLNETFramework::Engine::IsSplitScreen;
				Shared::EngineFunctions[head++] = (void*)&UnrealSOLNETFramework::Engine::IsEditor;
				Shared::EngineFunctions[head++] = (void*)&UnrealSOLNETFramework::Engine::IsForegroundWindow;
				Shared::EngineFunctions[head++] = (void*)&UnrealSOLNETFramework::Engine::IsExitRequested;
				Shared::EngineFunctions[head++] = (void*)&UnrealSOLNETFramework::Engine::GetNetMode;
				Shared::EngineFunctions[head++] = (void*)&UnrealSOLNETFramework::Engine::GetFrameNumber;
				Shared::EngineFunctions[head++] = (void*)&UnrealSOLNETFramework::Engine::GetViewportSize;
				Shared::EngineFunctions[head++] = (void*)&UnrealSOLNETFramework::Engine::GetScreenResolution;
				Shared::EngineFunctions[head++] = (void*)&UnrealSOLNETFramework::Engine::GetWindowMode;
				Shared::EngineFunctions[head++] = (void*)&UnrealSOLNETFramework::Engine::GetVersion;
				Shared::EngineFunctions[head++] = (void*)&UnrealSOLNETFramework::Engine::GetMaxFPS;
				Shared::EngineFunctions[head++] = (void*)&UnrealSOLNETFramework::Engine::SetMaxFPS;
				Shared::EngineFunctions[head++] = (void*)&UnrealSOLNETFramework::Engine::SetTitle;
				Shared::EngineFunctions[head++] = (void*)&UnrealSOLNETFramework::Engine::AddActionMapping;
				Shared::EngineFunctions[head++] = (void*)&UnrealSOLNETFramework::Engine::AddAxisMapping;
				Shared::EngineFunctions[head++] = (void*)&UnrealSOLNETFramework::Engine::ForceGarbageCollection;
				Shared::EngineFunctions[head++] = (void*)&UnrealSOLNETFramework::Engine::DelayGarbageCollection;

				checksum += head;
			}

			{
				int32 head = 0;
				Shared::Functions[position++] = Shared::HeadMountedDisplayFunctions;

				Shared::HeadMountedDisplayFunctions[head++] = (void*)&UnrealSOLNETFramework::HeadMountedDisplay::IsConnected;
				Shared::HeadMountedDisplayFunctions[head++] = (void*)&UnrealSOLNETFramework::HeadMountedDisplay::GetEnabled;
				Shared::HeadMountedDisplayFunctions[head++] = (void*)&UnrealSOLNETFramework::HeadMountedDisplay::GetLowPersistenceMode;
				Shared::HeadMountedDisplayFunctions[head++] = (void*)&UnrealSOLNETFramework::HeadMountedDisplay::GetDeviceName;
				Shared::HeadMountedDisplayFunctions[head++] = (void*)&UnrealSOLNETFramework::HeadMountedDisplay::SetEnable;
				Shared::HeadMountedDisplayFunctions[head++] = (void*)&UnrealSOLNETFramework::HeadMountedDisplay::SetLowPersistenceMode;

				checksum += head;
			}

			{
				int32 head = 0;
				Shared::Functions[position++] = Shared::WorldFunctions;

				Shared::WorldFunctions[head++] = (void*)&UnrealSOLNETFramework::World::ForEachActor;
				Shared::WorldFunctions[head++] = (void*)&UnrealSOLNETFramework::World::GetActorCount;
				Shared::WorldFunctions[head++] = (void*)&UnrealSOLNETFramework::World::GetDeltaSeconds;
				Shared::WorldFunctions[head++] = (void*)&UnrealSOLNETFramework::World::GetRealTimeSeconds;
				Shared::WorldFunctions[head++] = (void*)&UnrealSOLNETFramework::World::GetTimeSeconds;
				Shared::WorldFunctions[head++] = (void*)&UnrealSOLNETFramework::World::GetCurrentLevelName;
				Shared::WorldFunctions[head++] = (void*)&UnrealSOLNETFramework::World::GetSimulatePhysics;
				Shared::WorldFunctions[head++] = (void*)&UnrealSOLNETFramework::World::GetWorldOrigin;
				Shared::WorldFunctions[head++] = (void*)&UnrealSOLNETFramework::World::GetActor;
				Shared::WorldFunctions[head++] = (void*)&UnrealSOLNETFramework::World::GetActorByTag;
				Shared::WorldFunctions[head++] = (void*)&UnrealSOLNETFramework::World::GetActorByID;
				Shared::WorldFunctions[head++] = (void*)&UnrealSOLNETFramework::World::GetFirstPlayerController;
				Shared::WorldFunctions[head++] = (void*)&UnrealSOLNETFramework::World::GetGameMode;
				Shared::WorldFunctions[head++] = (void*)&UnrealSOLNETFramework::World::SetOnActorBeginOverlapCallback;
				Shared::WorldFunctions[head++] = (void*)&UnrealSOLNETFramework::World::SetOnActorBeginCursorOverCallback;
				Shared::WorldFunctions[head++] = (void*)&UnrealSOLNETFramework::World::SetOnActorEndCursorOverCallback;
				Shared::WorldFunctions[head++] = (void*)&UnrealSOLNETFramework::World::SetOnActorClickedCallback;
				Shared::WorldFunctions[head++] = (void*)&UnrealSOLNETFramework::World::SetOnActorReleasedCallback;
				Shared::WorldFunctions[head++] = (void*)&UnrealSOLNETFramework::World::SetOnActorEndOverlapCallback;
				Shared::WorldFunctions[head++] = (void*)&UnrealSOLNETFramework::World::SetOnActorHitCallback;
				Shared::WorldFunctions[head++] = (void*)&UnrealSOLNETFramework::World::SetOnComponentBeginOverlapCallback;
				Shared::WorldFunctions[head++] = (void*)&UnrealSOLNETFramework::World::SetOnComponentEndOverlapCallback;
				Shared::WorldFunctions[head++] = (void*)&UnrealSOLNETFramework::World::SetOnComponentHitCallback;
				Shared::WorldFunctions[head++] = (void*)&UnrealSOLNETFramework::World::SetOnComponentBeginCursorOverCallback;
				Shared::WorldFunctions[head++] = (void*)&UnrealSOLNETFramework::World::SetOnComponentEndCursorOverCallback;
				Shared::WorldFunctions[head++] = (void*)&UnrealSOLNETFramework::World::SetOnComponentClickedCallback;
				Shared::WorldFunctions[head++] = (void*)&UnrealSOLNETFramework::World::SetOnComponentReleasedCallback;
				Shared::WorldFunctions[head++] = (void*)&UnrealSOLNETFramework::World::SetSimulatePhysics;
				Shared::WorldFunctions[head++] = (void*)&UnrealSOLNETFramework::World::SetGravity;
				Shared::WorldFunctions[head++] = (void*)&UnrealSOLNETFramework::World::SetWorldOrigin;
				Shared::WorldFunctions[head++] = (void*)&UnrealSOLNETFramework::World::OpenLevel;
				Shared::WorldFunctions[head++] = (void*)&UnrealSOLNETFramework::World::LineTraceTestByChannel;
				Shared::WorldFunctions[head++] = (void*)&UnrealSOLNETFramework::World::LineTraceTestByProfile;
				Shared::WorldFunctions[head++] = (void*)&UnrealSOLNETFramework::World::LineTraceSingleByChannel;
				Shared::WorldFunctions[head++] = (void*)&UnrealSOLNETFramework::World::LineTraceSingleByProfile;
				Shared::WorldFunctions[head++] = (void*)&UnrealSOLNETFramework::World::SweepTestByChannel;
				Shared::WorldFunctions[head++] = (void*)&UnrealSOLNETFramework::World::SweepTestByProfile;
				Shared::WorldFunctions[head++] = (void*)&UnrealSOLNETFramework::World::SweepSingleByChannel;
				Shared::WorldFunctions[head++] = (void*)&UnrealSOLNETFramework::World::SweepSingleByProfile;
				Shared::WorldFunctions[head++] = (void*)&UnrealSOLNETFramework::World::OverlapAnyTestByChannel;
				Shared::WorldFunctions[head++] = (void*)&UnrealSOLNETFramework::World::OverlapAnyTestByProfile;
				Shared::WorldFunctions[head++] = (void*)&UnrealSOLNETFramework::World::OverlapBlockingTestByChannel;
				Shared::WorldFunctions[head++] = (void*)&UnrealSOLNETFramework::World::OverlapBlockingTestByProfile;

				checksum += head;
			}

			{
				int32 head = 0;
				Shared::Functions[position++] = Shared::AssetFunctions;

				Shared::AssetFunctions[head++] = (void*)&UnrealSOLNETFramework::Asset::IsValid;
				Shared::AssetFunctions[head++] = (void*)&UnrealSOLNETFramework::Asset::GetName;
				Shared::AssetFunctions[head++] = (void*)&UnrealSOLNETFramework::Asset::GetPath;

				checksum += head;
			}

			{
				int32 head = 0;
				Shared::Functions[position++] = Shared::AssetRegistryFunctions;

				Shared::AssetRegistryFunctions[head++] = (void*)&UnrealSOLNETFramework::AssetRegistry::Get;
				Shared::AssetRegistryFunctions[head++] = (void*)&UnrealSOLNETFramework::AssetRegistry::HasAssets;
				Shared::AssetRegistryFunctions[head++] = (void*)&UnrealSOLNETFramework::AssetRegistry::ForEachAsset;

				checksum += head;
			}

			{
				int32 head = 0;
				Shared::Functions[position++] = Shared::BlueprintFunctions;

				Shared::BlueprintFunctions[head++] = (void*)&UnrealSOLNETFramework::Blueprint::IsValidActorClass;
				Shared::BlueprintFunctions[head++] = (void*)&UnrealSOLNETFramework::Blueprint::IsValidComponentClass;

				checksum += head;
			}

			{
				int32 head = 0;
				Shared::Functions[position++] = Shared::ConsoleObjectFunctions;

				Shared::ConsoleObjectFunctions[head++] = (void*)&UnrealSOLNETFramework::ConsoleObject::IsBool;
				Shared::ConsoleObjectFunctions[head++] = (void*)&UnrealSOLNETFramework::ConsoleObject::IsInt;
				Shared::ConsoleObjectFunctions[head++] = (void*)&UnrealSOLNETFramework::ConsoleObject::IsFloat;
				Shared::ConsoleObjectFunctions[head++] = (void*)&UnrealSOLNETFramework::ConsoleObject::IsString;

				checksum += head;
			}

			{
				int32 head = 0;
				Shared::Functions[position++] = Shared::ConsoleVariableFunctions;

				Shared::ConsoleVariableFunctions[head++] = (void*)&UnrealSOLNETFramework::ConsoleVariable::GetBool;
				Shared::ConsoleVariableFunctions[head++] = (void*)&UnrealSOLNETFramework::ConsoleVariable::GetInt;
				Shared::ConsoleVariableFunctions[head++] = (void*)&UnrealSOLNETFramework::ConsoleVariable::GetFloat;
				Shared::ConsoleVariableFunctions[head++] = (void*)&UnrealSOLNETFramework::ConsoleVariable::GetString;
				Shared::ConsoleVariableFunctions[head++] = (void*)&UnrealSOLNETFramework::ConsoleVariable::SetBool;
				Shared::ConsoleVariableFunctions[head++] = (void*)&UnrealSOLNETFramework::ConsoleVariable::SetInt;
				Shared::ConsoleVariableFunctions[head++] = (void*)&UnrealSOLNETFramework::ConsoleVariable::SetFloat;
				Shared::ConsoleVariableFunctions[head++] = (void*)&UnrealSOLNETFramework::ConsoleVariable::SetString;
				Shared::ConsoleVariableFunctions[head++] = (void*)&UnrealSOLNETFramework::ConsoleVariable::SetOnChangedCallback;
				Shared::ConsoleVariableFunctions[head++] = (void*)&UnrealSOLNETFramework::ConsoleVariable::ClearOnChangedCallback;

				checksum += head;
			}

			{
				int32 head = 0;
				Shared::Functions[position++] = Shared::ActorFunctions;

				Shared::ActorFunctions[head++] = (void*)&UnrealSOLNETFramework::Actor::IsRootComponentMovable;
				Shared::ActorFunctions[head++] = (void*)&UnrealSOLNETFramework::Actor::IsOverlappingActor;
				Shared::ActorFunctions[head++] = (void*)&UnrealSOLNETFramework::Actor::ForEachComponent;
				Shared::ActorFunctions[head++] = (void*)&UnrealSOLNETFramework::Actor::ForEachAttachedActor;
				Shared::ActorFunctions[head++] = (void*)&UnrealSOLNETFramework::Actor::ForEachChildActor;
				Shared::ActorFunctions[head++] = (void*)&UnrealSOLNETFramework::Actor::ForEachOverlappingActor;
				Shared::ActorFunctions[head++] = (void*)&UnrealSOLNETFramework::Actor::Spawn;
				Shared::ActorFunctions[head++] = (void*)&UnrealSOLNETFramework::Actor::Destroy;
				Shared::ActorFunctions[head++] = (void*)&UnrealSOLNETFramework::Actor::Rename;
				Shared::ActorFunctions[head++] = (void*)&UnrealSOLNETFramework::Actor::Hide;
				Shared::ActorFunctions[head++] = (void*)&UnrealSOLNETFramework::Actor::TeleportTo;
				Shared::ActorFunctions[head++] = (void*)&UnrealSOLNETFramework::Actor::GetComponent;
				Shared::ActorFunctions[head++] = (void*)&UnrealSOLNETFramework::Actor::GetComponentByTag;
				Shared::ActorFunctions[head++] = (void*)&UnrealSOLNETFramework::Actor::GetComponentByID;
				Shared::ActorFunctions[head++] = (void*)&UnrealSOLNETFramework::Actor::GetRootComponent;
				Shared::ActorFunctions[head++] = (void*)&UnrealSOLNETFramework::Actor::GetInputComponent;
				Shared::ActorFunctions[head++] = (void*)&UnrealSOLNETFramework::Actor::GetCreationTime;
				Shared::ActorFunctions[head++] = (void*)&UnrealSOLNETFramework::Actor::GetBlockInput;
				Shared::ActorFunctions[head++] = (void*)&UnrealSOLNETFramework::Actor::GetDistanceTo;
				Shared::ActorFunctions[head++] = (void*)&UnrealSOLNETFramework::Actor::GetHorizontalDistanceTo;
				Shared::ActorFunctions[head++] = (void*)&UnrealSOLNETFramework::Actor::GetBounds;
				Shared::ActorFunctions[head++] = (void*)&UnrealSOLNETFramework::Actor::GetEyesViewPoint;
				Shared::ActorFunctions[head++] = (void*)&UnrealSOLNETFramework::Actor::SetRootComponent;
				Shared::ActorFunctions[head++] = (void*)&UnrealSOLNETFramework::Actor::SetInputComponent;
				Shared::ActorFunctions[head++] = (void*)&UnrealSOLNETFramework::Actor::SetBlockInput;
				Shared::ActorFunctions[head++] = (void*)&UnrealSOLNETFramework::Actor::SetLifeSpan;
				Shared::ActorFunctions[head++] = (void*)&UnrealSOLNETFramework::Actor::SetEnableInput;
				Shared::ActorFunctions[head++] = (void*)&UnrealSOLNETFramework::Actor::SetEnableCollision;
				Shared::ActorFunctions[head++] = (void*)&UnrealSOLNETFramework::Actor::AddTag;
				Shared::ActorFunctions[head++] = (void*)&UnrealSOLNETFramework::Actor::RemoveTag;
				Shared::ActorFunctions[head++] = (void*)&UnrealSOLNETFramework::Actor::HasTag;
				Shared::ActorFunctions[head++] = (void*)&UnrealSOLNETFramework::Actor::RegisterEvent;
				Shared::ActorFunctions[head++] = (void*)&UnrealSOLNETFramework::Actor::UnregisterEvent;

				checksum += head;
			}

			{
				int32 head = 0;
				Shared::Functions[position++] = Shared::GameModeBaseFunctions;

				Shared::GameModeBaseFunctions[head++] = (void*)&UnrealSOLNETFramework::GameModeBase::GetUseSeamlessTravel;
				Shared::GameModeBaseFunctions[head++] = (void*)&UnrealSOLNETFramework::GameModeBase::SetUseSeamlessTravel;
				Shared::GameModeBaseFunctions[head++] = (void*)&UnrealSOLNETFramework::GameModeBase::SwapPlayerControllers;

				checksum += head;
			}

			{
				int32 head = 0;
				Shared::Functions[position++] = Shared::PawnFunctions;

				Shared::PawnFunctions[head++] = (void*)&UnrealSOLNETFramework::Pawn::IsControlled;
				Shared::PawnFunctions[head++] = (void*)&UnrealSOLNETFramework::Pawn::IsPlayerControlled;
				Shared::PawnFunctions[head++] = (void*)&UnrealSOLNETFramework::Pawn::GetAutoPossessAI;
				Shared::PawnFunctions[head++] = (void*)&UnrealSOLNETFramework::Pawn::GetAutoPossessPlayer;
				Shared::PawnFunctions[head++] = (void*)&UnrealSOLNETFramework::Pawn::GetUseControllerRotationYaw;
				Shared::PawnFunctions[head++] = (void*)&UnrealSOLNETFramework::Pawn::GetUseControllerRotationPitch;
				Shared::PawnFunctions[head++] = (void*)&UnrealSOLNETFramework::Pawn::GetUseControllerRotationRoll;
				Shared::PawnFunctions[head++] = (void*)&UnrealSOLNETFramework::Pawn::GetGravityDirection;
				Shared::PawnFunctions[head++] = (void*)&UnrealSOLNETFramework::Pawn::GetAIController;
				Shared::PawnFunctions[head++] = (void*)&UnrealSOLNETFramework::Pawn::GetPlayerController;
				Shared::PawnFunctions[head++] = (void*)&UnrealSOLNETFramework::Pawn::SetAutoPossessAI;
				Shared::PawnFunctions[head++] = (void*)&UnrealSOLNETFramework::Pawn::SetAutoPossessPlayer;
				Shared::PawnFunctions[head++] = (void*)&UnrealSOLNETFramework::Pawn::SetUseControllerRotationYaw;
				Shared::PawnFunctions[head++] = (void*)&UnrealSOLNETFramework::Pawn::SetUseControllerRotationPitch;
				Shared::PawnFunctions[head++] = (void*)&UnrealSOLNETFramework::Pawn::SetUseControllerRotationRoll;
				Shared::PawnFunctions[head++] = (void*)&UnrealSOLNETFramework::Pawn::AddControllerYawInput;
				Shared::PawnFunctions[head++] = (void*)&UnrealSOLNETFramework::Pawn::AddControllerPitchInput;
				Shared::PawnFunctions[head++] = (void*)&UnrealSOLNETFramework::Pawn::AddControllerRollInput;
				Shared::PawnFunctions[head++] = (void*)&UnrealSOLNETFramework::Pawn::AddMovementInput;

				checksum += head;
			}

			{
				int32 head = 0;
				Shared::Functions[position++] = Shared::CharacterFunctions;

				Shared::CharacterFunctions[head++] = (void*)&UnrealSOLNETFramework::Character::IsCrouched;
				Shared::CharacterFunctions[head++] = (void*)&UnrealSOLNETFramework::Character::CanCrouch;
				Shared::CharacterFunctions[head++] = (void*)&UnrealSOLNETFramework::Character::CanJump;
				Shared::CharacterFunctions[head++] = (void*)&UnrealSOLNETFramework::Character::CheckJumpInput;
				Shared::CharacterFunctions[head++] = (void*)&UnrealSOLNETFramework::Character::ClearJumpInput;
				Shared::CharacterFunctions[head++] = (void*)&UnrealSOLNETFramework::Character::Launch;
				Shared::CharacterFunctions[head++] = (void*)&UnrealSOLNETFramework::Character::Crouch;
				Shared::CharacterFunctions[head++] = (void*)&UnrealSOLNETFramework::Character::StopCrouching;
				Shared::CharacterFunctions[head++] = (void*)&UnrealSOLNETFramework::Character::Jump;
				Shared::CharacterFunctions[head++] = (void*)&UnrealSOLNETFramework::Character::StopJumping;
				Shared::CharacterFunctions[head++] = (void*)&UnrealSOLNETFramework::Character::SetOnLandedCallback;

				checksum += head;
			}

			{
				int32 head = 0;
				Shared::Functions[position++] = Shared::ControllerFunctions;

				Shared::ControllerFunctions[head++] = (void*)&UnrealSOLNETFramework::Controller::IsLookInputIgnored;
				Shared::ControllerFunctions[head++] = (void*)&UnrealSOLNETFramework::Controller::IsMoveInputIgnored;
				Shared::ControllerFunctions[head++] = (void*)&UnrealSOLNETFramework::Controller::IsPlayerController;
				Shared::ControllerFunctions[head++] = (void*)&UnrealSOLNETFramework::Controller::GetPawn;
				Shared::ControllerFunctions[head++] = (void*)&UnrealSOLNETFramework::Controller::GetCharacter;
				Shared::ControllerFunctions[head++] = (void*)&UnrealSOLNETFramework::Controller::GetViewTarget;
				Shared::ControllerFunctions[head++] = (void*)&UnrealSOLNETFramework::Controller::GetControlRotation;
				Shared::ControllerFunctions[head++] = (void*)&UnrealSOLNETFramework::Controller::GetDesiredRotation;
				Shared::ControllerFunctions[head++] = (void*)&UnrealSOLNETFramework::Controller::LineOfSightTo;
				Shared::ControllerFunctions[head++] = (void*)&UnrealSOLNETFramework::Controller::SetControlRotation;
				Shared::ControllerFunctions[head++] = (void*)&UnrealSOLNETFramework::Controller::SetInitialLocationAndRotation;
				Shared::ControllerFunctions[head++] = (void*)&UnrealSOLNETFramework::Controller::SetIgnoreLookInput;
				Shared::ControllerFunctions[head++] = (void*)&UnrealSOLNETFramework::Controller::SetIgnoreMoveInput;
				Shared::ControllerFunctions[head++] = (void*)&UnrealSOLNETFramework::Controller::ResetIgnoreLookInput;
				Shared::ControllerFunctions[head++] = (void*)&UnrealSOLNETFramework::Controller::ResetIgnoreMoveInput;
				Shared::ControllerFunctions[head++] = (void*)&UnrealSOLNETFramework::Controller::Possess;
				Shared::ControllerFunctions[head++] = (void*)&UnrealSOLNETFramework::Controller::Unpossess;

				checksum += head;
			}

			{
				int32 head = 0;
				Shared::Functions[position++] = Shared::AIControllerFunctions;

				Shared::AIControllerFunctions[head++] = (void*)&UnrealSOLNETFramework::AIController::ClearFocus;
				Shared::AIControllerFunctions[head++] = (void*)&UnrealSOLNETFramework::AIController::GetFocalPoint;
				Shared::AIControllerFunctions[head++] = (void*)&UnrealSOLNETFramework::AIController::SetFocalPoint;
				Shared::AIControllerFunctions[head++] = (void*)&UnrealSOLNETFramework::AIController::GetFocusActor;
				Shared::AIControllerFunctions[head++] = (void*)&UnrealSOLNETFramework::AIController::GetAllowStrafe;
				Shared::AIControllerFunctions[head++] = (void*)&UnrealSOLNETFramework::AIController::SetAllowStrafe;
				Shared::AIControllerFunctions[head++] = (void*)&UnrealSOLNETFramework::AIController::SetFocus;

				checksum += head;
			}

			{
				int32 head = 0;
				Shared::Functions[position++] = Shared::PlayerControllerFunctions;

				Shared::PlayerControllerFunctions[head++] = (void*)&UnrealSOLNETFramework::PlayerController::IsPaused;
				Shared::PlayerControllerFunctions[head++] = (void*)&UnrealSOLNETFramework::PlayerController::GetShowMouseCursor;
				Shared::PlayerControllerFunctions[head++] = (void*)&UnrealSOLNETFramework::PlayerController::GetEnableClickEvents;
				Shared::PlayerControllerFunctions[head++] = (void*)&UnrealSOLNETFramework::PlayerController::GetEnableMouseOverEvents;
				Shared::PlayerControllerFunctions[head++] = (void*)&UnrealSOLNETFramework::PlayerController::GetMousePosition;
				Shared::PlayerControllerFunctions[head++] = (void*)&UnrealSOLNETFramework::PlayerController::GetPlayer;
				Shared::PlayerControllerFunctions[head++] = (void*)&UnrealSOLNETFramework::PlayerController::GetPlayerInput;
				Shared::PlayerControllerFunctions[head++] = (void*)&UnrealSOLNETFramework::PlayerController::GetHitResultAtScreenPosition;
				Shared::PlayerControllerFunctions[head++] = (void*)&UnrealSOLNETFramework::PlayerController::GetHitResultUnderCursor;
				Shared::PlayerControllerFunctions[head++] = (void*)&UnrealSOLNETFramework::PlayerController::SetShowMouseCursor;
				Shared::PlayerControllerFunctions[head++] = (void*)&UnrealSOLNETFramework::PlayerController::SetEnableClickEvents;
				Shared::PlayerControllerFunctions[head++] = (void*)&UnrealSOLNETFramework::PlayerController::SetEnableMouseOverEvents;
				Shared::PlayerControllerFunctions[head++] = (void*)&UnrealSOLNETFramework::PlayerController::SetMousePosition;
				Shared::PlayerControllerFunctions[head++] = (void*)&UnrealSOLNETFramework::PlayerController::ConsoleCommand;
				Shared::PlayerControllerFunctions[head++] = (void*)&UnrealSOLNETFramework::PlayerController::SetPause;
				Shared::PlayerControllerFunctions[head++] = (void*)&UnrealSOLNETFramework::PlayerController::SetViewTarget;
				Shared::PlayerControllerFunctions[head++] = (void*)&UnrealSOLNETFramework::PlayerController::SetViewTargetWithBlend;
				Shared::PlayerControllerFunctions[head++] = (void*)&UnrealSOLNETFramework::PlayerController::AddYawInput;
				Shared::PlayerControllerFunctions[head++] = (void*)&UnrealSOLNETFramework::PlayerController::AddPitchInput;
				Shared::PlayerControllerFunctions[head++] = (void*)&UnrealSOLNETFramework::PlayerController::AddRollInput;

				checksum += head;
			}

			{
				int32 head = 0;
				Shared::Functions[position++] = Shared::VolumeFunctions;

				Shared::VolumeFunctions[head++] = (void*)&UnrealSOLNETFramework::Volume::EncompassesPoint;

				checksum += head;
			}

			{
				int32 head = 0;
				Shared::Functions[position++] = Shared::PostProcessVolumeFunctions;

				Shared::PostProcessVolumeFunctions[head++] = (void*)&UnrealSOLNETFramework::PostProcessVolume::GetEnabled;
				Shared::PostProcessVolumeFunctions[head++] = (void*)&UnrealSOLNETFramework::PostProcessVolume::GetBlendRadius;
				Shared::PostProcessVolumeFunctions[head++] = (void*)&UnrealSOLNETFramework::PostProcessVolume::GetBlendWeight;
				Shared::PostProcessVolumeFunctions[head++] = (void*)&UnrealSOLNETFramework::PostProcessVolume::GetUnbound;
				Shared::PostProcessVolumeFunctions[head++] = (void*)&UnrealSOLNETFramework::PostProcessVolume::GetPriority;
				Shared::PostProcessVolumeFunctions[head++] = (void*)&UnrealSOLNETFramework::PostProcessVolume::SetEnabled;
				Shared::PostProcessVolumeFunctions[head++] = (void*)&UnrealSOLNETFramework::PostProcessVolume::SetBlendRadius;
				Shared::PostProcessVolumeFunctions[head++] = (void*)&UnrealSOLNETFramework::PostProcessVolume::SetBlendWeight;
				Shared::PostProcessVolumeFunctions[head++] = (void*)&UnrealSOLNETFramework::PostProcessVolume::SetUnbound;
				Shared::PostProcessVolumeFunctions[head++] = (void*)&UnrealSOLNETFramework::PostProcessVolume::SetPriority;

				checksum += head;
			}

			{
				int32 head = 0;
				Shared::Functions[position++] = Shared::SoundBaseFunctions;

				Shared::SoundBaseFunctions[head++] = (void*)&UnrealSOLNETFramework::SoundBase::GetDuration;

				checksum += head;
			}

			{
				int32 head = 0;
				Shared::Functions[position++] = Shared::SoundWaveFunctions;

				Shared::SoundWaveFunctions[head++] = (void*)&UnrealSOLNETFramework::SoundWave::GetLoop;
				Shared::SoundWaveFunctions[head++] = (void*)&UnrealSOLNETFramework::SoundWave::SetLoop;

				checksum += head;
			}

			{
				int32 head = 0;
				Shared::Functions[position++] = Shared::AnimationInstanceFunctions;

				Shared::AnimationInstanceFunctions[head++] = (void*)&UnrealSOLNETFramework::AnimationInstance::GetCurrentActiveMontage;
				Shared::AnimationInstanceFunctions[head++] = (void*)&UnrealSOLNETFramework::AnimationInstance::IsPlaying;
				Shared::AnimationInstanceFunctions[head++] = (void*)&UnrealSOLNETFramework::AnimationInstance::GetPlayRate;
				Shared::AnimationInstanceFunctions[head++] = (void*)&UnrealSOLNETFramework::AnimationInstance::GetPosition;
				Shared::AnimationInstanceFunctions[head++] = (void*)&UnrealSOLNETFramework::AnimationInstance::GetBlendTime;
				Shared::AnimationInstanceFunctions[head++] = (void*)&UnrealSOLNETFramework::AnimationInstance::GetCurrentSection;
				Shared::AnimationInstanceFunctions[head++] = (void*)&UnrealSOLNETFramework::AnimationInstance::SetPlayRate;
				Shared::AnimationInstanceFunctions[head++] = (void*)&UnrealSOLNETFramework::AnimationInstance::SetPosition;
				Shared::AnimationInstanceFunctions[head++] = (void*)&UnrealSOLNETFramework::AnimationInstance::SetNextSection;
				Shared::AnimationInstanceFunctions[head++] = (void*)&UnrealSOLNETFramework::AnimationInstance::PlayMontage;
				Shared::AnimationInstanceFunctions[head++] = (void*)&UnrealSOLNETFramework::AnimationInstance::PauseMontage;
				Shared::AnimationInstanceFunctions[head++] = (void*)&UnrealSOLNETFramework::AnimationInstance::ResumeMontage;
				Shared::AnimationInstanceFunctions[head++] = (void*)&UnrealSOLNETFramework::AnimationInstance::StopMontage;
				Shared::AnimationInstanceFunctions[head++] = (void*)&UnrealSOLNETFramework::AnimationInstance::JumpToSection;
				Shared::AnimationInstanceFunctions[head++] = (void*)&UnrealSOLNETFramework::AnimationInstance::JumpToSectionsEnd;

				checksum += head;
			}

			{
				int32 head = 0;
				Shared::Functions[position++] = Shared::PlayerFunctions;

				Shared::PlayerFunctions[head++] = (void*)&UnrealSOLNETFramework::Player::GetPlayerController;

				checksum += head;
			}

			{
				int32 head = 0;
				Shared::Functions[position++] = Shared::PlayerInputFunctions;

				Shared::PlayerInputFunctions[head++] = (void*)&UnrealSOLNETFramework::PlayerInput::IsKeyPressed;
				Shared::PlayerInputFunctions[head++] = (void*)&UnrealSOLNETFramework::PlayerInput::GetTimeKeyPressed;
				Shared::PlayerInputFunctions[head++] = (void*)&UnrealSOLNETFramework::PlayerInput::GetMouseSensitivity;
				Shared::PlayerInputFunctions[head++] = (void*)&UnrealSOLNETFramework::PlayerInput::SetMouseSensitivity;
				Shared::PlayerInputFunctions[head++] = (void*)&UnrealSOLNETFramework::PlayerInput::AddActionMapping;
				Shared::PlayerInputFunctions[head++] = (void*)&UnrealSOLNETFramework::PlayerInput::AddAxisMapping;
				Shared::PlayerInputFunctions[head++] = (void*)&UnrealSOLNETFramework::PlayerInput::RemoveActionMapping;
				Shared::PlayerInputFunctions[head++] = (void*)&UnrealSOLNETFramework::PlayerInput::RemoveAxisMapping;

				checksum += head;
			}

			{
				int32 head = 0;
				Shared::Functions[position++] = Shared::FontFunctions;

				Shared::FontFunctions[head++] = (void*)&UnrealSOLNETFramework::Font::GetStringSize;

				checksum += head;
			}

			{
				int32 head = 0;
				Shared::Functions[position++] = Shared::Texture2DFunctions;

				Shared::Texture2DFunctions[head++] = (void*)&UnrealSOLNETFramework::Texture2D::CreateFromFile;
				Shared::Texture2DFunctions[head++] = (void*)&UnrealSOLNETFramework::Texture2D::CreateFromBuffer;
				Shared::Texture2DFunctions[head++] = (void*)&UnrealSOLNETFramework::Texture2D::HasAlphaChannel;
				Shared::Texture2DFunctions[head++] = (void*)&UnrealSOLNETFramework::Texture2D::GetSize;
				Shared::Texture2DFunctions[head++] = (void*)&UnrealSOLNETFramework::Texture2D::GetPixelFormat;

				checksum += head;
			}

			{
				int32 head = 0;
				Shared::Functions[position++] = Shared::ActorComponentFunctions;

				Shared::ActorComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::ActorComponent::IsOwnerSelected;
				Shared::ActorComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::ActorComponent::GetOwner;
				Shared::ActorComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::ActorComponent::Destroy;
				Shared::ActorComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::ActorComponent::AddTag;
				Shared::ActorComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::ActorComponent::RemoveTag;
				Shared::ActorComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::ActorComponent::HasTag;

				checksum += head;
			}

			{
				int32 head = 0;
				Shared::Functions[position++] = Shared::InputComponentFunctions;

				Shared::InputComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::InputComponent::HasBindings;
				Shared::InputComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::InputComponent::GetActionBindingsNumber;
				Shared::InputComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::InputComponent::ClearActionBindings;
				Shared::InputComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::InputComponent::BindAction;
				Shared::InputComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::InputComponent::BindAxis;
				Shared::InputComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::InputComponent::RemoveActionBinding;
				Shared::InputComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::InputComponent::GetBlockInput;
				Shared::InputComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::InputComponent::SetBlockInput;
				Shared::InputComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::InputComponent::GetPriority;
				Shared::InputComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::InputComponent::SetPriority;

				checksum += head;
			}

			{
				int32 head = 0;
				Shared::Functions[position++] = Shared::MovementComponentFunctions;

				Shared::MovementComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::MovementComponent::GetConstrainToPlane;
				Shared::MovementComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::MovementComponent::GetSnapToPlaneAtStart;
				Shared::MovementComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::MovementComponent::GetVelocity;
				Shared::MovementComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::MovementComponent::GetPlaneConstraint;
				Shared::MovementComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::MovementComponent::GetUpdateOnlyIfRendered;
				Shared::MovementComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::MovementComponent::GetPlaneConstraintNormal;
				Shared::MovementComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::MovementComponent::GetPlaneConstraintOrigin;
				Shared::MovementComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::MovementComponent::GetGravity;
				Shared::MovementComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::MovementComponent::GetMaxSpeed;
				Shared::MovementComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::MovementComponent::SetConstrainToPlane;
				Shared::MovementComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::MovementComponent::SetSnapToPlaneAtStart;
				Shared::MovementComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::MovementComponent::SetUpdateOnlyIfRendered;
				Shared::MovementComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::MovementComponent::SetVelocity;
				Shared::MovementComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::MovementComponent::SetPlaneConstraint;
				Shared::MovementComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::MovementComponent::SetPlaneConstraintNormal;
				Shared::MovementComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::MovementComponent::SetPlaneConstraintOrigin;
				Shared::MovementComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::MovementComponent::SetPlaneConstraintFromVectors;
				Shared::MovementComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::MovementComponent::IsExceedingMaxSpeed;
				Shared::MovementComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::MovementComponent::IsInWater;
				Shared::MovementComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::MovementComponent::StopMovement;
				Shared::MovementComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::MovementComponent::ConstrainDirectionToPlane;
				Shared::MovementComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::MovementComponent::ConstrainLocationToPlane;
				Shared::MovementComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::MovementComponent::ConstrainNormalToPlane;

				checksum += head;
			}

			{
				int32 head = 0;
				Shared::Functions[position++] = Shared::RotatingMovementComponentFunctions;

				Shared::RotatingMovementComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::RotatingMovementComponent::Create;
				Shared::RotatingMovementComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::RotatingMovementComponent::GetRotationInLocalSpace;
				Shared::RotatingMovementComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::RotatingMovementComponent::GetPivotTranslation;
				Shared::RotatingMovementComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::RotatingMovementComponent::GetRotationRate;
				Shared::RotatingMovementComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::RotatingMovementComponent::SetRotationInLocalSpace;
				Shared::RotatingMovementComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::RotatingMovementComponent::SetPivotTranslation;
				Shared::RotatingMovementComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::RotatingMovementComponent::SetRotationRate;

				checksum += head;
			}

			{
				int32 head = 0;
				Shared::Functions[position++] = Shared::SceneComponentFunctions;

				Shared::SceneComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SceneComponent::IsAttachedToComponent;
				Shared::SceneComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SceneComponent::IsAttachedToActor;
				Shared::SceneComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SceneComponent::IsVisible;
				Shared::SceneComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SceneComponent::IsSocketExists;
				Shared::SceneComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SceneComponent::HasAnySockets;
				Shared::SceneComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SceneComponent::CanAttachAsChild;
				Shared::SceneComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SceneComponent::ForEachAttachedChild;
				Shared::SceneComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SceneComponent::Create;
				Shared::SceneComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SceneComponent::AttachToComponent;
				Shared::SceneComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SceneComponent::DetachFromComponent;
				Shared::SceneComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SceneComponent::Activate;
				Shared::SceneComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SceneComponent::Deactivate;
				Shared::SceneComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SceneComponent::UpdateToWorld;
				Shared::SceneComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SceneComponent::AddLocalOffset;
				Shared::SceneComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SceneComponent::AddLocalRotation;
				Shared::SceneComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SceneComponent::AddRelativeLocation;
				Shared::SceneComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SceneComponent::AddRelativeRotation;
				Shared::SceneComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SceneComponent::AddLocalTransform;
				Shared::SceneComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SceneComponent::AddWorldOffset;
				Shared::SceneComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SceneComponent::AddWorldRotation;
				Shared::SceneComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SceneComponent::AddWorldTransform;
				Shared::SceneComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SceneComponent::GetAttachedSocketName;
				Shared::SceneComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SceneComponent::GetBounds;
				Shared::SceneComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SceneComponent::GetSocketLocation;
				Shared::SceneComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SceneComponent::GetSocketRotation;
				Shared::SceneComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SceneComponent::GetComponentVelocity;
				Shared::SceneComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SceneComponent::GetComponentLocation;
				Shared::SceneComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SceneComponent::GetComponentRotation;
				Shared::SceneComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SceneComponent::GetComponentScale;
				Shared::SceneComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SceneComponent::GetComponentTransform;
				Shared::SceneComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SceneComponent::GetForwardVector;
				Shared::SceneComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SceneComponent::GetRightVector;
				Shared::SceneComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SceneComponent::GetUpVector;
				Shared::SceneComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SceneComponent::SetMobility;
				Shared::SceneComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SceneComponent::SetVisibility;
				Shared::SceneComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SceneComponent::SetRelativeLocation;
				Shared::SceneComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SceneComponent::SetRelativeRotation;
				Shared::SceneComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SceneComponent::SetRelativeTransform;
				Shared::SceneComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SceneComponent::SetWorldLocation;
				Shared::SceneComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SceneComponent::SetWorldRotation;
				Shared::SceneComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SceneComponent::SetWorldScale;
				Shared::SceneComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SceneComponent::SetWorldTransform;

				checksum += head;
			}

			{
				int32 head = 0;
				Shared::Functions[position++] = Shared::AudioComponentFunctions;

				Shared::AudioComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::AudioComponent::IsPlaying;
				Shared::AudioComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::AudioComponent::GetPaused;
				Shared::AudioComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::AudioComponent::SetSound;
				Shared::AudioComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::AudioComponent::SetPaused;
				Shared::AudioComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::AudioComponent::Play;
				Shared::AudioComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::AudioComponent::Stop;
				Shared::AudioComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::AudioComponent::FadeIn;
				Shared::AudioComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::AudioComponent::FadeOut;

				checksum += head;
			}

			{
				int32 head = 0;
				Shared::Functions[position++] = Shared::CameraComponentFunctions;

				Shared::CameraComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::CameraComponent::GetConstrainAspectRatio;
				Shared::CameraComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::CameraComponent::GetAspectRatio;
				Shared::CameraComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::CameraComponent::GetFieldOfView;
				Shared::CameraComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::CameraComponent::GetOrthoFarClipPlane;
				Shared::CameraComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::CameraComponent::GetOrthoNearClipPlane;
				Shared::CameraComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::CameraComponent::GetOrthoWidth;
				Shared::CameraComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::CameraComponent::GetLockToHeadMountedDisplay;
				Shared::CameraComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::CameraComponent::SetProjectionMode;
				Shared::CameraComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::CameraComponent::SetConstrainAspectRatio;
				Shared::CameraComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::CameraComponent::SetAspectRatio;
				Shared::CameraComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::CameraComponent::SetFieldOfView;
				Shared::CameraComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::CameraComponent::SetOrthoFarClipPlane;
				Shared::CameraComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::CameraComponent::SetOrthoNearClipPlane;
				Shared::CameraComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::CameraComponent::SetOrthoWidth;
				Shared::CameraComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::CameraComponent::SetLockToHeadMountedDisplay;

				checksum += head;
			}

			{
				int32 head = 0;
				Shared::Functions[position++] = Shared::ChildActorComponentFunctions;

				Shared::ChildActorComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::ChildActorComponent::GetChildActor;
				Shared::ChildActorComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::ChildActorComponent::SetChildActor;

				checksum += head;
			}

			{
				int32 head = 0;
				Shared::Functions[position++] = Shared::SpringArmComponentFunctions;

				Shared::SpringArmComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SpringArmComponent::IsCollisionFixApplied;
				Shared::SpringArmComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SpringArmComponent::GetDrawDebugLagMarkers;
				Shared::SpringArmComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SpringArmComponent::GetCollisionTest;
				Shared::SpringArmComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SpringArmComponent::GetCameraPositionLag;
				Shared::SpringArmComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SpringArmComponent::GetCameraRotationLag;
				Shared::SpringArmComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SpringArmComponent::GetCameraLagSubstepping;
				Shared::SpringArmComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SpringArmComponent::GetInheritPitch;
				Shared::SpringArmComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SpringArmComponent::GetInheritRoll;
				Shared::SpringArmComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SpringArmComponent::GetInheritYaw;
				Shared::SpringArmComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SpringArmComponent::GetCameraLagMaxDistance;
				Shared::SpringArmComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SpringArmComponent::GetCameraLagMaxTimeStep;
				Shared::SpringArmComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SpringArmComponent::GetCameraPositionLagSpeed;
				Shared::SpringArmComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SpringArmComponent::GetCameraRotationLagSpeed;
				Shared::SpringArmComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SpringArmComponent::GetProbeChannel;
				Shared::SpringArmComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SpringArmComponent::GetProbeSize;
				Shared::SpringArmComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SpringArmComponent::GetSocketOffset;
				Shared::SpringArmComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SpringArmComponent::GetTargetArmLength;
				Shared::SpringArmComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SpringArmComponent::GetTargetOffset;
				Shared::SpringArmComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SpringArmComponent::GetUnfixedCameraPosition;
				Shared::SpringArmComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SpringArmComponent::GetDesiredRotation;
				Shared::SpringArmComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SpringArmComponent::GetTargetRotation;
				Shared::SpringArmComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SpringArmComponent::GetUsePawnControlRotation;
				Shared::SpringArmComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SpringArmComponent::SetDrawDebugLagMarkers;
				Shared::SpringArmComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SpringArmComponent::SetCollisionTest;
				Shared::SpringArmComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SpringArmComponent::SetCameraPositionLag;
				Shared::SpringArmComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SpringArmComponent::SetCameraRotationLag;
				Shared::SpringArmComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SpringArmComponent::SetCameraLagSubstepping;
				Shared::SpringArmComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SpringArmComponent::SetInheritPitch;
				Shared::SpringArmComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SpringArmComponent::SetInheritRoll;
				Shared::SpringArmComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SpringArmComponent::SetInheritYaw;
				Shared::SpringArmComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SpringArmComponent::SetCameraLagMaxDistance;
				Shared::SpringArmComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SpringArmComponent::SetCameraLagMaxTimeStep;
				Shared::SpringArmComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SpringArmComponent::SetCameraPositionLagSpeed;
				Shared::SpringArmComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SpringArmComponent::SetCameraRotationLagSpeed;
				Shared::SpringArmComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SpringArmComponent::SetProbeChannel;
				Shared::SpringArmComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SpringArmComponent::SetProbeSize;
				Shared::SpringArmComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SpringArmComponent::SetSocketOffset;
				Shared::SpringArmComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SpringArmComponent::SetTargetArmLength;
				Shared::SpringArmComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SpringArmComponent::SetTargetOffset;
				Shared::SpringArmComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SpringArmComponent::SetUsePawnControlRotation;

				checksum += head;
			}

			{
				int32 head = 0;
				Shared::Functions[position++] = Shared::PostProcessComponentFunctions;

				Shared::PostProcessComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PostProcessComponent::GetEnabled;
				Shared::PostProcessComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PostProcessComponent::GetBlendRadius;
				Shared::PostProcessComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PostProcessComponent::GetBlendWeight;
				Shared::PostProcessComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PostProcessComponent::GetUnbound;
				Shared::PostProcessComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PostProcessComponent::GetPriority;
				Shared::PostProcessComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PostProcessComponent::SetEnabled;
				Shared::PostProcessComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PostProcessComponent::SetBlendRadius;
				Shared::PostProcessComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PostProcessComponent::SetBlendWeight;
				Shared::PostProcessComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PostProcessComponent::SetUnbound;
				Shared::PostProcessComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PostProcessComponent::SetPriority;

				checksum += head;
			}

			{
				int32 head = 0;
				Shared::Functions[position++] = Shared::PrimitiveComponentFunctions;

				Shared::PrimitiveComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PrimitiveComponent::IsGravityEnabled;
				Shared::PrimitiveComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PrimitiveComponent::IsOverlappingComponent;
				Shared::PrimitiveComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PrimitiveComponent::ForEachOverlappingComponent;
				Shared::PrimitiveComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PrimitiveComponent::AddAngularImpulseInDegrees;
				Shared::PrimitiveComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PrimitiveComponent::AddAngularImpulseInRadians;
				Shared::PrimitiveComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PrimitiveComponent::AddForce;
				Shared::PrimitiveComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PrimitiveComponent::AddForceAtLocation;
				Shared::PrimitiveComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PrimitiveComponent::AddImpulse;
				Shared::PrimitiveComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PrimitiveComponent::AddImpulseAtLocation;
				Shared::PrimitiveComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PrimitiveComponent::AddRadialForce;
				Shared::PrimitiveComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PrimitiveComponent::AddRadialImpulse;
				Shared::PrimitiveComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PrimitiveComponent::AddTorqueInDegrees;
				Shared::PrimitiveComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PrimitiveComponent::AddTorqueInRadians;
				Shared::PrimitiveComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PrimitiveComponent::GetMass;
				Shared::PrimitiveComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PrimitiveComponent::GetPhysicsLinearVelocity;
				Shared::PrimitiveComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PrimitiveComponent::GetPhysicsLinearVelocityAtPoint;
				Shared::PrimitiveComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PrimitiveComponent::GetPhysicsAngularVelocityInDegrees;
				Shared::PrimitiveComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PrimitiveComponent::GetPhysicsAngularVelocityInRadians;
				Shared::PrimitiveComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PrimitiveComponent::GetCastShadow;
				Shared::PrimitiveComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PrimitiveComponent::GetOnlyOwnerSee;
				Shared::PrimitiveComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PrimitiveComponent::GetOwnerNoSee;
				Shared::PrimitiveComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PrimitiveComponent::GetIgnoreRadialForce;
				Shared::PrimitiveComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PrimitiveComponent::GetIgnoreRadialImpulse;
				Shared::PrimitiveComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PrimitiveComponent::GetMaterial;
				Shared::PrimitiveComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PrimitiveComponent::GetMaterialsNumber;
				Shared::PrimitiveComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PrimitiveComponent::GetDistanceToCollision;
				Shared::PrimitiveComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PrimitiveComponent::GetSquaredDistanceToCollision;
				Shared::PrimitiveComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PrimitiveComponent::GetAngularDamping;
				Shared::PrimitiveComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PrimitiveComponent::GetLinearDamping;
				Shared::PrimitiveComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PrimitiveComponent::SetGenerateOverlapEvents;
				Shared::PrimitiveComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PrimitiveComponent::SetGenerateHitEvents;
				Shared::PrimitiveComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PrimitiveComponent::SetMass;
				Shared::PrimitiveComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PrimitiveComponent::SetCenterOfMass;
				Shared::PrimitiveComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PrimitiveComponent::SetPhysicsLinearVelocity;
				Shared::PrimitiveComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PrimitiveComponent::SetPhysicsAngularVelocityInDegrees;
				Shared::PrimitiveComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PrimitiveComponent::SetPhysicsAngularVelocityInRadians;
				Shared::PrimitiveComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PrimitiveComponent::SetPhysicsMaxAngularVelocityInDegrees;
				Shared::PrimitiveComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PrimitiveComponent::SetPhysicsMaxAngularVelocityInRadians;
				Shared::PrimitiveComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PrimitiveComponent::SetCastShadow;
				Shared::PrimitiveComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PrimitiveComponent::SetOnlyOwnerSee;
				Shared::PrimitiveComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PrimitiveComponent::SetOwnerNoSee;
				Shared::PrimitiveComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PrimitiveComponent::SetIgnoreRadialForce;
				Shared::PrimitiveComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PrimitiveComponent::SetIgnoreRadialImpulse;
				Shared::PrimitiveComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PrimitiveComponent::SetMaterial;
				Shared::PrimitiveComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PrimitiveComponent::SetSimulatePhysics;
				Shared::PrimitiveComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PrimitiveComponent::SetAngularDamping;
				Shared::PrimitiveComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PrimitiveComponent::SetLinearDamping;
				Shared::PrimitiveComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PrimitiveComponent::SetEnableGravity;
				Shared::PrimitiveComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PrimitiveComponent::SetCollisionMode;
				Shared::PrimitiveComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PrimitiveComponent::SetCollisionChannel;
				Shared::PrimitiveComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PrimitiveComponent::SetCollisionProfileName;
				Shared::PrimitiveComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PrimitiveComponent::SetCollisionResponseToChannel;
				Shared::PrimitiveComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PrimitiveComponent::SetCollisionResponseToAllChannels;
				Shared::PrimitiveComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PrimitiveComponent::SetIgnoreActorWhenMoving;
				Shared::PrimitiveComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PrimitiveComponent::SetIgnoreComponentWhenMoving;
				Shared::PrimitiveComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PrimitiveComponent::ClearMoveIgnoreActors;
				Shared::PrimitiveComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PrimitiveComponent::ClearMoveIgnoreComponents;
				Shared::PrimitiveComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PrimitiveComponent::CreateAndSetMaterialInstanceDynamic;
				Shared::PrimitiveComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PrimitiveComponent::RegisterEvent;
				Shared::PrimitiveComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::PrimitiveComponent::UnregisterEvent;

				checksum += head;
			}

			{
				int32 head = 0;
				Shared::Functions[position++] = Shared::ShapeComponentFunctions;

				Shared::ShapeComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::ShapeComponent::GetDynamicObstacle;
				Shared::ShapeComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::ShapeComponent::GetShapeColor;
				Shared::ShapeComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::ShapeComponent::SetDynamicObstacle;
				Shared::ShapeComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::ShapeComponent::SetShapeColor;

				checksum += head;
			}

			{
				int32 head = 0;
				Shared::Functions[position++] = Shared::BoxComponentFunctions;

				Shared::BoxComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::BoxComponent::GetScaledBoxExtent;
				Shared::BoxComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::BoxComponent::GetUnscaledBoxExtent;
				Shared::BoxComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::BoxComponent::SetBoxExtent;
				Shared::BoxComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::BoxComponent::InitBoxExtent;

				checksum += head;
			}

			{
				int32 head = 0;
				Shared::Functions[position++] = Shared::SphereComponentFunctions;

				Shared::SphereComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SphereComponent::GetScaledSphereRadius;
				Shared::SphereComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SphereComponent::GetUnscaledSphereRadius;
				Shared::SphereComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SphereComponent::GetShapeScale;
				Shared::SphereComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SphereComponent::SetSphereRadius;
				Shared::SphereComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SphereComponent::InitSphereRadius;

				checksum += head;
			}

			{
				int32 head = 0;
				Shared::Functions[position++] = Shared::CapsuleComponentFunctions;

				Shared::CapsuleComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::CapsuleComponent::GetScaledCapsuleRadius;
				Shared::CapsuleComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::CapsuleComponent::GetUnscaledCapsuleRadius;
				Shared::CapsuleComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::CapsuleComponent::GetShapeScale;
				Shared::CapsuleComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::CapsuleComponent::GetScaledCapsuleSize;
				Shared::CapsuleComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::CapsuleComponent::GetUnscaledCapsuleSize;
				Shared::CapsuleComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::CapsuleComponent::SetCapsuleRadius;
				Shared::CapsuleComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::CapsuleComponent::SetCapsuleSize;
				Shared::CapsuleComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::CapsuleComponent::InitCapsuleSize;

				checksum += head;
			}

			{
				int32 head = 0;
				Shared::Functions[position++] = Shared::MeshComponentFunctions;

				Shared::MeshComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::MeshComponent::IsValidMaterialSlotName;
				Shared::MeshComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::MeshComponent::GetMaterialIndex;

				checksum += head;
			}

			{
				int32 head = 0;
				Shared::Functions[position++] = Shared::TextRenderComponentFunctions;

				Shared::TextRenderComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::TextRenderComponent::SetFont;
				Shared::TextRenderComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::TextRenderComponent::SetText;
				Shared::TextRenderComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::TextRenderComponent::SetTextMaterial;
				Shared::TextRenderComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::TextRenderComponent::SetTextRenderColor;
				Shared::TextRenderComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::TextRenderComponent::SetHorizontalAlignment;
				Shared::TextRenderComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::TextRenderComponent::SetHorizontalSpacingAdjustment;
				Shared::TextRenderComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::TextRenderComponent::SetVerticalAlignment;
				Shared::TextRenderComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::TextRenderComponent::SetVerticalSpacingAdjustment;
				Shared::TextRenderComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::TextRenderComponent::SetScale;
				Shared::TextRenderComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::TextRenderComponent::SetWorldSize;

				checksum += head;
			}

			{
				int32 head = 0;
				Shared::Functions[position++] = Shared::LightComponentBaseFunctions;

				Shared::LightComponentBaseFunctions[head++] = (void*)&UnrealSOLNETFramework::LightComponentBase::GetIntensity;
				Shared::LightComponentBaseFunctions[head++] = (void*)&UnrealSOLNETFramework::LightComponentBase::GetCastShadows;
				Shared::LightComponentBaseFunctions[head++] = (void*)&UnrealSOLNETFramework::LightComponentBase::SetCastShadows;

				checksum += head;
			}

			{
				int32 head = 0;
				Shared::Functions[position++] = Shared::LightComponentFunctions;

				Shared::LightComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::LightComponent::SetIntensity;
				Shared::LightComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::LightComponent::SetLightColor;

				checksum += head;
			}

			{
				int32 head = 0;
				Shared::Functions[position++] = Shared::MotionControllerComponentFunctions;

				Shared::MotionControllerComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::MotionControllerComponent::IsTracked;
				Shared::MotionControllerComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::MotionControllerComponent::GetDisplayDeviceModel;
				Shared::MotionControllerComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::MotionControllerComponent::GetDisableLowLatencyUpdate;
				Shared::MotionControllerComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::MotionControllerComponent::GetTrackingSource;
				Shared::MotionControllerComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::MotionControllerComponent::SetDisplayDeviceModel;
				Shared::MotionControllerComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::MotionControllerComponent::SetDisableLowLatencyUpdate;
				Shared::MotionControllerComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::MotionControllerComponent::SetTrackingSource;
				Shared::MotionControllerComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::MotionControllerComponent::SetTrackingMotionSource;
				Shared::MotionControllerComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::MotionControllerComponent::SetAssociatedPlayerIndex;
				Shared::MotionControllerComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::MotionControllerComponent::SetCustomDisplayMesh;
				Shared::MotionControllerComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::MotionControllerComponent::SetDisplayModelSource;

				checksum += head;
			}

			{
				int32 head = 0;
				Shared::Functions[position++] = Shared::StaticMeshComponentFunctions;

				Shared::StaticMeshComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::StaticMeshComponent::GetLocalBounds;
				Shared::StaticMeshComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::StaticMeshComponent::GetStaticMesh;
				Shared::StaticMeshComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::StaticMeshComponent::SetStaticMesh;

				checksum += head;
			}

			{
				int32 head = 0;
				Shared::Functions[position++] = Shared::InstancedStaticMeshComponentFunctions;

				Shared::InstancedStaticMeshComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::InstancedStaticMeshComponent::GetInstanceCount;
				Shared::InstancedStaticMeshComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::InstancedStaticMeshComponent::GetInstanceTransform;
				Shared::InstancedStaticMeshComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::InstancedStaticMeshComponent::AddInstance;
				Shared::InstancedStaticMeshComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::InstancedStaticMeshComponent::AddInstances;
				Shared::InstancedStaticMeshComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::InstancedStaticMeshComponent::UpdateInstanceTransform;
				Shared::InstancedStaticMeshComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::InstancedStaticMeshComponent::BatchUpdateInstanceTransforms;
				Shared::InstancedStaticMeshComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::InstancedStaticMeshComponent::RemoveInstance;
				Shared::InstancedStaticMeshComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::InstancedStaticMeshComponent::ClearInstances;

				checksum += head;
			}

			{
				int32 head = 0;
				Shared::Functions[position++] = Shared::HierarchicalInstancedStaticMeshComponentFunctions;

				Shared::HierarchicalInstancedStaticMeshComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::HierarchicalInstancedStaticMeshComponent::GetDisableCollision;
				Shared::HierarchicalInstancedStaticMeshComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::HierarchicalInstancedStaticMeshComponent::SetDisableCollision;

				checksum += head;
			}

			{
				int32 head = 0;
				Shared::Functions[position++] = Shared::SkinnedMeshComponentFunctions;

				Shared::SkinnedMeshComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SkinnedMeshComponent::GetBonesNumber;
				Shared::SkinnedMeshComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SkinnedMeshComponent::GetBoneIndex;
				Shared::SkinnedMeshComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SkinnedMeshComponent::GetBoneName;
				Shared::SkinnedMeshComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SkinnedMeshComponent::GetBoneTransform;
				Shared::SkinnedMeshComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SkinnedMeshComponent::SetSkeletalMesh;

				checksum += head;
			}

			{
				int32 head = 0;
				Shared::Functions[position++] = Shared::SkeletalMeshComponentFunctions;

				Shared::SkeletalMeshComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SkeletalMeshComponent::IsPlaying;
				Shared::SkeletalMeshComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SkeletalMeshComponent::GetAnimationInstance;
				Shared::SkeletalMeshComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SkeletalMeshComponent::SetAnimation;
				Shared::SkeletalMeshComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SkeletalMeshComponent::SetAnimationMode;
				Shared::SkeletalMeshComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SkeletalMeshComponent::SetAnimationBlueprint;
				Shared::SkeletalMeshComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SkeletalMeshComponent::Play;
				Shared::SkeletalMeshComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SkeletalMeshComponent::PlayAnimation;
				Shared::SkeletalMeshComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SkeletalMeshComponent::Stop;

				checksum += head;
			}

			{
				int32 head = 0;
				Shared::Functions[position++] = Shared::SplineComponentFunctions;

				Shared::SplineComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SplineComponent::IsClosedLoop;
				Shared::SplineComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SplineComponent::GetDuration;
				Shared::SplineComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SplineComponent::GetSplinePointType;
				Shared::SplineComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SplineComponent::GetSplinePointsNumber;
				Shared::SplineComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SplineComponent::GetSplineSegmentsNumber;
				Shared::SplineComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SplineComponent::GetTangentAtDistanceAlongSpline;
				Shared::SplineComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SplineComponent::GetTangentAtSplinePoint;
				Shared::SplineComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SplineComponent::GetTangentAtTime;
				Shared::SplineComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SplineComponent::GetTransformAtDistanceAlongSpline;
				Shared::SplineComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SplineComponent::GetTransformAtSplinePoint;
				Shared::SplineComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SplineComponent::GetArriveTangentAtSplinePoint;
				Shared::SplineComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SplineComponent::GetDefaultUpVector;
				Shared::SplineComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SplineComponent::GetDirectionAtDistanceAlongSpline;
				Shared::SplineComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SplineComponent::GetDirectionAtSplinePoint;
				Shared::SplineComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SplineComponent::GetDirectionAtTime;
				Shared::SplineComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SplineComponent::GetDistanceAlongSplineAtSplinePoint;
				Shared::SplineComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SplineComponent::GetLeaveTangentAtSplinePoint;
				Shared::SplineComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SplineComponent::GetLocationAndTangentAtSplinePoint;
				Shared::SplineComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SplineComponent::GetLocationAtDistanceAlongSpline;
				Shared::SplineComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SplineComponent::GetLocationAtSplinePoint;
				Shared::SplineComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SplineComponent::GetLocationAtTime;
				Shared::SplineComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SplineComponent::GetRightVectorAtDistanceAlongSpline;
				Shared::SplineComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SplineComponent::GetRightVectorAtSplinePoint;
				Shared::SplineComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SplineComponent::GetRightVectorAtTime;
				Shared::SplineComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SplineComponent::GetRollAtDistanceAlongSpline;
				Shared::SplineComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SplineComponent::GetRollAtSplinePoint;
				Shared::SplineComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SplineComponent::GetRollAtTime;
				Shared::SplineComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SplineComponent::GetRotationAtDistanceAlongSpline;
				Shared::SplineComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SplineComponent::GetRotationAtSplinePoint;
				Shared::SplineComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SplineComponent::GetRotationAtTime;
				Shared::SplineComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SplineComponent::GetScaleAtDistanceAlongSpline;
				Shared::SplineComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SplineComponent::GetScaleAtSplinePoint;
				Shared::SplineComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SplineComponent::GetScaleAtTime;
				Shared::SplineComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SplineComponent::GetSplineLength;
				Shared::SplineComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SplineComponent::GetTransformAtTime;
				Shared::SplineComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SplineComponent::GetUpVectorAtDistanceAlongSpline;
				Shared::SplineComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SplineComponent::GetUpVectorAtSplinePoint;
				Shared::SplineComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SplineComponent::GetUpVectorAtTime;
				Shared::SplineComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SplineComponent::SetDuration;
				Shared::SplineComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SplineComponent::SetSplinePointType;
				Shared::SplineComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SplineComponent::SetClosedLoop;
				Shared::SplineComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SplineComponent::SetDefaultUpVector;
				Shared::SplineComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SplineComponent::SetLocationAtSplinePoint;
				Shared::SplineComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SplineComponent::SetTangentAtSplinePoint;
				Shared::SplineComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SplineComponent::SetTangentsAtSplinePoint;
				Shared::SplineComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SplineComponent::SetUpVectorAtSplinePoint;
				Shared::SplineComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SplineComponent::AddSplinePoint;
				Shared::SplineComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SplineComponent::AddSplinePointAtIndex;
				Shared::SplineComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SplineComponent::ClearSplinePoints;
				Shared::SplineComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SplineComponent::FindDirectionClosestToWorldLocation;
				Shared::SplineComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SplineComponent::FindLocationClosestToWorldLocation;
				Shared::SplineComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SplineComponent::FindUpVectorClosestToWorldLocation;
				Shared::SplineComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SplineComponent::FindRightVectorClosestToWorldLocation;
				Shared::SplineComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SplineComponent::FindRollClosestToWorldLocation;
				Shared::SplineComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SplineComponent::FindScaleClosestToWorldLocation;
				Shared::SplineComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SplineComponent::FindTangentClosestToWorldLocation;
				Shared::SplineComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SplineComponent::FindTransformClosestToWorldLocation;
				Shared::SplineComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SplineComponent::RemoveSplinePoint;
				Shared::SplineComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::SplineComponent::UpdateSpline;

				checksum += head;
			}

			{
				int32 head = 0;
				Shared::Functions[position++] = Shared::RadialForceComponentFunctions;

				Shared::RadialForceComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::RadialForceComponent::GetIgnoreOwningActor;
				Shared::RadialForceComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::RadialForceComponent::GetImpulseVelocityChange;
				Shared::RadialForceComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::RadialForceComponent::GetLinearFalloff;
				Shared::RadialForceComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::RadialForceComponent::GetForceStrength;
				Shared::RadialForceComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::RadialForceComponent::GetImpulseStrength;
				Shared::RadialForceComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::RadialForceComponent::GetRadius;
				Shared::RadialForceComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::RadialForceComponent::SetIgnoreOwningActor;
				Shared::RadialForceComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::RadialForceComponent::SetImpulseVelocityChange;
				Shared::RadialForceComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::RadialForceComponent::SetLinearFalloff;
				Shared::RadialForceComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::RadialForceComponent::SetForceStrength;
				Shared::RadialForceComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::RadialForceComponent::SetImpulseStrength;
				Shared::RadialForceComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::RadialForceComponent::SetRadius;
				Shared::RadialForceComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::RadialForceComponent::AddCollisionChannelToAffect;
				Shared::RadialForceComponentFunctions[head++] = (void*)&UnrealSOLNETFramework::RadialForceComponent::FireImpulse;

				checksum += head;
			}

			{
				int32 head = 0;
				Shared::Functions[position++] = Shared::MaterialInterfaceFunctions;

				Shared::MaterialInterfaceFunctions[head++] = (void*)&UnrealSOLNETFramework::MaterialInterface::IsTwoSided;

				checksum += head;
			}

			{
				int32 head = 0;
				Shared::Functions[position++] = Shared::MaterialFunctions;

				Shared::MaterialFunctions[head++] = (void*)&UnrealSOLNETFramework::Material::IsDefaultMaterial;

				checksum += head;
			}

			{
				int32 head = 0;
				Shared::Functions[position++] = Shared::MaterialInstanceFunctions;

				Shared::MaterialInstanceFunctions[head++] = (void*)&UnrealSOLNETFramework::MaterialInstance::IsChildOf;
				Shared::MaterialInstanceFunctions[head++] = (void*)&UnrealSOLNETFramework::MaterialInstance::GetParent;

				checksum += head;
			}

			{
				int32 head = 0;
				Shared::Functions[position++] = Shared::MaterialInstanceDynamicFunctions;

				Shared::MaterialInstanceDynamicFunctions[head++] = (void*)&UnrealSOLNETFramework::MaterialInstanceDynamic::ClearParameterValues;
				Shared::MaterialInstanceDynamicFunctions[head++] = (void*)&UnrealSOLNETFramework::MaterialInstanceDynamic::SetTextureParameterValue;
				Shared::MaterialInstanceDynamicFunctions[head++] = (void*)&UnrealSOLNETFramework::MaterialInstanceDynamic::SetVectorParameterValue;
				Shared::MaterialInstanceDynamicFunctions[head++] = (void*)&UnrealSOLNETFramework::MaterialInstanceDynamic::SetScalarParameterValue;

				checksum += head;
			}

			checksum += position;

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
			UnrealSOLNET::Engine::Manager = NewObject<UUnrealSOLNETManager>();
			UnrealSOLNET::Engine::Manager->AddToRoot();
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
		UnrealSOLNET::Engine::Manager->RemoveFromRoot();
		UnrealSOLNET::Engine::Manager = nullptr;
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
	LevelActor->PrimaryActorTick.AddPrerequisite(UnrealSOLNET::Engine::Manager, TickFunction);
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
