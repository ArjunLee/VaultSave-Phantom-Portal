#include "UIManager.hpp"
#include "LocalizationManager.hpp"
#include "../config/ConfigManager.hpp"
#include "../core/EventBus.hpp"
#include "../core/Logger.hpp"

#include "imgui/imgui.h"

namespace vspp
{
    namespace ui
    {

        UIManager &UIManager::get()
        {
            static UIManager instance;
            return instance;
        }

        void UIManager::initialize()
        {
            // Note: If using REFramework's on_imgui_draw_ui, we don't need to init ImGui context ourselves
            // But if we want to customize style, we can do it here.
            m_initialized = true;
            core::Logger::get().info("UIManager initialized.");
        }

        void UIManager::toggle_visible()
        {
            m_visible = !m_visible;
        }

        void UIManager::render()
        {
            if (!m_initialized)
                return;

            // Check if we should draw
            // If we are part of the REFramework overlay, we usually draw when is_drawing_ui() is true.
            // The user requirement: "Fix plugin cannot show/hide via REFramework shortcut".
            // This implies we should be visible when the overlay is visible.

            bool reframework_ui_visible = false;
            if (auto *api = reframework::API::try_get())
            {
                if (api->param()->functions->is_drawing_ui())
                {
                    reframework_ui_visible = true;
                }
            }

            // Logic: We draw if (m_visible [custom toggle] OR reframework_ui_visible [menu open])
            // But typically plugins are EITHER "Always on overlay" OR "Menu windows".
            // If the user wants "Show/Hide via REFramework shortcut", they probably mean:
            // "When I press Insert, the window should appear/disappear."
            // So we should respect reframework_ui_visible.
            // BUT if the user ALSO wants a custom shortcut (F5 to toggle window), we support that too.

            if (!m_visible && !reframework_ui_visible)
            {
                return;
            }

            draw_main_window();
        }

        void UIManager::draw_main_window()
        {
            auto &loc = LocalizationManager::get();
            auto &cfg_mgr = config::ConfigManager::get();
            auto &config = cfg_mgr.get_config();

            if (ImGui::Begin(loc.get_text("window_title").c_str(), nullptr))
            {
                ImGui::Text("%s", loc.get_text("welcome").c_str());

                if (ImGui::TreeNode(loc.get_text("settings").c_str()))
                {
                    // Language selection simplified
                    if (ImGui::Button("English"))
                    {
                        config.language = "en";
                        loc.set_language("en");
                        cfg_mgr.save();
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("简体中文"))
                    {
                        config.language = "zh_cn";
                        loc.set_language("zh_cn");
                        cfg_mgr.save();
                    }
                    ImGui::TreePop();
                }

                ImGui::Separator();

                ImGui::Text("%s: %d", loc.get_text("slot").c_str(), config.current_slot_index);

                // Slot selector (simplified)
                if (ImGui::Button("-"))
                {
                    if (config.current_slot_index > 1)
                    {
                        config.current_slot_index--;
                        cfg_mgr.save();
                    }
                }
                ImGui::SameLine();
                if (ImGui::Button("+"))
                {
                    if (config.current_slot_index < 10)
                    {
                        config.current_slot_index++;
                        cfg_mgr.save();
                    }
                }

                if (ImGui::Button(loc.get_text("save_pos").c_str()))
                {
                    core::EventBus::get().publish({core::EventType::SavePosition, config.current_slot_index});
                }
                ImGui::SameLine();
                if (ImGui::Button(loc.get_text("teleport").c_str()))
                {
                    core::EventBus::get().publish({core::EventType::Teleport, config.current_slot_index});
                }
            }
            ImGui::End();
        }

    } // namespace ui
} // namespace vspp
