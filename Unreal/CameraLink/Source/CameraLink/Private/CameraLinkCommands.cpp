// Copyright Epic Games, Inc. All Rights Reserved.

#include "CameraLinkCommands.h"

#define LOCTEXT_NAMESPACE "FCameraLinkModule"

void FCameraLinkCommands::RegisterCommands()
{
	UI_COMMAND(PluginAction, "CameraLink", "Execute CameraLink action", EUserInterfaceActionType::Button, FInputChord());
}

#undef LOCTEXT_NAMESPACE
