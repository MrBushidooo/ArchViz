// Copyright Epic Games, Inc. All Rights Reserved.

#include "ArchVizGameMode.h"
#include "ArchVizCharacter.h"
#include "UObject/ConstructorHelpers.h"

AArchVizGameMode::AArchVizGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}
