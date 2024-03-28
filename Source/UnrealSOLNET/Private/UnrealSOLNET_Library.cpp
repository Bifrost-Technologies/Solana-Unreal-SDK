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


#include "UnrealSOLNET_Library.h"

FManagedFunction::FManagedFunction() : Pointer() { }

UUnrealSOLNETLibrary::UUnrealSOLNETLibrary(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) { }

void UUnrealSOLNETLibrary::ExecuteSDKFunction(FString Method, bool Optional, bool& Result, UObject* Object = nullptr) {
	FManagedFunction managedFunction;

	if (UnrealSOLNET::Status == UnrealSOLNET::StatusType::Running && !Method.IsEmpty())
		managedFunction.Pointer = UnrealSOLNET::ManagedCommand(UnrealSOLNET::Command(TCHAR_TO_ANSI(*Method), Optional));

	Result = managedFunction.Pointer != nullptr;

	if (UnrealSOLNET::Status == UnrealSOLNET::StatusType::Running && managedFunction.Pointer)
		UnrealSOLNET::ManagedCommand(UnrealSOLNET::Command(managedFunction.Pointer, Object));
}

