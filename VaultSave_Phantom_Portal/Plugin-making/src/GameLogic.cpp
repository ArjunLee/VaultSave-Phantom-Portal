#include "GameLogic.hpp"
#include "core/Logger.hpp"
#include "config/ConfigManager.hpp"
#include "ui/UIManager.hpp"

#include <vector>
#include <string_view>
#include <optional>
#include <sstream>

#include "imgui/imgui.h" // For GetIO().DeltaTime

// --- REFramework Helpers (Moved from Main.cpp) ---
// In a full refactor, these should be in src/sdk/SDKHelper.hpp

using namespace reframework;

// --- Helper Functions ---

static std::optional<reframework::InvokeRet> try_invoke_ret(API::ManagedObject *obj, std::string_view method_name, const std::vector<void *> &args)
{
    if (obj == nullptr)
        return std::nullopt;
    auto t = obj->get_type_definition();
    if (t == nullptr)
        return std::nullopt;
    auto m = t->find_method(method_name);
    if (m == nullptr)
        return std::nullopt;
    const auto ret = m->invoke(obj, args);
    if (ret.exception_thrown)
        return std::nullopt;
    return ret;
}

static API::ManagedObject *try_invoke_object_any(API::ManagedObject *obj, std::initializer_list<std::string_view> method_names)
{
    for (const auto &method_name : method_names)
    {
        const auto ret = try_invoke_ret(obj, method_name, {});
        if (ret && ret->ptr != nullptr)
            return (API::ManagedObject *)ret->ptr;
    }
    return nullptr;
}

static bool try_invoke_void_1_any(API::ManagedObject *obj, void *arg0, std::initializer_list<std::string_view> method_names)
{
    if (obj == nullptr)
        return false;
    std::vector<void *> args{};
    args.push_back(arg0);
    for (const auto &method_name : method_names)
    {
        const auto ret = try_invoke_ret(obj, method_name, args);
        if (ret)
            return true;
    }
    return false;
}

static API::ManagedObject *find_managed_singleton_fallback(std::string_view preferred_full_name, std::string_view substring_hint)
{
    auto &api = API::get();
    auto direct = api->get_managed_singleton(preferred_full_name);
    if (direct != nullptr)
        return direct;

    const auto singletons = api->get_managed_singletons();
    for (const auto &s : singletons)
    {
        if (s.instance == nullptr || s.t == nullptr)
            continue;
        const auto t = (API::TypeDefinition *)s.t;
        const auto full_name = t->get_full_name();
        if (full_name == preferred_full_name)
            return (API::ManagedObject *)s.instance;
        if (!substring_hint.empty() && full_name.find(std::string{substring_hint}) != std::string::npos)
            return (API::ManagedObject *)s.instance;
    }
    return nullptr;
}

static API::ManagedObject *get_local_player()
{
    const char *manager_names[] = {"app.PlayerManager", "app.SurvivorManager"};
    for (const auto &name : manager_names)
    {
        auto mgr = find_managed_singleton_fallback(name, std::string_view{name}.substr(std::string_view{name}.find('.') + 1));
        if (mgr == nullptr)
            continue;
        auto player = try_invoke_object_any(mgr, {"get_Player", "get_Player()"});
        if (player != nullptr)
            return player;
        auto field_ptr = mgr->get_field<API::ManagedObject *>("_Player");
        if (field_ptr != nullptr && *field_ptr != nullptr)
            return *field_ptr;
    }
    return nullptr;
}

static API::ManagedObject *get_player_context()
{
    auto character_manager = find_managed_singleton_fallback("app.CharacterManager", "CharacterManager");
    if (character_manager == nullptr)
        return nullptr;
    return try_invoke_object_any(character_manager, {"getPlayerContextRefFast", "getPlayerContextRefFast185379", "get_PlayerContextFast", "get_PlayerContextFast185242"});
}

static API::ManagedObject *get_transform_from_player_or_context(API::ManagedObject *player_or_context)
{
    if (player_or_context == nullptr)
        return nullptr;
    auto transform = try_invoke_object_any(player_or_context, {"get_Transform", "get_Transform()"});
    if (transform != nullptr)
        return transform;
    auto game_object = try_invoke_object_any(player_or_context, {"get_GameObject", "get_GameObject()"});
    if (game_object == nullptr)
        return nullptr;
    return try_invoke_object_any(game_object, {"get_Transform", "get_Transform()"});
}

static API::ManagedObject *get_player_transform()
{
    auto ctx = get_player_context();
    auto ctx_transform = get_transform_from_player_or_context(ctx);
    if (ctx_transform != nullptr)
        return ctx_transform;
    auto player = get_local_player();
    return get_transform_from_player_or_context(player);
}

// --- Position Helpers ---

static std::optional<vspp::core::Vector3> try_get_position(API::ManagedObject *transform)
{
    if (transform == nullptr)
        return std::nullopt;
    const auto ret = try_invoke_ret(transform, "get_Position", {});
    if (!ret)
    {
        const auto ret2 = try_invoke_ret(transform, "get_Position()", {});
        if (!ret2)
            return std::nullopt;
        vspp::core::Vector3 out{};
        std::memcpy(&out, ret2->bytes.data(), sizeof(vspp::core::Vector3));
        return out;
    }
    vspp::core::Vector3 out{};
    std::memcpy(&out, ret->bytes.data(), sizeof(vspp::core::Vector3));
    return out;
}

static std::optional<vspp::core::Quaternion> try_get_rotation(API::ManagedObject *transform)
{
    if (transform == nullptr)
        return std::nullopt;
    const auto ret = try_invoke_ret(transform, "get_Rotation", {});
    if (!ret)
    {
        const auto ret2 = try_invoke_ret(transform, "get_Rotation()", {});
        if (!ret2)
            return std::nullopt;
        vspp::core::Quaternion out{};
        std::memcpy(&out, ret2->bytes.data(), sizeof(vspp::core::Quaternion));
        return out;
    }
    vspp::core::Quaternion out{};
    std::memcpy(&out, ret->bytes.data(), sizeof(vspp::core::Quaternion));
    return out;
}

static bool try_set_position(API::ManagedObject *transform, vspp::core::Vector3 *pos)
{
    return try_invoke_void_1_any(transform, pos, {"set_Position", "set_Position(via.vec3)", "set_Position(via.vec3f)"});
}

static bool try_set_rotation(API::ManagedObject *transform, vspp::core::Quaternion *rot)
{
    return try_invoke_void_1_any(transform, rot, {"set_Rotation", "set_Rotation(via.Quat)", "set_Rotation(via.quat)"});
}

static void update_context_position(const vspp::core::Vector3 &pos)
{
    auto ctx = get_player_context();
    if (ctx == nullptr)
        return;
    vspp::core::Vector3 copy = pos;
    if (try_invoke_void_1_any(ctx, &copy, {"set_PositionFast", "set_PositionFast(via.vec3)", "set_PositionFast(via.vec3f)"}))
        return;
    if (try_invoke_void_1_any(ctx, &copy, {"set_PositionFast232259", "set_PositionFast232259(via.vec3)", "set_PositionFast232259(via.vec3f)"}))
        return;

    // Fallback: Try to set position on context transform
    auto transform = get_transform_from_player_or_context(ctx);
    if (transform)
    {
        try_set_position(transform, &copy);
    }
}

// --- Noclip / Component Helpers (Simplified for Brevity but functional) ---

static API::ManagedObject *try_invoke_object_1_any(API::ManagedObject *obj, void *arg0, std::initializer_list<std::string_view> method_names)
{
    if (obj == nullptr)
        return nullptr;
    std::vector<void *> args{};
    args.push_back(arg0);
    for (const auto &method_name : method_names)
    {
        const auto ret = try_invoke_ret(obj, method_name, args);
        if (ret && ret->ptr != nullptr)
            return (API::ManagedObject *)ret->ptr;
    }
    return nullptr;
}

static API::ManagedObject *try_get_game_object_from_any(API::ManagedObject *obj)
{
    return try_invoke_object_any(obj, {"get_GameObject", "get_GameObject()"});
}

namespace vspp
{

    GameLogic::GameLogic()
    {
    }

    GameLogic::~GameLogic()
    {
        stop_noclip_teleport();
    }

    void GameLogic::initialize()
    {
        auto &bus = core::EventBus::get();
        bus.subscribe(core::EventType::SavePosition, [this](const core::Event &e)
                      { this->on_save_position(e); });
        bus.subscribe(core::EventType::Teleport, [this](const core::Event &e)
                      { this->on_teleport(e); });

        core::Logger::get().info("GameLogic initialized.");
    }

    void GameLogic::on_frame()
    {
        // Tick noclip logic
        if (m_noclip_active)
        {
            tick_noclip(ImGui::GetIO().DeltaTime);
        }
    }

    bool GameLogic::on_message(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
    {
        // Handle Shortcuts
        if (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN)
        {
            auto &config = config::ConfigManager::get().get_config();

            // Toggle UI
            if (config.toggle_ui_key.device == "keyboard" && (int)wparam == config.toggle_ui_key.code)
            {
                ui::UIManager::get().toggle_visible();
                return false; // Consume input? Maybe.
            }

            // Save
            if (config.save_key.device == "keyboard" && (int)wparam == config.save_key.code)
            {
                core::EventBus::get().publish({core::EventType::SavePosition, config.current_slot_index});
            }

            // Load
            if (config.load_key.device == "keyboard" && (int)wparam == config.load_key.code)
            {
                core::EventBus::get().publish({core::EventType::Teleport, config.current_slot_index});
            }
        }
        return true;
    }

    void GameLogic::on_save_position(const core::Event &e)
    {
        int slot_index = std::get<int>(e.data);
        auto transform = get_player_transform();
        if (!transform)
        {
            // Use debug level to avoid spamming user if they hold the key or if player is not ready
            core::Logger::get().debug("Save failed: Player not found");
            return;
        }

        auto pos = try_get_position(transform);
        auto rot = try_get_rotation(transform);

        if (pos && rot)
        {
            auto &cfg_mgr = config::ConfigManager::get();
            auto &config = cfg_mgr.get_config();

            // Ensure slots size
            if ((size_t)slot_index > config.slots.size())
            {
                config.slots.resize(slot_index);
            }

            // Convert core::Vector3 to config::Vector3 (same layout)
            config::SlotData &slot = config.slots[slot_index - 1];
            slot.position = {pos->x, pos->y, pos->z};
            slot.rotation = {rot->x, rot->y, rot->z, rot->w};

            cfg_mgr.save();
            core::Logger::get().info("Saved to slot " + std::to_string(slot_index));
        }
    }

    void GameLogic::on_teleport(const core::Event &e)
    {
        int slot_index = std::get<int>(e.data);
        auto &config = config::ConfigManager::get().get_config();

        if (slot_index < 1 || (size_t)slot_index > config.slots.size())
            return;

        const auto &slot = config.slots[slot_index - 1];
        if (slot.position && slot.rotation)
        {
            core::Vector3 p = {slot.position->x, slot.position->y, slot.position->z};
            core::Quaternion r = {slot.rotation->x, slot.rotation->y, slot.rotation->z, slot.rotation->w};

            start_noclip_teleport(p, r);
            core::Logger::get().info("Teleporting to slot " + std::to_string(slot_index));
        }
    }

    // --- Noclip Implementation ---

    void GameLogic::disable_physics_components(API::ManagedObject *game_object)
    {
        if (!game_object)
            return;

        auto tdb = API::get()->tdb();

        // Helper to check enable state and disable
        auto check_and_disable = [&](API::ManagedObject *comp)
        {
            if (!comp)
                return;
            auto enabled_ret = try_invoke_ret(comp, "get_Enabled", {});
            bool enabled = false;
            if (enabled_ret)
                std::memcpy(&enabled, enabled_ret->bytes.data(), sizeof(bool));

            if (enabled)
            {
                bool new_enabled = false;
                try_invoke_void_1_any(comp, &new_enabled, {"set_Enabled"});

                // Avoid adding duplicates to pending list
                bool found = false;
                for (auto *p : m_pending_components)
                {
                    if (p == comp)
                    {
                        found = true;
                        break;
                    }
                }
                if (!found)
                {
                    m_pending_components.push_back(comp);
                    core::Logger::get().debug("Disabled component: " + std::string(comp->get_type_definition()->get_name()));
                }
            }
        };

        // 1. via.physics.Collider
        auto collider_type = tdb->find_type("via.physics.Collider");
        if (collider_type)
        {
            std::vector<void *> args{collider_type->get_instance()};
            auto ret = try_invoke_ret(game_object, "getComponents(System.Type)", args);
            if (ret && ret->ptr)
            {
                auto list = (API::ManagedObject *)ret->ptr;
                auto count_ret = try_invoke_ret(list, "get_Count", {});
                int count = 0;
                if (count_ret && count_ret->bytes.size() >= sizeof(int))
                    std::memcpy(&count, count_ret->bytes.data(), sizeof(int));

                for (int i = 0; i < count; ++i)
                {
                    std::vector<void *> idx_args{(void *)(uintptr_t)i}; // Cast to avoid warning
                    // Actually, pass pointer to int
                    int idx = i;
                    idx_args[0] = &idx;

                    auto item_ret = try_invoke_ret(list, "get_Item", idx_args);
                    if (item_ret && item_ret->ptr)
                    {
                        auto component = (API::ManagedObject *)item_ret->ptr;
                        auto enabled_ret = try_invoke_ret(component, "get_Enabled", {});
                        bool enabled = false;
                        if (enabled_ret)
                            std::memcpy(&enabled, enabled_ret->bytes.data(), sizeof(bool));

                        if (enabled)
                        {
                            auto filter_info = try_invoke_object_any(component, {"get_FilterInfo"});
                            if (filter_info)
                            {
                                auto mask_ret = try_invoke_ret(filter_info, "get_MaskBits", {});
                                uint32_t old_mask = 0;
                                if (mask_ret)
                                    std::memcpy(&old_mask, mask_ret->bytes.data(), sizeof(uint32_t));

                                if (old_mask != 0)
                                {
                                    m_pending_filter_infos.push_back({filter_info, old_mask});
                                    uint32_t new_mask = 0;
                                    try_invoke_void_1_any(filter_info, &new_mask, {"set_MaskBits"});
                                    core::Logger::get().debug("Disabled Collider mask");
                                }
                            }
                            else
                            {
                                check_and_disable(component);
                            }
                        }
                    }
                }
            }
        }

        // 2. via.physics.CharacterController
        auto cc_type = tdb->find_type("via.physics.CharacterController");
        if (cc_type)
        {
            std::vector<void *> args{cc_type->get_instance()};
            auto ret = try_invoke_ret(game_object, "getComponent(System.Type)", args);
            if (ret && ret->ptr)
                check_and_disable((API::ManagedObject *)ret->ptr);
        }

        // 3. app.PlayerFPSMovementDriver
        auto driver_type = tdb->find_type("app.PlayerFPSMovementDriver");
        if (driver_type)
        {
            std::vector<void *> args{driver_type->get_instance()};
            auto ret = try_invoke_ret(game_object, "getComponent(System.Type)", args);
            if (ret && ret->ptr)
                check_and_disable((API::ManagedObject *)ret->ptr);
        }

        // 4. app.PlayerMovement
        auto movement_type = tdb->find_type("app.PlayerMovement");
        if (movement_type)
        {
            std::vector<void *> args{movement_type->get_instance()};
            auto ret = try_invoke_ret(game_object, "getComponents(System.Type)", args);
            if (ret && ret->ptr)
            {
                auto list = (API::ManagedObject *)ret->ptr;
                auto count_ret = try_invoke_ret(list, "get_Count", {});
                int count = 0;
                if (count_ret && count_ret->bytes.size() >= sizeof(int))
                    std::memcpy(&count, count_ret->bytes.data(), sizeof(int));

                for (int i = 0; i < count; ++i)
                {
                    int idx = i;
                    std::vector<void *> idx_args{&idx};
                    auto item_ret = try_invoke_ret(list, "get_Item", idx_args);
                    if (item_ret && item_ret->ptr)
                        check_and_disable((API::ManagedObject *)item_ret->ptr);
                }
            }
        }
    }

    void GameLogic::restore_physics_components()
    {
        core::Logger::get().debug("Restoring physics components...");
        for (auto &info : m_pending_filter_infos)
        {
            if (info.filter_info)
            {
                try_invoke_void_1_any(info.filter_info, &info.old_mask, {"set_MaskBits"});
            }
        }
        m_pending_filter_infos.clear();

        for (auto &comp : m_pending_components)
        {
            if (comp)
            {
                bool enabled = true;
                try_invoke_void_1_any(comp, &enabled, {"set_Enabled"});
            }
        }
        m_pending_components.clear();
        core::Logger::get().info("<<< Stopping noclip sequence... Done.");
    }

    void GameLogic::start_noclip_teleport(const core::Vector3 &pos, const core::Quaternion &rot)
    {
        if (m_noclip_active)
            stop_noclip_teleport();

        core::Logger::get().info(">>> Starting noclip sequence...");
        m_noclip_active = true;
        m_noclip_elapsed = 0.0f;
        m_noclip_target_pos = pos;
        m_noclip_target_rot = rot;

        // 1. Get Game Object
        auto transform = get_player_transform();
        if (!transform)
        {
            core::Logger::get().error("start_noclip: Transform not found");
            return;
        }
        auto go = try_get_game_object_from_any(transform);
        if (!go)
        {
            core::Logger::get().error("start_noclip: GameObject not found");
            return;
        }

        // 2. Disable Physics
        disable_physics_components(go);

        // 3. Set Initial Position
        vspp::core::Vector3 p = pos;
        vspp::core::Quaternion r = rot;
        try_set_position(transform, &p);
        try_set_rotation(transform, &r);
        update_context_position(p);

        // 4. Initial Warp (CharacterController)
        auto tdb = API::get()->tdb();
        auto cc_type = tdb->find_type("via.physics.CharacterController");
        if (cc_type)
        {
            std::vector<void *> args{cc_type->get_instance()};
            auto ret = try_invoke_ret(go, "getComponent(System.Type)", args);
            if (ret && ret->ptr)
            {
                auto cc = (API::ManagedObject *)ret->ptr;
                try_invoke_ret(cc, "warp", {});
            }
        }
    }

    void GameLogic::stop_noclip_teleport()
    {
        if (!m_noclip_active)
            return;
        m_noclip_active = false;
        restore_physics_components();
    }

    void GameLogic::tick_noclip(float delta_time)
    {
        m_noclip_elapsed += delta_time;
        if (m_noclip_elapsed > 0.5f) // 0.5s duration
        {
            stop_noclip_teleport();
            return;
        }

        auto transform = get_player_transform();
        if (transform)
        {
            // 1. Force Transform
            vspp::core::Vector3 p = m_noclip_target_pos;
            vspp::core::Quaternion r = m_noclip_target_rot;
            try_set_position(transform, &p);
            try_set_rotation(transform, &r);

            // 2. Force Context
            update_context_position(p);

            // 3. Force CharacterController Disable & Warp
            auto go = try_get_game_object_from_any(transform);
            if (go)
            {
                disable_physics_components(go); // Re-disable any re-enabled components

                auto tdb = API::get()->tdb();
                auto cc_type = tdb->find_type("via.physics.CharacterController");
                if (cc_type)
                {
                    std::vector<void *> args{cc_type->get_instance()};
                    auto ret = try_invoke_ret(go, "getComponent(System.Type)", args);
                    if (ret && ret->ptr)
                    {
                        auto cc = (API::ManagedObject *)ret->ptr;

                        // Force set position on CC too
                        try_invoke_void_1_any(cc, &p, {"set_Position", "set_Position(via.vec3)"});

                        // Warp
                        try_invoke_ret(cc, "warp", {});
                    }
                }
            }
        }
    }

} // namespace vspp
