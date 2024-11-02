// Copyright Epic Games, Inc. All Rights Reserved.

#include "CustomWIndowPluginCommands.h"

#define LOCTEXT_NAMESPACE "FCustomWIndowPluginModule"

void FCustomWIndowPluginCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "CustomWIndowPlugin", "Bring up CustomWIndowPlugin window", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
