#pragma once

#include <string>
#include <vector>
#include <optional>
#include <filesystem>
#include <mutex>

#include "../core/Types.hpp"

namespace vspp
{
    namespace config
    {

        using core::Quaternion;
        using core::Vector3;

        struct SlotData
        {
            std::optional<Vector3> position{};
            std::optional<Quaternion> rotation{};
            std::string note{};
        };

        struct KeyBinding
        {
            std::string device{"keyboard"};
            int code{0};
        };

        struct AppConfig
        {
            int current_slot_index = 1;
            KeyBinding save_key = {"keyboard", 116};     // F5
            KeyBinding load_key = {"keyboard", 120};     // F9
            KeyBinding toggle_ui_key = {"keyboard", 45}; // Insert (Default) - user can change
            std::string language{"System"};
            std::vector<SlotData> slots{};
        };

        /**
         * @brief ConfigManager handles loading and saving application configuration.
         * Adheres to REFramework's data directory structure.
         */
        class ConfigManager
        {
        public:
            static ConfigManager &get();

            void initialize(const std::filesystem::path &config_path);
            void reset();
            void load();
            void save();

            AppConfig &get_config();
            const AppConfig &get_config() const;

        private:
            ConfigManager() = default;
            ~ConfigManager() = default;
            ConfigManager(const ConfigManager &) = delete;
            ConfigManager &operator=(const ConfigManager &) = delete;

            std::filesystem::path m_config_path;
            AppConfig m_config;
            std::mutex m_mutex;
            bool m_initialized{false};

            void parse_json(const std::string &json_content);
            std::string serialize_json() const;
        };

    } // namespace config
} // namespace vspp
