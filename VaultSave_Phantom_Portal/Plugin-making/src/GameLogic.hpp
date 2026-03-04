#pragma once

#include <windows.h>
#include <reframework/API.hpp>
#include "core/EventBus.hpp"
#include "core/Types.hpp"

namespace vspp
{

    /**
     * @brief Core game logic handler (Noclip, Teleport, Shortcuts).
     */
    class GameLogic
    {
    public:
        GameLogic();
        ~GameLogic();

        /**
         * @brief Initialize game logic and subscribe to events.
         */
        void initialize();

        /**
         * @brief Per-frame update logic (e.g., noclip tick).
         */
        void on_frame();

        /**
         * @brief Handle window messages for shortcuts.
         * @param hwnd Window handle.
         * @param msg Message ID.
         * @param wparam wParam.
         * @param lparam lParam.
         * @return true if message should be passed to next handler, false if consumed.
         */
        bool on_message(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

    private:
        void on_save_position(const core::Event &e);
        void on_teleport(const core::Event &e);

        // Noclip Logic (Ported from original Main.cpp)
        void tick_noclip(float delta_time);
        void start_noclip_teleport(const core::Vector3 &pos, const core::Quaternion &rot);
        void stop_noclip_teleport();
        void disable_physics_components(reframework::API::ManagedObject *game_object);
        void restore_physics_components();

        bool m_noclip_active{false};
        float m_noclip_elapsed{0.0f};
        core::Vector3 m_noclip_target_pos;
        core::Quaternion m_noclip_target_rot;

        struct PendingFilterInfo
        {
            reframework::API::ManagedObject *filter_info;
            uint32_t old_mask;
        };

        std::vector<reframework::API::ManagedObject *> m_pending_components;
        std::vector<PendingFilterInfo> m_pending_filter_infos;
    };

} // namespace vspp
