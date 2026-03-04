#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <thread>

// Core Systems
#include "../src/core/EventBus.hpp"
#include "../src/core/Logger.hpp"
#include "../src/config/ConfigManager.hpp"
#include "../src/ui/LocalizationManager.hpp"
#include "../src/ui/UIManager.hpp"

// ImGui for UIManager tests
#include "../src/imgui/imgui.h"

// Helper to clean up test files
class TestEnv : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Setup ImGui context for UI tests
        ImGui::CreateContext();

        // Ensure clean slate for config
        if (std::filesystem::exists("test_config.json"))
            std::filesystem::remove("test_config.json");
        if (std::filesystem::exists("test_log.log"))
            std::filesystem::remove("test_log.log");

        // Initialize Logger
        vspp::core::Logger::get().initialize("test_log.log");

        // Reset Config
        vspp::config::ConfigManager::get().reset();

        // Reset EventBus
        vspp::core::EventBus::get().reset();
    }

    void TearDown() override
    {
        ImGui::DestroyContext();

        // Close Logger so file can be deleted
        vspp::core::Logger::get().close();

        // Reset Config
        vspp::config::ConfigManager::get().reset();

        // Reset EventBus
        vspp::core::EventBus::get().reset();

        if (std::filesystem::exists("test_config.json"))
            std::filesystem::remove("test_config.json");
        if (std::filesystem::exists("test_log.log"))
            std::filesystem::remove("test_log.log");
    }
};

// --- ConfigManager Tests ---
TEST_F(TestEnv, ConfigSaveAndLoad)
{
    auto &cm = vspp::config::ConfigManager::get();
    cm.initialize("test_config.json");

    // Modify config
    cm.get_config().current_slot_index = 5;
    cm.get_config().language = "zh_cn";
    cm.save();

    // Reset and reload
    cm.reset();
    cm.initialize("test_config.json"); // Re-init to load

    EXPECT_EQ(cm.get_config().current_slot_index, 5);
    EXPECT_EQ(cm.get_config().language, "zh_cn");
}

TEST_F(TestEnv, ConfigDefaultValues)
{
    auto &cm = vspp::config::ConfigManager::get();
    // Initialize with non-existent file
    if (std::filesystem::exists("test_config.json"))
        std::filesystem::remove("test_config.json");
    cm.initialize("test_config.json");

    EXPECT_EQ(cm.get_config().current_slot_index, 1);
    EXPECT_EQ(cm.get_config().language, "System");
}

TEST_F(TestEnv, ConfigUpdateNote)
{
    auto &cm = vspp::config::ConfigManager::get();
    cm.initialize("test_config.json");

    // Default has 10 slots
    ASSERT_GE(cm.get_config().slots.size(), 1);
    cm.get_config().slots[0].note = "Test Note";
    cm.save();

    cm.reset();
    cm.initialize("test_config.json");

    EXPECT_EQ(cm.get_config().slots[0].note, "Test Note");
}

TEST_F(TestEnv, ConfigSlotCount)
{
    auto &cm = vspp::config::ConfigManager::get();
    cm.initialize("test_config.json");
    EXPECT_EQ(cm.get_config().slots.size(), 10);
}

TEST_F(TestEnv, ConfigKeyBindingDefaults)
{
    auto &cm = vspp::config::ConfigManager::get();
    cm.initialize("test_config.json");
    EXPECT_EQ(cm.get_config().save_key.code, 116); // F5
}

// --- EventBus Tests ---
TEST_F(TestEnv, EventBusPublishSubscribe)
{
    auto &bus = vspp::core::EventBus::get();
    bool event_received = false;
    int received_slot = -1;

    bus.subscribe(vspp::core::EventType::Teleport, [&](const vspp::core::Event &e)
                  {
        event_received = true;
        received_slot = std::get<int>(e.data); });

    bus.publish({vspp::core::EventType::Teleport, 42});

    EXPECT_TRUE(event_received);
    EXPECT_EQ(received_slot, 42);
}

TEST_F(TestEnv, EventBusMultipleHandlers)
{
    auto &bus = vspp::core::EventBus::get();
    int count = 0;

    bus.subscribe(vspp::core::EventType::SavePosition, [&](const vspp::core::Event &e)
                  { count++; });
    bus.subscribe(vspp::core::EventType::SavePosition, [&](const vspp::core::Event &e)
                  { count++; });

    bus.publish({vspp::core::EventType::SavePosition, 0});

    EXPECT_EQ(count, 2);
}

TEST_F(TestEnv, EventBusDifferentTypes)
{
    auto &bus = vspp::core::EventBus::get();
    bool save_called = false;
    bool load_called = false;

    bus.subscribe(vspp::core::EventType::SavePosition, [&](const vspp::core::Event &e)
                  { save_called = true; });
    bus.subscribe(vspp::core::EventType::Teleport, [&](const vspp::core::Event &e)
                  { load_called = true; });

    bus.publish({vspp::core::EventType::SavePosition, 0});

    EXPECT_TRUE(save_called);
    EXPECT_FALSE(load_called);
}

TEST_F(TestEnv, EventBusReset)
{
    auto &bus = vspp::core::EventBus::get();
    int count = 0;
    bus.subscribe(vspp::core::EventType::SavePosition, [&](const vspp::core::Event &e)
                  { count++; });

    bus.reset();
    bus.publish({vspp::core::EventType::SavePosition, 0});

    EXPECT_EQ(count, 0);
}

// --- LocalizationManager Tests ---
TEST_F(TestEnv, LocalizationSwitch)
{
    auto &loc = vspp::ui::LocalizationManager::get();

    loc.set_language("en");
    EXPECT_EQ(loc.get_text("window_title"), "VaultSave Phantom Portal");

    loc.set_language("zh_cn");
    EXPECT_EQ(loc.get_text("window_title"), "幽灵传送门 (VaultSave)");
}

TEST_F(TestEnv, LocalizationFallback)
{
    auto &loc = vspp::ui::LocalizationManager::get();
    loc.set_language("invalid_lang");
    // Should fallback to English
    EXPECT_EQ(loc.get_text("window_title"), "VaultSave Phantom Portal");
}

TEST_F(TestEnv, LocalizationMissingKey)
{
    auto &loc = vspp::ui::LocalizationManager::get();
    loc.set_language("en");
    EXPECT_EQ(loc.get_text("non_existent_key"), "non_existent_key");
}

// --- UIManager Tests ---
TEST_F(TestEnv, UIVisibilityToggle)
{
    auto &ui = vspp::ui::UIManager::get();
    ui.initialize();

    ui.set_visible(false);
    EXPECT_FALSE(ui.is_visible());

    ui.toggle_visible();
    EXPECT_TRUE(ui.is_visible());

    ui.toggle_visible();
    EXPECT_FALSE(ui.is_visible());
}

TEST_F(TestEnv, UIInitialState)
{
    auto &ui = vspp::ui::UIManager::get();
    // Default might be false
    ui.initialize();
    ui.set_visible(false);
    EXPECT_FALSE(ui.is_visible());
}

TEST_F(TestEnv, UISlotSelection)
{
    // Mock UI interaction by checking logic if possible
    // UIManager uses ConfigManager for slots.
    auto &cm = vspp::config::ConfigManager::get();
    cm.initialize("test_config.json");

    // Verify initial slot index
    EXPECT_EQ(cm.get_config().current_slot_index, 1);
}

// --- Logger Tests ---
TEST_F(TestEnv, LoggerInit)
{
    auto &logger = vspp::core::Logger::get();
    // Already init in SetUp
    logger.info("Test message");
    // Verify file exists
    EXPECT_TRUE(std::filesystem::exists("test_log.log"));
}

TEST_F(TestEnv, LoggerClose)
{
    auto &logger = vspp::core::Logger::get();
    logger.close();
    // File should be closable/removable
    EXPECT_TRUE(std::filesystem::remove("test_log.log"));

    // Re-init for TearDown safety (although TearDown closes it anyway)
    // But SetUp/TearDown pair is per test.
    // If we remove it here, TearDown might fail to remove it if it expects it?
    // TearDown checks exists() before remove(). So it's fine.
}

TEST_F(TestEnv, LoggerLevels)
{
    auto &logger = vspp::core::Logger::get();
    logger.info("Info msg");
    logger.warn("Warn msg");
    logger.error("Error msg");

    logger.close();

    std::ifstream file("test_log.log");
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    EXPECT_NE(content.find("[INFO] Info msg"), std::string::npos);
    EXPECT_NE(content.find("[WARN] Warn msg"), std::string::npos);
    EXPECT_NE(content.find("[ERROR] Error msg"), std::string::npos);
}

// --- Integration/Logic Tests ---
TEST_F(TestEnv, SaveEventFlow)
{
    auto &bus = vspp::core::EventBus::get();
    auto &cm = vspp::config::ConfigManager::get();
    cm.initialize("test_config.json");

    bool save_triggered = false;
    bus.subscribe(vspp::core::EventType::SavePosition, [&](const vspp::core::Event &e)
                  { save_triggered = true; });

    // Simulate save trigger (e.g. from UI or Key)
    bus.publish({vspp::core::EventType::SavePosition, 1});

    EXPECT_TRUE(save_triggered);
}

TEST_F(TestEnv, LoadEventFlow)
{
    auto &bus = vspp::core::EventBus::get();
    bool load_triggered = false;
    bus.subscribe(vspp::core::EventType::Teleport, [&](const vspp::core::Event &e)
                  { load_triggered = true; });

    bus.publish({vspp::core::EventType::Teleport, 1});

    EXPECT_TRUE(load_triggered);
}
