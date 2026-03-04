#include "ConfigManager.hpp"
#include "../core/Logger.hpp"

#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>

// Minimal JSON parser helper (since we avoid external dependencies if possible, matching Main.cpp approach)
// But for robustness, we reuse the logic from Main.cpp but cleaner.

namespace vspp
{
    namespace config
    {

        // Helper functions for manual JSON parsing (simplified for this specific struct)
        static std::string trim(const std::string &str)
        {
            size_t first = str.find_first_not_of(" \t\n\r");
            if (std::string::npos == first)
                return str;
            size_t last = str.find_last_not_of(" \t\n\r");
            return str.substr(first, (last - first + 1));
        }

        static std::optional<std::string> extract_value(const std::string &json, const std::string &key)
        {
            size_t pos = json.find("\"" + key + "\"");
            if (pos == std::string::npos)
                return std::nullopt;

            size_t colon = json.find(":", pos);
            if (colon == std::string::npos)
                return std::nullopt;

            size_t start = json.find_first_not_of(" \t\n\r", colon + 1);
            if (start == std::string::npos)
                return std::nullopt;

            if (json[start] == '"')
            {
                size_t end = json.find("\"", start + 1);
                if (end == std::string::npos)
                    return std::nullopt;
                return json.substr(start + 1, end - start - 1);
            }
            else
            {
                size_t end = json.find_first_of(",}", start);
                if (end == std::string::npos)
                    end = json.length();
                return trim(json.substr(start, end - start));
            }
        }

        ConfigManager &ConfigManager::get()
        {
            static ConfigManager instance;
            return instance;
        }

        void ConfigManager::initialize(const std::filesystem::path &config_path)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_config_path = config_path;
            m_initialized = true;

            // Ensure directory exists
            if (m_config_path.has_parent_path())
            {
                std::filesystem::create_directories(m_config_path.parent_path());
            }

            load();
            core::Logger::get().info("ConfigManager initialized.");
        }

        void ConfigManager::reset()
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_initialized = false;
            m_config = AppConfig();
            m_config_path.clear();
        }

        void ConfigManager::load()
        {
            // Basic implementation - in a real scenario, use a JSON library
            // For now, we load defaults if file missing
            if (!std::filesystem::exists(m_config_path))
            {
                core::Logger::get().warn("Config file not found, creating default.");
                // Create default slots
                m_config.slots.resize(10);
                for (int i = 0; i < 10; ++i)
                    m_config.slots[i].note = "Slot " + std::to_string(i + 1);
                save();
                return;
            }

            std::ifstream file(m_config_path);
            if (!file.is_open())
            {
                core::Logger::get().error("Failed to open config file for reading.");
                return;
            }

            std::stringstream buffer;
            buffer << file.rdbuf();
            parse_json(buffer.str());
        }

        void ConfigManager::save()
        {
            std::ofstream file(m_config_path);
            if (!file.is_open())
            {
                core::Logger::get().error("Failed to open config file for writing.");
                return;
            }

            file << serialize_json();
        }

        AppConfig &ConfigManager::get_config()
        {
            return m_config;
        }

        const AppConfig &ConfigManager::get_config() const
        {
            return m_config;
        }

        void ConfigManager::parse_json(const std::string &json)
        {
            // Very naive parser for demonstration and meeting "no external libs" constraint if implied
            // In production, use nlohmann/json
            // Parsing logic ported from original Main.cpp but simplified

            auto val = extract_value(json, "current_slot_index");
            if (val)
                m_config.current_slot_index = std::stoi(*val);

            val = extract_value(json, "language");
            if (val)
                m_config.language = *val;

            // Parse slots notes (Simple manual parsing)
            m_config.slots.clear();
            m_config.slots.resize(10);

            size_t pos = 0;
            int slot_idx = 0;
            while ((pos = json.find("\"note\"", pos)) != std::string::npos && slot_idx < 10)
            {
                size_t colon = json.find(":", pos);
                if (colon == std::string::npos)
                    break;

                size_t start_quote = json.find("\"", colon + 1);
                if (start_quote == std::string::npos)
                    break;

                size_t end_quote = json.find("\"", start_quote + 1);
                if (end_quote == std::string::npos)
                    break;

                std::string note = json.substr(start_quote + 1, end_quote - start_quote - 1);
                m_config.slots[slot_idx].note = note;

                // Advance pos
                pos = end_quote + 1;
                slot_idx++;
            }

            // Fill remaining with defaults if any
            for (int i = slot_idx; i < 10; ++i)
            {
                m_config.slots[i].note = "Slot " + std::to_string(i + 1);
            }
        }

        std::string ConfigManager::serialize_json() const
        {
            std::stringstream ss;
            ss << "{\n";
            ss << "  \"current_slot_index\": " << m_config.current_slot_index << ",\n";
            ss << "  \"language\": \"" << m_config.language << "\",\n";
            ss << "  \"save_key\": { \"device\": \"" << m_config.save_key.device << "\", \"code\": " << m_config.save_key.code << " },\n";
            ss << "  \"load_key\": { \"device\": \"" << m_config.load_key.device << "\", \"code\": " << m_config.load_key.code << " },\n";
            ss << "  \"toggle_ui_key\": { \"device\": \"" << m_config.toggle_ui_key.device << "\", \"code\": " << m_config.toggle_ui_key.code << " },\n";

            ss << "  \"slots\": [\n";
            for (size_t i = 0; i < m_config.slots.size(); ++i)
            {
                const auto &slot = m_config.slots[i];
                ss << "    {\n";
                ss << "      \"note\": \"" << slot.note << "\",\n";
                if (slot.position)
                {
                    ss << "      \"position\": { \"x\": " << slot.position->x << ", \"y\": " << slot.position->y << ", \"z\": " << slot.position->z << " },\n";
                }
                else
                {
                    ss << "      \"position\": null,\n";
                }
                if (slot.rotation)
                {
                    ss << "      \"rotation\": { \"x\": " << slot.rotation->x << ", \"y\": " << slot.rotation->y << ", \"z\": " << slot.rotation->z << ", \"w\": " << slot.rotation->w << " }\n";
                }
                else
                {
                    ss << "      \"rotation\": null\n";
                }
                ss << "    }" << (i < m_config.slots.size() - 1 ? "," : "") << "\n";
            }
            ss << "  ]\n";
            ss << "}\n";
            return ss.str();
        }

    } // namespace config
} // namespace vspp
