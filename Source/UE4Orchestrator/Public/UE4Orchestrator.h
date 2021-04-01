/* -*- mode: c; tab-width: 4; indent-tabs-mode: nil; -*- */

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

class FUE4OrchestratorModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

// Logging stuff ...
DECLARE_LOG_CATEGORY_EXTERN(LogUE4Orc, Log, All);

#include "mongoose.h"

// UE4
//#include "IPlatformFilePak.h"
//#include "Runtime/Core/Public/HAL/FileManagerGeneric.h"
//#include "StreamingNetworkPlatformFile.h"
//#include "Runtime/AssetRegistry/Public/AssetRegistryModule.h"
//#include "Modules/ModuleInterface.h"

#if WITH_EDITOR
#  include "LevelEditor.h"
#  include "Editor.h"
#  include "Editor/LevelEditor/Public/ILevelViewport.h"
#  include "Editor/LevelEditor/Public/LevelEditorActions.h"
#  include "Editor/UnrealEd/Public/LevelEditorViewport.h"
#endif

////////////////////////////////////////////////////////////////////////////////

typedef struct mg_str               mg_str_t;
typedef struct http_message         http_message_t;
typedef FModuleManager              FManager;
typedef FActorComponentTickFunction FTickFn;

#if WITH_EDITOR
  typedef FLevelEditorModule        FLvlEditor;
#endif

#define T                   TEXT
#define LOG(fmt, ...)       UE_LOG(LogUE4Orc, Log, TEXT(fmt), __VA_ARGS__)



