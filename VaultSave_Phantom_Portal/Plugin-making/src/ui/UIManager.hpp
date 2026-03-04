#pragma once

#include <reframework/API.hpp>
#include "imgui/imgui.h"

namespace vspp
{
    namespace ui
    {

        /**
         * @brief Manages UI rendering and visibility state.
         */
        class UIManager
        {
        public:
            /**
             * @brief Get the singleton instance.
             * @return UIManager& Reference to the singleton instance.
             */
            static UIManager &get();

            /**
             * @brief Initialize the UI manager.
             */
            void initialize();

            /**
             * @brief Render the UI frame. Should be called from on_imgui_draw_ui callback.
             * @param data The REFramework ImGui frame callback data.
             */
            void render(REFImGuiFrameCbData *data);

            /**
             * @brief Toggle the visibility of the main UI window.
             */
            void toggle_visible();

            /**
             * @brief Check if the UI is currently visible.
             * @return true if visible, false otherwise.
             */
            bool is_visible() const { return m_visible; }

            /**
             * @brief Set the visibility of the main UI window.
             * @param visible The new visibility state.
             */
            void set_visible(bool visible) { m_visible = visible; }

        private:
            UIManager() = default;
            ~UIManager() = default;

            bool m_initialized{false};
            bool m_visible{false};

            void draw_main_window();
        };

    } // namespace ui
} // namespace vspp
