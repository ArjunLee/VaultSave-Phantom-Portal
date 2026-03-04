#include <windows.h>
#include <reframework/API.hpp>
#include <filesystem>
#include <shlobj.h>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>
#include <cstring>
#include <limits>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_dx12.h"
#include "imgui/imgui_impl_win32.h"

#include "rendering/d3d11.hpp"
#include "rendering/d3d12.hpp"

using namespace reframework;

struct Vector3
{
    float x, y, z;
};

struct Quaternion
{
    float x, y, z, w;
};

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

struct SaveData
{
    int current_slot_index = 1;
    KeyBinding save_key = {"keyboard", 116};
    KeyBinding load_key = {"keyboard", 120};
    std::string language{"System"};
    std::vector<SlotData> slots{};
};

bool g_initialized{false};
HWND g_wnd{};
Vector3 g_saved_position = {0.0f, 0.0f, 0.0f};
bool g_has_saved_position = false;
Quaternion g_saved_rotation = {0.0f, 0.0f, 0.0f, 1.0f};
bool g_has_saved_rotation = false;
int g_current_slot_index = 1;
std::string g_status_message = "Ready";
std::vector<SlotData> g_runtime_slots{};
bool g_logged_player_lookup_failure = false;

static std::filesystem::path get_data_file_path()
{
    return std::filesystem::path{"reframework/data/VaultSavePhantomTravel/data.json"};
}

static std::filesystem::path get_debug_log_file_path()
{
    return std::filesystem::path{"reframework/logs/VaultSavePhantomTravel/VSPT-DebugLog.txt"};
}

static void ensure_data_dir_exists()
{
    const auto config_dir = std::filesystem::path{"reframework/data/VaultSavePhantomTravel"};
    if (std::filesystem::exists(config_dir))
    {
        return;
    }
    std::filesystem::create_directories(config_dir);
}

static void ensure_debug_log_dir_exists()
{
    const auto dir = std::filesystem::path{"reframework/logs/VaultSavePhantomTravel"};
    if (std::filesystem::exists(dir))
    {
        return;
    }
    std::filesystem::create_directories(dir);
}

static void write_debug_log_line(std::string_view line)
{
    ensure_debug_log_dir_exists();

    const auto path = get_debug_log_file_path();
    std::ofstream file{path, std::ios::binary | std::ios::app};
    if (!file.is_open())
    {
        return;
    }

    file.write(line.data(), (std::streamsize)line.size());
    file.write("\n", 1);
}

static std::optional<std::string> read_text_file(const std::filesystem::path &path)
{
    std::ifstream file{path, std::ios::binary};
    if (!file.is_open())
    {
        return std::nullopt;
    }

    std::ostringstream ss{};
    ss << file.rdbuf();
    return ss.str();
}

static bool write_text_file(const std::filesystem::path &path, const std::string &content)
{
    std::ofstream file{path, std::ios::binary | std::ios::trunc};
    if (!file.is_open())
    {
        return false;
    }

    file.write(content.data(), (std::streamsize)content.size());
    return file.good();
}

static size_t find_matching_bracket(std::string_view s, size_t open_pos, char open_ch, char close_ch)
{
    if (open_pos >= s.size())
    {
        return std::string_view::npos;
    }
    if (s[open_pos] != open_ch)
    {
        return std::string_view::npos;
    }

    bool in_string = false;
    bool escaped = false;
    int depth = 0;

    for (size_t i = open_pos; i < s.size(); ++i)
    {
        const char c = s[i];
        if (escaped)
        {
            escaped = false;
            continue;
        }

        if (c == '\\')
        {
            escaped = true;
            continue;
        }

        if (c == '"')
        {
            in_string = !in_string;
            continue;
        }

        if (in_string)
        {
            continue;
        }

        if (c == open_ch)
        {
            depth++;
            continue;
        }

        if (c == close_ch)
        {
            depth--;
            if (depth == 0)
            {
                return i;
            }
        }
    }

    return std::string_view::npos;
}

static std::string_view trim(std::string_view s)
{
    while (!s.empty() && (s.front() == ' ' || s.front() == '\n' || s.front() == '\r' || s.front() == '\t'))
    {
        s.remove_prefix(1);
    }

    while (!s.empty() && (s.back() == ' ' || s.back() == '\n' || s.back() == '\r' || s.back() == '\t'))
    {
        s.remove_suffix(1);
    }

    return s;
}

static std::optional<std::string_view> find_json_value_token(std::string_view json, std::string_view key_with_quotes)
{
    const auto key_pos = json.find(key_with_quotes);
    if (key_pos == std::string_view::npos)
    {
        return std::nullopt;
    }

    const auto colon_pos = json.find(':', key_pos + key_with_quotes.size());
    if (colon_pos == std::string_view::npos)
    {
        return std::nullopt;
    }

    auto rest = trim(json.substr(colon_pos + 1));
    if (rest.empty())
    {
        return std::nullopt;
    }

    if (rest.front() == '"')
    {
        bool escaped = false;
        for (size_t i = 1; i < rest.size(); ++i)
        {
            const char c = rest[i];
            if (escaped)
            {
                escaped = false;
                continue;
            }
            if (c == '\\')
            {
                escaped = true;
                continue;
            }
            if (c == '"')
            {
                return rest.substr(1, i - 1);
            }
        }
        return std::nullopt;
    }

    size_t end = 0;
    while (end < rest.size())
    {
        const char c = rest[end];
        if (c == ',' || c == '}' || c == ']' || c == '\n' || c == '\r' || c == '\t' || c == ' ')
        {
            break;
        }
        end++;
    }

    if (end == 0)
    {
        return std::nullopt;
    }

    return trim(rest.substr(0, end));
}

static std::optional<float> parse_float_token(std::string_view token)
{
    if (token.empty())
    {
        return std::nullopt;
    }

    std::string tmp{token};
    char *endptr = nullptr;
    const float v = std::strtof(tmp.c_str(), &endptr);
    if (endptr == tmp.c_str())
    {
        return std::nullopt;
    }
    return v;
}

static std::optional<int> parse_int_token(std::string_view token)
{
    if (token.empty())
    {
        return std::nullopt;
    }

    std::string tmp{token};
    char *endptr = nullptr;
    const long v = std::strtol(tmp.c_str(), &endptr, 10);
    if (endptr == tmp.c_str())
    {
        return std::nullopt;
    }

    if (v < std::numeric_limits<int>::min() || v > std::numeric_limits<int>::max())
    {
        return std::nullopt;
    }

    return (int)v;
}

static std::optional<Vector3> parse_vec3_object(std::string_view obj_json)
{
    const auto x_token = find_json_value_token(obj_json, "\"x\"");
    const auto y_token = find_json_value_token(obj_json, "\"y\"");
    const auto z_token = find_json_value_token(obj_json, "\"z\"");
    if (!x_token || !y_token || !z_token)
    {
        return std::nullopt;
    }

    const auto x = parse_float_token(*x_token);
    const auto y = parse_float_token(*y_token);
    const auto z = parse_float_token(*z_token);
    if (!x || !y || !z)
    {
        return std::nullopt;
    }

    return Vector3{*x, *y, *z};
}

static std::optional<Quaternion> parse_quat_object(std::string_view obj_json)
{
    const auto x_token = find_json_value_token(obj_json, "\"x\"");
    const auto y_token = find_json_value_token(obj_json, "\"y\"");
    const auto z_token = find_json_value_token(obj_json, "\"z\"");
    const auto w_token = find_json_value_token(obj_json, "\"w\"");
    if (!x_token || !y_token || !z_token || !w_token)
    {
        return std::nullopt;
    }

    const auto x = parse_float_token(*x_token);
    const auto y = parse_float_token(*y_token);
    const auto z = parse_float_token(*z_token);
    const auto w = parse_float_token(*w_token);
    if (!x || !y || !z || !w)
    {
        return std::nullopt;
    }

    return Quaternion{*x, *y, *z, *w};
}

static std::optional<std::string_view> slice_object_value(std::string_view json, std::string_view key_with_quotes)
{
    const auto key_pos = json.find(key_with_quotes);
    if (key_pos == std::string_view::npos)
    {
        return std::nullopt;
    }

    const auto colon_pos = json.find(':', key_pos + key_with_quotes.size());
    if (colon_pos == std::string_view::npos)
    {
        return std::nullopt;
    }

    auto rest = trim(json.substr(colon_pos + 1));
    if (rest.empty())
    {
        return std::nullopt;
    }

    if (rest.front() != '{')
    {
        return std::nullopt;
    }

    const auto end = find_matching_bracket(rest, 0, '{', '}');
    if (end == std::string_view::npos)
    {
        return std::nullopt;
    }

    return rest.substr(0, end + 1);
}

static SaveData make_default_data()
{
    SaveData data{};
    data.current_slot_index = 1;
    data.save_key = {"keyboard", 116};
    data.load_key = {"keyboard", 120};
    data.language = "System";
    data.slots.resize(3);
    data.slots[0].note = "Slot 1 note";
    data.slots[1].note = "Slot 2 note";
    data.slots[2].note = "Slot 3 note";
    return data;
}

static KeyBinding parse_key_binding_object(std::string_view obj_json, KeyBinding fallback)
{
    const auto device_token = find_json_value_token(obj_json, "\"device\"");
    if (device_token)
    {
        fallback.device = std::string{*device_token};
    }

    const auto code_token = find_json_value_token(obj_json, "\"code\"");
    if (code_token)
    {
        const auto code = parse_int_token(*code_token);
        if (code)
        {
            fallback.code = *code;
        }
    }

    return fallback;
}

static std::optional<std::string_view> get_slots_array_slice(std::string_view json)
{
    const auto slots_pos = json.find("\"slots\"");
    if (slots_pos == std::string_view::npos)
    {
        return std::nullopt;
    }

    const auto colon_pos = json.find(':', slots_pos + 7);
    if (colon_pos == std::string_view::npos)
    {
        return std::nullopt;
    }

    auto rest = trim(json.substr(colon_pos + 1));
    if (rest.empty() || rest.front() != '[')
    {
        return std::nullopt;
    }

    const auto end = find_matching_bracket(rest, 0, '[', ']');
    if (end == std::string_view::npos)
    {
        return std::nullopt;
    }

    return rest.substr(0, end + 1);
}

static std::optional<std::string_view> get_nth_object_in_array(std::string_view array_json, int index_1_based)
{
    if (index_1_based < 1)
    {
        return std::nullopt;
    }

    auto s = array_json;
    s = trim(s);
    if (s.size() < 2 || s.front() != '[')
    {
        return std::nullopt;
    }

    size_t i = 1;
    int current = 0;
    while (i < s.size())
    {
        const auto obj_start = s.find('{', i);
        if (obj_start == std::string_view::npos)
        {
            return std::nullopt;
        }

        const auto obj_end = find_matching_bracket(s, obj_start, '{', '}');
        if (obj_end == std::string_view::npos)
        {
            return std::nullopt;
        }

        current++;
        if (current == index_1_based)
        {
            return s.substr(obj_start, obj_end - obj_start + 1);
        }

        i = obj_end + 1;
    }

    return std::nullopt;
}

static SlotData parse_slot_object(std::string_view slot_json, int slot_index_1_based)
{
    SlotData slot{};

    const auto note_token = find_json_value_token(slot_json, "\"note\"");
    if (note_token)
    {
        slot.note = std::string{*note_token};
    }
    else
    {
        slot.note = "Slot " + std::to_string(slot_index_1_based) + " note";
    }

    const auto pos_obj = slice_object_value(slot_json, "\"position\"");
    if (pos_obj)
    {
        slot.position = parse_vec3_object(*pos_obj);
    }

    const auto rot_obj = slice_object_value(slot_json, "\"rotation\"");
    if (rot_obj)
    {
        slot.rotation = parse_quat_object(*rot_obj);
    }

    return slot;
}

static SaveData load_persistent_data()
{
    ensure_data_dir_exists();
    const auto path = get_data_file_path();
    const auto txt = read_text_file(path);
    if (!txt)
    {
        auto data = make_default_data();
        return data;
    }

    const std::string_view json{*txt};
    auto data = make_default_data();

    const auto slot_index_token = find_json_value_token(json, "\"current_slot_index\"");
    if (slot_index_token)
    {
        std::string tmp{*slot_index_token};
        try
        {
            data.current_slot_index = std::stoi(tmp);
        }
        catch (...)
        {
        }
    }

    const auto language_token = find_json_value_token(json, "\"language\"");
    if (language_token)
    {
        data.language = std::string{*language_token};
    }

    const auto save_key_obj = slice_object_value(json, "\"save_key\"");
    if (save_key_obj)
    {
        data.save_key = parse_key_binding_object(*save_key_obj, data.save_key);
    }

    const auto load_key_obj = slice_object_value(json, "\"load_key\"");
    if (load_key_obj)
    {
        data.load_key = parse_key_binding_object(*load_key_obj, data.load_key);
    }

    const auto slots_slice = get_slots_array_slice(json);
    if (!slots_slice)
    {
        return data;
    }

    std::vector<SlotData> slots{};
    for (int i = 1; i <= 10; ++i)
    {
        const auto slot_obj = get_nth_object_in_array(*slots_slice, i);
        if (!slot_obj)
        {
            break;
        }
        slots.push_back(parse_slot_object(*slot_obj, i));
    }

    if (slots.size() < 3)
    {
        while (slots.size() < 3)
        {
            const int idx = (int)slots.size() + 1;
            SlotData s{};
            s.note = "Slot " + std::to_string(idx) + " note";
            slots.push_back(s);
        }
    }

    data.slots = std::move(slots);
    return data;
}

static std::string format_float_9(float v)
{
    std::ostringstream ss{};
    ss.setf(std::ios::fixed);
    ss.precision(9);
    ss << v;
    return ss.str();
}

static std::string serialize_persistent_data(const SaveData &data)
{
    std::ostringstream ss{};
    ss << "{\n";
    ss << "  \"current_slot_index\": " << data.current_slot_index << ",\n";
    ss << "  \"save_key\": { \"device\": \"" << (data.save_key.device.empty() ? "keyboard" : data.save_key.device) << "\", \"code\": " << data.save_key.code << " },\n";
    ss << "  \"load_key\": { \"device\": \"" << (data.load_key.device.empty() ? "keyboard" : data.load_key.device) << "\", \"code\": " << data.load_key.code << " },\n";
    ss << "  \"language\": \"" << (data.language.empty() ? "System" : data.language) << "\",\n";
    ss << "  \"slots\": [\n";

    for (size_t i = 0; i < data.slots.size(); ++i)
    {
        const auto &slot = data.slots[i];
        ss << "    {\n";

        if (slot.position.has_value())
        {
            const auto &p = *slot.position;
            ss << "      \"position\": { \"x\": \"" << format_float_9(p.x) << "\", \"y\": \"" << format_float_9(p.y) << "\", \"z\": \"" << format_float_9(p.z) << "\" },\n";
        }
        else
        {
            ss << "      \"position\": null,\n";
        }

        if (slot.rotation.has_value())
        {
            const auto &r = *slot.rotation;
            ss << "      \"rotation\": { \"x\": \"" << format_float_9(r.x) << "\", \"y\": \"" << format_float_9(r.y) << "\", \"z\": \"" << format_float_9(r.z) << "\", \"w\": \"" << format_float_9(r.w) << "\" },\n";
        }
        else
        {
            ss << "      \"rotation\": null,\n";
        }

        ss << "      \"note\": \"" << (slot.note.empty() ? ("Slot " + std::to_string(i + 1) + " note") : slot.note) << "\"\n";
        ss << "    }";

        if (i + 1 < data.slots.size())
        {
            ss << ",";
        }
        ss << "\n";
    }

    ss << "  ]\n";
    ss << "}\n";
    return ss.str();
}

static void save_persistent_data(const SaveData &data)
{
    ensure_data_dir_exists();
    const auto path = get_data_file_path();
    const auto content = serialize_persistent_data(data);
    write_text_file(path, content);
}

static std::optional<reframework::InvokeRet> try_invoke_ret(API::ManagedObject *obj, std::string_view method_name, const std::vector<void *> &args)
{
    if (obj == nullptr)
    {
        return std::nullopt;
    }

    auto t = obj->get_type_definition();
    if (t == nullptr)
    {
        return std::nullopt;
    }

    auto m = t->find_method(method_name);
    if (m == nullptr)
    {
        return std::nullopt;
    }

    const auto ret = m->invoke(obj, args);
    if (ret.exception_thrown)
    {
        return std::nullopt;
    }

    return ret;
}

static API::ManagedObject *try_invoke_object_any(API::ManagedObject *obj, std::initializer_list<std::string_view> method_names)
{
    for (const auto &method_name : method_names)
    {
        const auto ret = try_invoke_ret(obj, method_name, {});
        if (ret && ret->ptr != nullptr)
        {
            return (API::ManagedObject *)ret->ptr;
        }
    }

    return nullptr;
}

static API::ManagedObject *find_managed_singleton_fallback(std::string_view preferred_full_name, std::string_view substring_hint)
{
    auto &api = API::get();

    auto direct = api->get_managed_singleton(preferred_full_name);
    if (direct != nullptr)
    {
        return direct;
    }

    const auto singletons = api->get_managed_singletons();
    for (const auto &s : singletons)
    {
        if (s.instance == nullptr || s.t == nullptr)
        {
            continue;
        }

        const auto t = (API::TypeDefinition *)s.t;
        const auto full_name = t->get_full_name();

        if (full_name == preferred_full_name)
        {
            return (API::ManagedObject *)s.instance;
        }

        if (!substring_hint.empty() && full_name.find(std::string{substring_hint}) != std::string::npos)
        {
            return (API::ManagedObject *)s.instance;
        }
    }

    return nullptr;
}

static void log_player_lookup_failure_once()
{
    if (g_logged_player_lookup_failure)
    {
        return;
    }

    g_logged_player_lookup_failure = true;

    const auto &api = API::get();
    if (!api)
    {
        return;
    }

    int logged = 0;
    for (const auto &s : api->get_managed_singletons())
    {
        if (logged >= 12)
        {
            break;
        }

        if (s.instance == nullptr || s.t == nullptr)
        {
            continue;
        }

        const auto t = (API::TypeDefinition *)s.t;
        const auto full_name = t->get_full_name();
        const bool maybe_relevant = full_name.find("CharacterManager") != std::string::npos ||
                                    full_name.find("PlayerManager") != std::string::npos ||
                                    full_name.find("SurvivorManager") != std::string::npos ||
                                    full_name.find("PlayerContext") != std::string::npos;
        if (!maybe_relevant)
        {
            continue;
        }

        api->log_warn("[VaultSave] singleton candidate: %s @ %p", full_name.c_str(), s.instance);
        logged++;
    }

    if (logged == 0)
    {
        api->log_warn("[VaultSave] singleton candidate: none matched Character/Player/Survivor/Context");
    }
}

static bool try_invoke_void_1_any(API::ManagedObject *obj, void *arg0, std::initializer_list<std::string_view> method_names)
{
    if (obj == nullptr)
    {
        return false;
    }

    std::vector<void *> args{};
    args.push_back(arg0);
    for (const auto &method_name : method_names)
    {
        const auto ret = try_invoke_ret(obj, method_name, args);
        if (ret)
        {
            return true;
        }
    }

    return false;
}

static std::optional<Vector3> try_get_position(API::ManagedObject *transform)
{
    if (transform == nullptr)
    {
        return std::nullopt;
    }

    const auto ret = try_invoke_ret(transform, "get_Position", {});
    if (!ret)
    {
        const auto ret2 = try_invoke_ret(transform, "get_Position()", {});
        if (!ret2)
        {
            return std::nullopt;
        }

        Vector3 out{};
        std::memcpy(&out, ret2->bytes.data(), sizeof(Vector3));
        return out;
    }

    Vector3 out{};
    std::memcpy(&out, ret->bytes.data(), sizeof(Vector3));
    return out;
}

static std::optional<Quaternion> try_get_rotation(API::ManagedObject *transform)
{
    if (transform == nullptr)
    {
        return std::nullopt;
    }

    const auto ret = try_invoke_ret(transform, "get_Rotation", {});
    if (!ret)
    {
        const auto ret2 = try_invoke_ret(transform, "get_Rotation()", {});
        if (!ret2)
        {
            return std::nullopt;
        }

        Quaternion out{};
        std::memcpy(&out, ret2->bytes.data(), sizeof(Quaternion));
        return out;
    }

    Quaternion out{};
    std::memcpy(&out, ret->bytes.data(), sizeof(Quaternion));
    return out;
}

static bool try_set_position(API::ManagedObject *transform, Vector3 *pos)
{
    return try_invoke_void_1_any(transform, pos, {"set_Position", "set_Position(via.vec3)", "set_Position(via.vec3f)"});
}

static bool try_set_rotation(API::ManagedObject *transform, Quaternion *rot)
{
    return try_invoke_void_1_any(transform, rot, {"set_Rotation", "set_Rotation(via.Quat)", "set_Rotation(via.quat)"});
}

static API::ManagedObject *get_player_context()
{
    auto &api = API::get();
    auto character_manager = find_managed_singleton_fallback("app.CharacterManager", "CharacterManager");
    if (character_manager == nullptr)
    {
        return nullptr;
    }

    return try_invoke_object_any(character_manager, {"getPlayerContextRefFast", "getPlayerContextRefFast185379", "get_PlayerContextFast", "get_PlayerContextFast185242"});
}

static API::ManagedObject *get_local_player()
{
    auto &api = API::get();
    const char *manager_names[] = {"app.PlayerManager", "app.SurvivorManager"};

    for (const auto &name : manager_names)
    {
        auto mgr = find_managed_singleton_fallback(name, std::string_view{name}.substr(std::string_view{name}.find('.') + 1));
        if (mgr == nullptr)
        {
            continue;
        }

        auto player = try_invoke_object_any(mgr, {"get_Player", "get_Player()"});
        if (player != nullptr)
        {
            return player;
        }

        auto field_ptr = mgr->get_field<API::ManagedObject *>("_Player");
        if (field_ptr != nullptr && *field_ptr != nullptr)
        {
            return *field_ptr;
        }
    }

    return nullptr;
}

static API::ManagedObject *get_transform_from_player_or_context(API::ManagedObject *player_or_context)
{
    if (player_or_context == nullptr)
    {
        return nullptr;
    }

    auto transform = try_invoke_object_any(player_or_context, {"get_Transform", "get_Transform()"});
    if (transform != nullptr)
    {
        return transform;
    }

    auto game_object = try_invoke_object_any(player_or_context, {"get_GameObject", "get_GameObject()"});
    if (game_object == nullptr)
    {
        return nullptr;
    }

    return try_invoke_object_any(game_object, {"get_Transform", "get_Transform()"});
}

static void update_context_position(const Vector3 &pos)
{
    auto ctx = get_player_context();
    if (ctx == nullptr)
    {
        return;
    }

    Vector3 copy = pos;
    if (try_invoke_void_1_any(ctx, &copy, {"set_PositionFast", "set_PositionFast(via.vec3)", "set_PositionFast(via.vec3f)"}))
    {
        return;
    }

    try_invoke_void_1_any(ctx, &copy, {"set_PositionFast232259", "set_PositionFast232259(via.vec3)", "set_PositionFast232259(via.vec3f)"});
}

API::ManagedObject *get_player_transform()
{
    auto ctx = get_player_context();
    auto ctx_transform = get_transform_from_player_or_context(ctx);
    if (ctx_transform != nullptr)
    {
        return ctx_transform;
    }

    auto player = get_local_player();
    return get_transform_from_player_or_context(player);
}

static bool save_to_current_slot()
{
    auto transform = get_player_transform();
    if (transform == nullptr)
    {
        g_status_message = "Error: Player not found";
        log_player_lookup_failure_once();
        write_debug_log_line("[VaultSave] save failed: player not found");
        return false;
    }

    const auto pos = try_get_position(transform);
    const auto rot = try_get_rotation(transform);
    if (!pos || !rot)
    {
        g_status_message = "Error: Failed to read position/rotation";
        return false;
    }

    g_saved_position = *pos;
    g_saved_rotation = *rot;
    g_has_saved_position = true;
    g_has_saved_rotation = true;

    auto data = load_persistent_data();
    data.current_slot_index = g_current_slot_index;

    const int slot_index = g_current_slot_index < 1 ? 1 : g_current_slot_index;
    if ((size_t)slot_index > data.slots.size())
    {
        data.slots.resize((size_t)slot_index);
    }

    auto &slot = data.slots[(size_t)slot_index - 1];
    slot.position = *pos;
    slot.rotation = *rot;
    if (slot.note.empty())
    {
        slot.note = "Slot " + std::to_string(slot_index) + " note";
    }

    save_persistent_data(data);

    if ((size_t)slot_index > g_runtime_slots.size())
    {
        g_runtime_slots.resize((size_t)slot_index);
    }
    g_runtime_slots[(size_t)slot_index - 1].position = *pos;
    g_runtime_slots[(size_t)slot_index - 1].rotation = *rot;

    g_status_message = "Saved to Slot " + std::to_string(slot_index);
    API::get()->log_info("[VaultSave] Saved slot %d pos(%.3f,%.3f,%.3f)", slot_index, pos->x, pos->y, pos->z);
    {
        const std::string msg = "[VaultSave] Saved slot " + std::to_string(slot_index);
        write_debug_log_line(msg);
    }
    return true;
}

static bool load_from_current_slot()
{
    auto data = load_persistent_data();
    const int slot_index = g_current_slot_index < 1 ? 1 : g_current_slot_index;
    std::optional<Vector3> target_pos{};
    std::optional<Quaternion> target_rot{};

    if ((size_t)slot_index <= g_runtime_slots.size())
    {
        const auto &runtime = g_runtime_slots[(size_t)slot_index - 1];
        if (runtime.position && runtime.rotation)
        {
            target_pos = runtime.position;
            target_rot = runtime.rotation;
        }
    }

    if (!target_pos || !target_rot)
    {
        if (slot_index > (int)data.slots.size())
        {
            g_status_message = "Slot Empty";
            write_debug_log_line("[VaultSave] load failed: slot empty");
            return false;
        }

        const auto &slot = data.slots[(size_t)slot_index - 1];
        if (!slot.position || !slot.rotation)
        {
            g_status_message = "Slot Empty";
            write_debug_log_line("[VaultSave] load failed: slot empty");
            return false;
        }

        target_pos = slot.position;
        target_rot = slot.rotation;
    }

    auto transform = get_player_transform();
    if (transform == nullptr)
    {
        g_status_message = "Error: Player not found";
        log_player_lookup_failure_once();
        write_debug_log_line("[VaultSave] load failed: player not found");
        return false;
    }

    Vector3 pos = *target_pos;
    Quaternion rot = *target_rot;

    const bool ok_pos = try_set_position(transform, &pos);
    const bool ok_rot = try_set_rotation(transform, &rot);
    update_context_position(pos);

    if (!ok_pos || !ok_rot)
    {
        g_status_message = "Error: Teleport failed";
        write_debug_log_line("[VaultSave] load failed: teleport failed");
        return false;
    }

    g_saved_position = pos;
    g_saved_rotation = rot;
    g_has_saved_position = true;
    g_has_saved_rotation = true;
    g_status_message = "Teleport Done!";
    API::get()->log_info("[VaultSave] Loaded slot %d pos(%.3f,%.3f,%.3f)", slot_index, pos.x, pos.y, pos.z);
    {
        const std::string msg = "[VaultSave] Loaded slot " + std::to_string(slot_index);
        write_debug_log_line(msg);
    }
    return true;
}

void save_position()
{
    save_to_current_slot();
}

void teleport_to_saved()
{
    load_from_current_slot();
}

bool initialize_imgui()
{
    if (g_initialized)
    {
        return true;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui::GetIO().IniFilename = "VaultSave_PhantomPortal_ui.ini";

    const auto renderer_data = API::get()->param()->renderer_data;

    DXGI_SWAP_CHAIN_DESC swap_desc{};
    auto swapchain = (IDXGISwapChain *)renderer_data->swapchain;
    swapchain->GetDesc(&swap_desc);

    g_wnd = swap_desc.OutputWindow;

    if (!ImGui_ImplWin32_Init(g_wnd))
    {
        return false;
    }

    if (renderer_data->renderer_type == REFRAMEWORK_RENDERER_D3D11)
    {
        if (!g_d3d11.initialize())
        {
            return false;
        }
    }
    else if (renderer_data->renderer_type == REFRAMEWORK_RENDERER_D3D12)
    {
        if (!g_d3d12.initialize())
        {
            return false;
        }
    }

    g_initialized = true;
    return true;
}

void on_present()
{
    if (!g_initialized)
    {
        if (!initialize_imgui())
        {
            return;
        }
    }

    const auto renderer_data = API::get()->param()->renderer_data;

    if (renderer_data->renderer_type == REFRAMEWORK_RENDERER_D3D11)
    {
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // UI Logic Here
        if (ImGui::Begin("VaultSave Phantom Portal"))
        {
            ImGui::Text("Welcome to Phantom Portal!");
            ImGui::Text("Version: 1.0.0 (Pre-Alpha)");

            ImGui::Text("Status: %s", g_status_message.c_str());

            int slot_index = g_current_slot_index;
            if (ImGui::InputInt("Slot", &slot_index))
            {
                if (slot_index < 1)
                {
                    slot_index = 1;
                }
                if (slot_index > 10)
                {
                    slot_index = 10;
                }
                g_current_slot_index = slot_index;
            }

            if (ImGui::Button("Save Position"))
            {
                save_position();
            }

            ImGui::SameLine();

            if (ImGui::Button("Teleport"))
            {
                teleport_to_saved();
            }

            if (g_has_saved_position)
            {
                ImGui::Text("Saved: %.2f, %.2f, %.2f", g_saved_position.x, g_saved_position.y, g_saved_position.z);
            }
        }
        ImGui::End();

        ImGui::EndFrame();
        ImGui::Render();

        g_d3d11.render_imgui();
    }
    else if (renderer_data->renderer_type == REFRAMEWORK_RENDERER_D3D12)
    {
        auto command_queue = (ID3D12CommandQueue *)renderer_data->command_queue;

        if (command_queue == nullptr)
        {
            return;
        }

        ImGui_ImplDX12_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // UI Logic Here
        if (ImGui::Begin("VaultSave Phantom Portal"))
        {
            ImGui::Text("Welcome to Phantom Portal!");
            ImGui::Text("Version: 1.0.0 (Pre-Alpha)");

            ImGui::Text("Status: %s", g_status_message.c_str());

            int slot_index = g_current_slot_index;
            if (ImGui::InputInt("Slot", &slot_index))
            {
                if (slot_index < 1)
                {
                    slot_index = 1;
                }
                if (slot_index > 10)
                {
                    slot_index = 10;
                }
                g_current_slot_index = slot_index;
            }

            if (ImGui::Button("Save Position"))
            {
                save_position();
            }

            ImGui::SameLine();

            if (ImGui::Button("Teleport"))
            {
                teleport_to_saved();
            }

            if (g_has_saved_position)
            {
                ImGui::Text("Saved: %.2f, %.2f, %.2f", g_saved_position.x, g_saved_position.y, g_saved_position.z);
            }
        }
        ImGui::End();

        ImGui::EndFrame();
        ImGui::Render();

        g_d3d12.render_imgui();
    }
}

void on_device_reset()
{
    const auto renderer_data = API::get()->param()->renderer_data;

    if (renderer_data->renderer_type == REFRAMEWORK_RENDERER_D3D11)
    {
        ImGui_ImplDX11_Shutdown();
        g_d3d11 = {};
    }

    if (renderer_data->renderer_type == REFRAMEWORK_RENDERER_D3D12)
    {
        ImGui_ImplDX12_Shutdown();
        g_d3d12 = {};
    }

    g_initialized = false;
}

bool on_message(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam);

    return !ImGui::GetIO().WantCaptureMouse && !ImGui::GetIO().WantCaptureKeyboard;
}

extern "C" __declspec(dllexport) bool reframework_plugin_initialize(const REFrameworkPluginInitializeParam *param)
{
    API::initialize(param);
    ImGui::CreateContext();

    const auto functions = param->functions;
    functions->on_present(on_present);
    functions->on_device_reset(on_device_reset);
    functions->on_message((REFOnMessageCb)on_message);

    // Output initialization log
    const auto &api = reframework::API::get();
    if (api)
    {
        api->log_info("[VaultSave] --------------------------------------------------");
        api->log_info("[VaultSave] Plugin Loaded Successfully!");
        api->log_info("[VaultSave] Version: 1.0.0 (Pre-Alpha)");
        api->log_info("[VaultSave] Phantom Portal Core Started - Awaiting Instructions");
        api->log_info("[VaultSave] --------------------------------------------------");

        // Create config directory
        try
        {
            std::filesystem::path config_dir = "reframework/data/VaultSavePhantomTravel";
            if (!std::filesystem::exists(config_dir))
            {
                if (std::filesystem::create_directories(config_dir))
                {
                    api->log_info("[VaultSave] Config directory created: %s", config_dir.string().c_str());
                }
                else
                {
                    api->log_warn("[VaultSave] Failed to create config directory (or it already exists).");
                }
            }
            else
            {
                api->log_info("[VaultSave] Config directory exists.");
            }
        }
        catch (const std::exception &e)
        {
            api->log_error("[VaultSave] Error creating directory: %s", e.what());
        }

        const auto data = load_persistent_data();
        g_current_slot_index = data.current_slot_index < 1 ? 1 : data.current_slot_index;
        if (g_current_slot_index > 10)
        {
            g_current_slot_index = 10;
        }
        g_runtime_slots.clear();
        g_runtime_slots.resize(data.slots.size());
        write_debug_log_line("[VaultSave] Plugin loaded");
    }
    return true;
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
