#include <windows.h>
#include <reframework/API.hpp>

extern "C" __declspec(dllexport) void reframework_plugin_initialize(const REFrameworkPluginInitializeParam *param)
{
    reframework::API::initialize(param);

    // 输出初始化日志，验证插件是否加载成功
    const auto &api = reframework::API::get();
    if (api)
    {
        api->log_info("[VaultCore] --------------------------------------------------");
        api->log_info("[VaultCore] Plugin Loaded Successfully!");
        api->log_info("[VaultCore] Version: 1.0.0 (Pre-Alpha)");
        api->log_info("[VaultCore] 幻影传送门核心已启动 - 等待指令");
        api->log_info("[VaultCore] --------------------------------------------------");
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
