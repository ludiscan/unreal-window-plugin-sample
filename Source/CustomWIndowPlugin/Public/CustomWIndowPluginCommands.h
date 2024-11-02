// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "CustomWIndowPluginStyle.h"

class FCustomWIndowPluginCommands : public TCommands<FCustomWIndowPluginCommands>
{
public:

	FCustomWIndowPluginCommands()
		: TCommands<FCustomWIndowPluginCommands>(TEXT("CustomWIndowPlugin"), NSLOCTEXT("Contexts", "CustomWIndowPlugin", "CustomWIndowPlugin Plugin"), NAME_None, FCustomWIndowPluginStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > OpenPluginWindow;
};