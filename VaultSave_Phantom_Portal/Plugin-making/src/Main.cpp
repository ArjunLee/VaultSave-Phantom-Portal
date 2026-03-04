#define NOMINMAX
#include <windows.h>
#include <filesystem>
#include <memory>

// REFramework SDK
#include <reframework/API.hpp>

// Core Systems
#include "core/Logger.hpp"
#include "core/EventBus.hpp"

// Managers
#include "config/ConfigManager.hpp"
#include "ui/UIManager.hpp"

// Game Logic
#include "GameLogic.hpp"

using namespace reframework;

// Global instance of GameLogic
std::unique_ptr<vspp::GameLogic> g_game_logic;

// Callback for frame update
void plugin_on_frame()
{
    if (g_game_logic)
    {
        g_game_logic->on_frame();
    }
}

// Callback for UI drawing
void plugin_on_draw_ui(REFImGuiFrameCbData *data)
{
    vspp::ui::UIManager::get().render(data);
}

// Callback for window messages
bool plugin_on_message(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    if (g_game_logic)
    {
        return g_game_logic->on_message(hwnd, msg, wparam, lparam);
    }
    return true;
}

// Plugin Entry Point
extern "C" __declspec(dllexport) bool reframework_plugin_initialize(const REFrameworkPluginInitializeParam *param)
{
    // Initialize REFramework API
    API::initialize(param);

    // Initialize Logger
    // Path: game_root/reframework/data/VaultSavePhantomPortal/vspp.log
    std::filesystem::path log_path = "reframework/data/VaultSavePhantomPortal/vspp.log";
    vspp::core::Logger::get().initialize(log_path);
    vspp::core::Logger::get().info("Plugin initialization started.");

    // Initialize ConfigManager
    // Path: game_root/reframework/data/VaultSavePhantomPortal/config.json
    std::filesystem::path config_path = "reframework/data/VaultSavePhantomPortal/config.json";
    vspp::config::ConfigManager::get().initialize(config_path);

    // Initialize UIManager
    vspp::ui::UIManager::get().initialize();

    // Initialize GameLogic
    g_game_logic = std::make_unique<vspp::GameLogic>();
    g_game_logic->initialize();

    // Register Callbacks
    const auto functions = param->functions;
    functions->on_present(plugin_on_frame);
    functions->on_message((REFOnMessageCb)plugin_on_message);
    functions->on_imgui_draw_ui(plugin_on_draw_ui);

    vspp::core::Logger::get().info("Plugin initialization completed successfully.");
    return true;
}
