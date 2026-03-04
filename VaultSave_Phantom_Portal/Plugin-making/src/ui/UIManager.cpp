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
            m_visible = true; // Default to visible so it shows when REFramework menu is open
            core::Logger::get().info("UIManager initialized.");
        }

        void UIManager::toggle_visible()
        {
            m_visible = !m_visible;
            core::Logger::get().info("UI visibility toggled: " + std::string(m_visible ? "ON" : "OFF"));
        }

        void UIManager::render(REFImGuiFrameCbData *data)
        {
            if (!m_initialized)
                return;

            if (!data || !data->context)
                return;

            ImGui::SetCurrentContext((ImGuiContext *)data->context);
            if (!ImGui::GetCurrentContext())
                return;

            bool reframework_ui_visible = false;
            if (auto *api = reframework::API::try_get())
            {
                if (api->param()->functions->is_drawing_ui())
                {
                    reframework_ui_visible = true;
                }
            }

            if (!m_visible && !reframework_ui_visible)
                return;

            try
            {
                draw_main_window();
            }
            catch (const std::exception &e)
            {
                core::Logger::get().error("Exception in UIManager::render: " + std::string(e.what()));
            }
            catch (...)
            {
                core::Logger::get().error("Unknown exception in UIManager::render");
            }
        }

        void UIManager::draw_main_window()
        {
            auto &loc = LocalizationManager::get();
            auto &cfg_mgr = config::ConfigManager::get();
            auto &config = cfg_mgr.get_config();

            // Use a fixed ID to prevent window state reset when language changes
            std::string window_title = loc.get_text("window_title") + "###VSPP_MainWindow";

            // Set a default size for the first time
            ImGui::SetNextWindowSize(ImVec2(300, 400), ImGuiCond_FirstUseEver);

            bool open = true;
            // Pass &open to allow closing via X button if desired, though we handle visibility externally
            if (ImGui::Begin(window_title.c_str(), &open))
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
