// Copyright Epic Games, Inc. All Rights Reserved.

#include "SolanaGameTemplateGameMode.h"
#include "../Characters/SolanaGameTemplateCharacter.h"
#include "UObject/ConstructorHelpers.h"

ASolanaGameTemplateGameMode::ASolanaGameTemplateGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
