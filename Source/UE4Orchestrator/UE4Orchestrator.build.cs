using UnrealBuildTool;
using System.IO;
using System.Collections.Generic;

public class UE4Orchestrator : ModuleRules
{
    public UE4Orchestrator(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "Sockets",
            "Json",
            "JsonUtilities",
            "NetworkFile"
        });

        PrivateDependencyModuleNames.AddRange(new string[] {
            "HTTP",
            "Sockets",
            "Json",
            "JsonUtilities",
            "NetworkFile"
        });
        //PrivateDependencyModuleNames.AddRange(
        //    new string[]
        //    {
        //        "Sockets",
        //        "StreamingFile",
        //        "LevelEditor",
        //        //"NetworkFile",
        //        //"PakFile",
        //        "InputCore",
        //        "Json",
        //        "JsonUtilities",
        //        "AssetTools",
        //        "HTTP",
        //        // ... add private dependencies that you statically link with here ...
        //    }
        //);

        //PublicDependencyModuleNames.AddRange(
        //    new string[] {
        //        "Core",
        //        "CoreUObject", // @todo Mac: for some reason it's needed to link in debug on Mac
        //  "Engine",
        //        //"PakFile",
        //        "Sockets",
        //        "StreamingFile",
        //        "LevelEditor",
        //        //"NetworkFile",
        //        "InputCore",
        //        "Json",
        //        "JsonUtilities",
        //        "AssetTools",
        //    }
        //);

        //PrivateIncludePaths.AddRange(
        //    new string[] {
        //        "UE4Orchestrator/Private",
        //        // ... add other private include paths required here ...
        //    }
        //    );

        //PublicIncludePaths.AddRange(
        //    new string[] {
        //        "UE4Orchestrator/Public",
        //        // ... add other private include paths required here ...
        //    }
        //    );

        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.Add("UnrealEd");
            PublicDependencyModuleNames.Add("UnrealEd");
        }
    }
}
