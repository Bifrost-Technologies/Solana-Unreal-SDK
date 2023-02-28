using System.IO;
using UnrealBuildTool;
using System.Collections.Generic;
using EpicGames.Core;

public class SolanaMobileUE5 : ModuleRules
{
    private string ThirdPartyPath
    {
        get { return Path.GetFullPath(Path.Combine(ModuleDirectory, "ThirdParty/")); }
    }

    private string PathThirdPartyAndroid
    {
        get { return Path.GetFullPath(Path.Combine(ThirdPartyPath, "Android/")); }
    }

    private string PathThirdPartyWindows
    {
        get { return Path.GetFullPath(Path.Combine(ThirdPartyPath, "Windows/")); }
    }

 
    public SolanaMobileUE5(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(new string[] { Path.Combine(ModuleDirectory, "Public") });        
        PrivateIncludePaths.AddRange(new string[] { Path.Combine(ModuleDirectory, "Private") });

        PublicDependencyModuleNames.AddRange(new string[]
            {
                "Engine",
                "Core",
                "CoreUObject",
                "InputCore",                
            }
        );

        PrivateDependencyModuleNames.AddRange(new string[]
            {
                "RenderCore",
                "Slate",
                "SlateCore"
            }
        );

        //-- Additional build steps
        LoadLib(Target);
    }

    //=====Enable libraries depending on the platform================
    public void LoadLib(ReadOnlyTargetRules Target)
    {
        //== If the Windows platform
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            //Add Include path
            PublicIncludePaths.Add(Path.Combine(PathThirdPartyWindows, "include"));

            string[] Libs = {
                /*"example.lib",
                    "example2.lib"*/
            };

            string[] DLLs = {
                /*"example.dll",
                   "example2.dll"*/
            };

            //Add Static Libraries Win
            foreach (string Lib in Libs)
            {
                PublicAdditionalLibraries.Add(Path.Combine(PathThirdPartyWindows, "lib", Lib));
            }

            //Add Dynamic Libraries Win
            foreach (string DLL in DLLs)
            {
                PublicDelayLoadDLLs.Add(Path.Combine(PathThirdPartyWindows, "lib", DLL));
            }
        }

        //== If the Android platform
        else if (Target.Platform == UnrealTargetPlatform.Android)
        {
            string ArchArmV7a = "armeabi-v7a";
            string ArchArmV8a = "arm64-v8a";

            //for JNI
            PrivateDependencyModuleNames.AddRange(new string[]
                {
                    "Launch"
                }
            );

            //------ .h--------------------
            PrivateIncludePaths.AddRange(new string[] { Path.Combine(ModuleDirectory, "Private", "Android") });

            //----- .so ------------------
            // To connect .so dynamic libraries also need to be added to .xml file

            // libc++_shared.so already added, but if you use the engine version 4.24 or less, you can uncomment these lines and in xml
            string[] Libs = {
				//"libc++_shared.so", 
            };

            foreach (string Lib in Libs)
            {
                PublicAdditionalLibraries.Add(Path.Combine(PathThirdPartyAndroid, ArchArmV7a, "lib", Lib));
                PublicAdditionalLibraries.Add(Path.Combine(PathThirdPartyAndroid, ArchArmV8a, "lib", Lib));
            }

            //-------XML---------------------------------------
            {/* Additional steps for building on Android. Basically, all the basic information is specified in xml.
             * The xml file is located on the path: "Plugins\SolanaMobileUE5\Source\SolanaMobileUE5\SolanaMobileUE5_UPL_Android.xml"
             */}

            AdditionalPropertiesForReceipt.Add("AndroidPlugin", Path.Combine(ModuleDirectory, "SolanaMobileUE5_UPL_Android.xml"));
        }

      
    }

}
