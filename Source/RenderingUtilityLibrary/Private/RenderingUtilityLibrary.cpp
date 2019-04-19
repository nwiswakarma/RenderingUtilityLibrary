////////////////////////////////////////////////////////////////////////////////
//
// MIT License
// 
// Copyright (c) 2018-2019 Nuraga Wiswakarma
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
////////////////////////////////////////////////////////////////////////////////
//

#include "RenderingUtilityLibrary.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"

#if WITH_EDITOR
#include "ISettingsModule.h"
#include "ISettingsSection.h"
#endif

#include "RenderingUtilitySettings.h"

#define LOCTEXT_NAMESPACE "IRenderingUtilityLibrary"

class FRenderingUtilityLibrary : public IRenderingUtilityLibrary
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:

    void RegisterSettings();
    void UnregisterSettings();
    bool HandleSettingsSaved();
};

void FRenderingUtilityLibrary::StartupModule()
{
    // Register shader source base directory
    FString PluginShaderDir = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("RenderingUtilityLibrary"))->GetBaseDir(), TEXT("Shaders"));
    AddShaderSourceDirectoryMapping(TEXT("/Plugin/RenderingUtilityLibrary"), PluginShaderDir);

#if WITH_EDITOR
    // We don't quite have control of when the "Settings" module is loaded, so we'll wait until PostEngineInit to register settings.
    FCoreDelegates::OnPostEngineInit.AddRaw(this, &FRenderingUtilityLibrary::RegisterSettings);
#endif // WITH_EDITOR
}

void FRenderingUtilityLibrary::ShutdownModule()
{
#if WITH_EDITOR
    UnregisterSettings();
#endif
}

#if WITH_EDITOR
void FRenderingUtilityLibrary::RegisterSettings()
{
    ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");

    // While this should usually be true, it's not guaranteed that the settings module will be loaded in the editor.
    // UBT allows setting bBuildDeveloperTools to false while bBuildEditor can be true.
    // The former option indirectly controls loading of the "Settings" module.
    if (SettingsModule)
    {
        ISettingsSectionPtr SettingsSection = SettingsModule->RegisterSettings("Project", "Plugins", "RenderingUtilityLibrary",
            LOCTEXT("RenderingUtilityLibrarySettingsName", "Rendering Utility Library"),
            LOCTEXT("RenderingUtilityLibrarySettingsDescription", "Configure the Rendering Utility Library plug-in."),
            GetMutableDefault<URenderingUtilitySettings>()
        );

        if (SettingsSection.IsValid())
        {
            SettingsSection->OnModified().BindRaw(this, &FRenderingUtilityLibrary::HandleSettingsSaved);
        }
    }
}

void FRenderingUtilityLibrary::UnregisterSettings()
{
    ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");

    if (SettingsModule)
    {
        SettingsModule->UnregisterSettings("Project", "Plugins", "RenderingUtilityLibrary");
    }
}

bool FRenderingUtilityLibrary::HandleSettingsSaved()
{
    URenderingUtilitySettings* Settings = GetMutableDefault<URenderingUtilitySettings>();

    if (IsValid(Settings))
    {
        Settings->SaveConfig();
    }

    return true;
}
#endif

IMPLEMENT_MODULE(FRenderingUtilityLibrary, RenderingUtilityLibrary)
DEFINE_LOG_CATEGORY(LogRUL);
DEFINE_LOG_CATEGORY(UntRUL);

#undef LOCTEXT_NAMESPACE
