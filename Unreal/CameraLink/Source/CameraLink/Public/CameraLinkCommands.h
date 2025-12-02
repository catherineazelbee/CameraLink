// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Framework/Commands/Commands.h"
#include "CameraLinkStyle.h"

class FCameraLinkCommands : public TCommands<FCameraLinkCommands>
{
public:

	FCameraLinkCommands()
		: TCommands<FCameraLinkCommands>(TEXT("CameraLink"), NSLOCTEXT("Contexts", "CameraLink", "CameraLink Plugin"), NAME_None, FCameraLinkStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr< FUICommandInfo > PluginAction;
};
