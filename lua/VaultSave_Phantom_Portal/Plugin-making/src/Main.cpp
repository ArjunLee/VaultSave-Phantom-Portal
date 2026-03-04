#include <windows.h>
#include <reframework/API.hpp>
#include <filesystem>
#include <shlobj.h> // For SHGetFolderPath or similar if needed, but relative path might work

extern "C" __declspec(dllexport) void reframework_plugin_initialize(const REFrameworkPluginInitializeParam *param)
{
    reframework::API::initialize(param);

    // Output initialization log to verify plugin loading
    const auto &api = reframework::API::get();
    if (api)
    {
        api->log_info("[VaultSave] --------------------------------------------------");
        api->log_info("[VaultSave] Plugin Loaded Successfully!");
        api->log_info("[VaultSave] Version: 1.0.0 (Pre-Alpha)");
        api->log_info("[VaultSave] Phantom Portal Core Started - Awaiting Instructions");
        api->log_info("[VaultSave] --------------------------------------------------");

        // Create config directory
        // Note: REFramework plugins usually work in the game root directory
        // We attempt to create reframework/data/VaultSavePhantomTravel
        try
        {
            std::filesystem::path config_dir = "reframework/data/VaultSavePhantomTravel";
            if (!std::filesystem::exists(config_dir))
            {
                if (std::filesystem::create_directories(config_dir))
                {
                    api->log_info("[VaultSave] Config directory created: %s", config_dir.string().c_str());
                }
                else
                {
                    api->log_warn("[VaultSave] Failed to create config directory (or it already exists).");
                }
            }
            else
            {
                api->log_info("[VaultSave] Config directory exists.");
            }
        }
        catch (const std::exception &e)
        {
            api->log_error("[VaultSave] Error creating directory: %s", e.what());
        }

        api->log_warn("[VaultSave] UI Not Implemented Yet: Requires ImGui integration.");
    }
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
