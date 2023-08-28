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


#include "UnrealSOLNET_Manager.h"

void UUnrealSOLNETManager::ActorBeginOverlap(AActor* OverlapActor, AActor* OtherActor) {
	if (UnrealSOLNET::Shared::Events[UnrealSOLNET::OnActorBeginOverlap]) {
		void* parameters[2] = {
			OverlapActor,
			OtherActor
		};

		UnrealSOLNET::ManagedCommand(UnrealSOLNET::Command(UnrealSOLNET::Shared::Events[UnrealSOLNET::OnActorBeginOverlap], UnrealSOLNET::Callback(parameters, UnrealSOLNET::CallbackType::ActorOverlapDelegate)));
	}
}

void UUnrealSOLNETManager::ActorEndOverlap(AActor* OverlapActor, AActor* OtherActor) {
	if (UnrealSOLNET::Shared::Events[UnrealSOLNET::OnActorEndOverlap]) {
		void* parameters[2] = {
			OverlapActor,
			OtherActor
		};

		UnrealSOLNET::ManagedCommand(UnrealSOLNET::Command(UnrealSOLNET::Shared::Events[UnrealSOLNET::OnActorEndOverlap], UnrealSOLNET::Callback(parameters, UnrealSOLNET::CallbackType::ActorOverlapDelegate)));
	}
}

void UUnrealSOLNETManager::ActorHit(AActor* HitActor, AActor* OtherActor, FVector NormalImpulse, const FHitResult& Hit) {
	if (UnrealSOLNET::Shared::Events[UnrealSOLNET::OnActorHit]) {
		UnrealSOLNETFramework::Vector3 normalImpulse(NormalImpulse);
		UnrealSOLNETFramework::Hit hit(Hit);

		void* parameters[4] = {
			HitActor,
			OtherActor,
			&normalImpulse,
			&hit
		};

		UnrealSOLNET::ManagedCommand(UnrealSOLNET::Command(UnrealSOLNET::Shared::Events[UnrealSOLNET::OnActorHit], UnrealSOLNET::Callback(parameters, UnrealSOLNET::CallbackType::ActorHitDelegate)));
	}
}

void UUnrealSOLNETManager::ActorBeginCursorOver(AActor* Actor) {
	if (UnrealSOLNET::Shared::Events[UnrealSOLNET::OnActorBeginCursorOver]) {
		void* parameters[1] = {
			Actor
		};

		UnrealSOLNET::ManagedCommand(UnrealSOLNET::Command(UnrealSOLNET::Shared::Events[UnrealSOLNET::OnActorBeginCursorOver], UnrealSOLNET::Callback(parameters, UnrealSOLNET::CallbackType::ActorCursorDelegate)));
	}
}

void UUnrealSOLNETManager::ActorEndCursorOver(AActor* Actor) {
	if (UnrealSOLNET::Shared::Events[UnrealSOLNET::OnActorEndCursorOver]) {
		void* parameters[1] = {
			Actor
		};

		UnrealSOLNET::ManagedCommand(UnrealSOLNET::Command(UnrealSOLNET::Shared::Events[UnrealSOLNET::OnActorEndCursorOver], UnrealSOLNET::Callback(parameters, UnrealSOLNET::CallbackType::ActorCursorDelegate)));
	}
}

void UUnrealSOLNETManager::ActorClicked(AActor* Actor, FKey Key) {
	if (UnrealSOLNET::Shared::Events[UnrealSOLNET::OnActorClicked]) {
		FString key = Key.ToString();

		void* parameters[2] = {
			Actor,
			TCHAR_TO_ANSI(*key)
		};

		UnrealSOLNET::ManagedCommand(UnrealSOLNET::Command(UnrealSOLNET::Shared::Events[UnrealSOLNET::OnActorClicked], UnrealSOLNET::Callback(parameters, UnrealSOLNET::CallbackType::ActorKeyDelegate)));
	}
}

void UUnrealSOLNETManager::ActorReleased(AActor* Actor, FKey Key) {
	if (UnrealSOLNET::Shared::Events[UnrealSOLNET::OnActorReleased]) {
		FString key = Key.ToString();

		void* parameters[2] = {
			Actor,
			TCHAR_TO_ANSI(*key)
		};

		UnrealSOLNET::ManagedCommand(UnrealSOLNET::Command(UnrealSOLNET::Shared::Events[UnrealSOLNET::OnActorReleased], UnrealSOLNET::Callback(parameters, UnrealSOLNET::CallbackType::ActorKeyDelegate)));
	}
}

void UUnrealSOLNETManager::ComponentBeginOverlap(UPrimitiveComponent* OverlapComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, bool FromSweep, const FHitResult& SweepResult) {
	if (UnrealSOLNET::Shared::Events[UnrealSOLNET::OnComponentBeginOverlap]) {
		void* parameters[2] = {
			OverlapComponent,
			OtherComponent
		};

		UnrealSOLNET::ManagedCommand(UnrealSOLNET::Command(UnrealSOLNET::Shared::Events[UnrealSOLNET::OnComponentBeginOverlap], UnrealSOLNET::Callback(parameters, UnrealSOLNET::CallbackType::ComponentOverlapDelegate)));
	}
}

void UUnrealSOLNETManager::ComponentEndOverlap(UPrimitiveComponent* OverlapComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex) {
	if (UnrealSOLNET::Shared::Events[UnrealSOLNET::OnComponentEndOverlap]) {
		void* parameters[2] = {
			OverlapComponent,
			OtherComponent
		};

		UnrealSOLNET::ManagedCommand(UnrealSOLNET::Command(UnrealSOLNET::Shared::Events[UnrealSOLNET::OnComponentEndOverlap], UnrealSOLNET::Callback(parameters, UnrealSOLNET::CallbackType::ComponentOverlapDelegate)));
	}
}

void UUnrealSOLNETManager::ComponentHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit) {
	if (UnrealSOLNET::Shared::Events[UnrealSOLNET::OnComponentHit]) {
		UnrealSOLNETFramework::Vector3 normalImpulse(NormalImpulse);
		UnrealSOLNETFramework::Hit hit(Hit);

		void* parameters[4] = {
			HitComponent,
			OtherComponent,
			&normalImpulse,
			&hit
		};

		UnrealSOLNET::ManagedCommand(UnrealSOLNET::Command(UnrealSOLNET::Shared::Events[UnrealSOLNET::OnComponentHit], UnrealSOLNET::Callback(parameters, UnrealSOLNET::CallbackType::ComponentHitDelegate)));
	}
}

void UUnrealSOLNETManager::ComponentBeginCursorOver(UPrimitiveComponent* Component) {
	if (UnrealSOLNET::Shared::Events[UnrealSOLNET::OnActorBeginCursorOver]) {
		void* parameters[1] = {
			Component
		};

		UnrealSOLNET::ManagedCommand(UnrealSOLNET::Command(UnrealSOLNET::Shared::Events[UnrealSOLNET::OnComponentBeginCursorOver], UnrealSOLNET::Callback(parameters, UnrealSOLNET::CallbackType::ComponentCursorDelegate)));
	}
}

void UUnrealSOLNETManager::ComponentEndCursorOver(UPrimitiveComponent* Component) {
	if (UnrealSOLNET::Shared::Events[UnrealSOLNET::OnComponentEndCursorOver]) {
		void* parameters[1] = {
			Component
		};

		UnrealSOLNET::ManagedCommand(UnrealSOLNET::Command(UnrealSOLNET::Shared::Events[UnrealSOLNET::OnComponentEndCursorOver], UnrealSOLNET::Callback(parameters, UnrealSOLNET::CallbackType::ComponentCursorDelegate)));
	}
}

void UUnrealSOLNETManager::ComponentClicked(UPrimitiveComponent* Component, FKey Key) {
	if (UnrealSOLNET::Shared::Events[UnrealSOLNET::OnComponentClicked]) {
		FString key = Key.ToString();

		void* parameters[2] = {
			Component,
			TCHAR_TO_ANSI(*key)
		};

		UnrealSOLNET::ManagedCommand(UnrealSOLNET::Command(UnrealSOLNET::Shared::Events[UnrealSOLNET::OnComponentClicked], UnrealSOLNET::Callback(parameters, UnrealSOLNET::CallbackType::ComponentKeyDelegate)));
	}
}

void UUnrealSOLNETManager::ComponentReleased(UPrimitiveComponent* Component, FKey Key) {
	if (UnrealSOLNET::Shared::Events[UnrealSOLNET::OnComponentReleased]) {
		FString key = Key.ToString();

		void* parameters[2] = {
			Component,
			TCHAR_TO_ANSI(*key)
		};

		UnrealSOLNET::ManagedCommand(UnrealSOLNET::Command(UnrealSOLNET::Shared::Events[UnrealSOLNET::OnComponentReleased], UnrealSOLNET::Callback(parameters, UnrealSOLNET::CallbackType::ComponentKeyDelegate)));
	}
}