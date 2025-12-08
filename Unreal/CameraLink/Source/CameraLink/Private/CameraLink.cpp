// Copyright Epic Games, Inc. All Rights Reserved.

#include "CameraLink.h"
#include "CameraLinkStyle.h"
#include "CameraLinkCommands.h"
#include "ToolMenus.h"

#include "IDesktopPlatform.h"
#include "DesktopPlatformModule.h"
#include "Framework/Application/SlateApplication.h"

#include "Interfaces/IPluginManager.h"
#include "Modules/ModuleManager.h"
#include "Misc/MessageDialog.h"

#include "IPythonScriptPlugin.h"


#define LOCTEXT_NAMESPACE "FCameraLinkModule"

void FCameraLinkModule::StartupModule()
{
	FCameraLinkStyle::Initialize();
	FCameraLinkStyle::ReloadTextures();

	FCameraLinkCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FCameraLinkCommands::Get().PluginAction,
		FExecuteAction::CreateRaw(this, &FCameraLinkModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FCameraLinkModule::RegisterMenus));

	// Add plugin Python path
    FString PluginDir = IPluginManager::Get().FindPlugin("CameraLink")->GetBaseDir();
    FString PythonPath = PluginDir / TEXT("Content/Python");

    PythonPath = PythonPath.Replace(TEXT("\\"), TEXT("/")); // Normalize

    FString AddPathCommand = FString::Printf(
        TEXT("import sys; sys.path.append(r\"%s\")"),
        *PythonPath
    );

    if (IPythonScriptPlugin* PythonPlugin = FModuleManager::GetModulePtr<IPythonScriptPlugin>("PythonScriptPlugin"))
    {
        PythonPlugin->ExecPythonCommand(*AddPathCommand);
    }
}

void FCameraLinkModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);
	FCameraLinkStyle::Shutdown();
	FCameraLinkCommands::Unregister();
}

void FCameraLinkModule::PluginButtonClicked()
{
	// Get the desktop platform for file dialog
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (!DesktopPlatform)
	{
		FMessageDialog::Open(EAppMsgType::Ok, 
			LOCTEXT("NoDesktopPlatform", "Could not open file dialog."));
		return;
	}

	// Get parent window handle
	TSharedPtr<SWindow> ParentWindow = FSlateApplication::Get().GetActiveTopLevelWindow();
	const void* ParentWindowHandle = nullptr;

	if (ParentWindow.IsValid() && ParentWindow->GetNativeWindow().IsValid())
	{
		ParentWindowHandle = ParentWindow->GetNativeWindow()->GetOSWindowHandle();
	}


	// Open file dialog
	TArray<FString> OutFiles;
	bool bOpened = DesktopPlatform->OpenFileDialog(
		ParentWindowHandle,
		TEXT("Select USD Camera File"),
		FPaths::GetProjectFilePath(),  // Default path
		TEXT(""),                       // Default file
		TEXT("USD Files (*.usda;*.usd)|*.usda;*.usd|All Files (*.*)|*.*"),
		EFileDialogFlags::None,
		OutFiles
	);

	if (bOpened && OutFiles.Num() > 0)
	{
		FString SelectedFile = OutFiles[0];
		
		// Normalize path separators for Python (use forward slashes)
		SelectedFile = SelectedFile.Replace(TEXT("\\"), TEXT("/"));
		
		// Execute Python import
		ExecutePythonImport(SelectedFile);
	}
}

void FCameraLinkModule::ExecutePythonImport(const FString& FilePath)
{
	// Check if Python plugin is available
	IPythonScriptPlugin* PythonPlugin = FModuleManager::GetModulePtr<IPythonScriptPlugin>("PythonScriptPlugin");
	
	if (!PythonPlugin)
	{
		FMessageDialog::Open(EAppMsgType::Ok,
			LOCTEXT("NoPython", "Python Script Plugin is not available. Please enable it in Plugins."));
		return;
	}

	// Get plugin Python path
	FString PluginDir = IPluginManager::Get().FindPlugin("CameraLink")->GetBaseDir();
	FString PythonPath = PluginDir / TEXT("Content/Python");
	PythonPath = PythonPath.Replace(TEXT("\\"), TEXT("/"));

	UE_LOG(LogTemp, Warning, TEXT("CameraLink: Plugin directory: %s"), *PluginDir);
	UE_LOG(LogTemp, Warning, TEXT("CameraLink: Python path: %s"), *PythonPath);

	// Build Python command - add path, print it, THEN import
	FString PythonCommand = FString::Printf(
		TEXT("import sys; "
		     "sys.path.append(r\"%s\"); "
		     "import unreal; "
		     "unreal.log('Python sys.path added: %s'); "
		     "import os; "
		     "unreal.log('File exists: ' + str(os.path.exists(r\"%s/unreal_usd_camera_import.py\"))); "
		     "import unreal_usd_camera_import; "
		     "unreal_usd_camera_import.import_camera(r\"%s\")"),
		*PythonPath,
		*PythonPath,
		*PythonPath,
		*FilePath
	);

	UE_LOG(LogTemp, Warning, TEXT("CameraLink: Executing Python command"));

	// Execute the Python command
	bool bSuccess = PythonPlugin->ExecPythonCommand(*PythonCommand);

	UE_LOG(LogTemp, Warning, TEXT("CameraLink: Python result: %s"), bSuccess ? TEXT("SUCCESS") : TEXT("FAILED"));

	if (!bSuccess)
	{
		FMessageDialog::Open(EAppMsgType::Ok,
			FText::Format(
				LOCTEXT("PythonError", "Failed to execute Python import.\n\nCheck Output Log for detailed path info.\n\nFile: {0}"),
				FText::FromString(FilePath)
			));
	}
}

void FCameraLinkModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);

	// Add to Window menu
	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FCameraLinkCommands::Get().PluginAction, PluginCommands);
		}
	}

	// Add toolbar button
	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PluginTools");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FCameraLinkCommands::Get().PluginAction));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FCameraLinkModule, CameraLink)