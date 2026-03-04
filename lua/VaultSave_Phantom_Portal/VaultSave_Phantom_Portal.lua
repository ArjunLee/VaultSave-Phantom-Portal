local api = {}

local PROGRAM_NAME = "VaultSavePhantomTravel"
-- REFramework json module uses "reframework/data" as base
local RELATIVE_DATA_PATH = PROGRAM_NAME .. "\\data.json" 
-- Lua io module uses Game Root as base, so we need full relative path
local LOG_FILE_PATH = PROGRAM_NAME .. "\\VSPT-DebugLog.txt"

-- i18n Configuration
local i18n = {
    en = {
        window_title = "VaultSave: Phantom Portal",
        settings = "Settings",
        language = "Language",
        follow_system = "Follow System",
        bind_instruction = "Click button to bind. Press desired key. ESC to cancel.",
        gamepad_instruction = "Gamepad: Must hold 3+ buttons simultaneously!",
        save_key_prefix = "Save Key: ",
        load_key_prefix = "Load Key: ",
        holding_buttons = "Holding %d buttons...",
        press_key_combo = "Press Key / Hold Gamepad Combo...",
        reset_defaults = "Reset to Defaults (F5/F9)",
        keys_reset_msg = "Keys Reset to F5/F9",
        select_slot_label = "Select Slot (Check to activate):",
        slot_prefix = "Slot %d",
        slot_note_hint = "Slot %d note",
        delete_error_min_slots = "Cannot delete: Minimum 3 slots required!",
        add_slot_btn = "Add Slot (+)",
        max_slots_reached = "Max slots reached (10)",
        save_to_slot_btn = "Save to Slot ",
        load_from_slot_btn = "Load from Slot ",
        cached_pos_prefix = "Cached Pos: ",
        slot_empty = "Slot Empty",
        teleport_done = "Teleport Done!",
        teleporting = "Teleporting...",
        slot_empty_msg = "Slot Empty!",
        error_invalid_data = "Error: Invalid Data",
        binding_cancelled = "Binding Cancelled",
        bound_gamepad = "Bound to Gamepad Combo",
        error_gamepad_min_buttons = "ERROR: Gamepad requires 3+ buttons!",
        bound_keyboard = "Bound to Keyboard: ",
        saved_to = "Saved to ",
        player_not_found = "Player not found!"
    },
    zh_cn = {
        window_title = "VaultSave: 幻影传送门",
        settings = "设置",
        language = "语言",
        follow_system = "跟随系统",
        bind_instruction = "点击按钮绑定。按下按键。ESC 取消。",
        gamepad_instruction = "手柄：必须同时按住 3 个以上按钮！",
        save_key_prefix = "存档键: ",
        load_key_prefix = "读档键: ",
        holding_buttons = "按住了 %d 个按钮...",
        press_key_combo = "按下按键 / 按住手柄组合...",
        reset_defaults = "还原默认 (F5/F9)",
        keys_reset_msg = "按键已重置为 F5/F9",
        select_slot_label = "选择槽位 (勾选激活):",
        slot_prefix = "槽位 %d",
        slot_note_hint = "槽位 %d 备注",
        delete_error_min_slots = "无法删除：最少保留 3 个槽位！",
        add_slot_btn = "新增槽位 (+)",
        max_slots_reached = "已达上限 (10)",
        save_to_slot_btn = "保存到 槽位 ",
        load_from_slot_btn = "读取自 槽位 ",
        cached_pos_prefix = "缓存坐标: ",
        slot_empty = "空槽位",
        teleport_done = "传送完成！",
        teleporting = "传送中...",
        slot_empty_msg = "空槽位!",
        error_invalid_data = "错误：无效数据",
        binding_cancelled = "绑定已取消",
        bound_gamepad = "已绑定手柄组合",
        error_gamepad_min_buttons = "错误：手柄需要 3+ 个按钮！",
        bound_keyboard = "已绑定键盘: ",
        saved_to = "已保存至 ",
        player_not_found = "未找到玩家！"
    },
    zh_tw = {
        window_title = "VaultSave: 幻影傳送門",
        settings = "設定",
        language = "語言",
        follow_system = "跟隨系統",
        bind_instruction = "點擊按鈕綁定。按下按鍵。ESC 取消。",
        gamepad_instruction = "手柄：必須同時按住 3 個以上按鈕！",
        save_key_prefix = "存檔鍵: ",
        load_key_prefix = "讀檔鍵: ",
        holding_buttons = "按住了 %d 個按鈕...",
        press_key_combo = "按下按鍵 / 按住手柄組合...",
        reset_defaults = "還原預設 (F5/F9)",
        keys_reset_msg = "按鍵已重置為 F5/F9",
        select_slot_label = "選擇槽位 (勾選激活):",
        slot_prefix = "槽位 %d",
        slot_note_hint = "槽位 %d 備註",
        delete_error_min_slots = "無法刪除：最少保留 3 個槽位！",
        add_slot_btn = "新增槽位 (+)",
        max_slots_reached = "已達上限 (10)",
        save_to_slot_btn = "保存到 槽位 ",
        load_from_slot_btn = "讀取自 槽位 ",
        cached_pos_prefix = "緩存座標: ",
        slot_empty = "空槽位",
        teleport_done = "傳送完成！",
        teleporting = "傳送中...",
        slot_empty_msg = "空槽位!",
        error_invalid_data = "錯誤：無效數據",
        binding_cancelled = "綁定已取消",
        bound_gamepad = "已綁定手柄組合",
        error_gamepad_min_buttons = "錯誤：手柄需要 3+ 個按鈕！",
        bound_keyboard = "已綁定鍵盤: ",
        saved_to = "已保存至 ",
        player_not_found = "未找到玩家！"
    },
    ja = {
        window_title = "VaultSave: Phantom Portal",
        settings = "設定",
        language = "言語",
        follow_system = "システムに従う",
        bind_instruction = "ボタンをクリックしてバインド。キーを押す。ESCでキャンセル。",
        gamepad_instruction = "パッド：3つ以上のボタンを同時押し！",
        save_key_prefix = "セーブキー: ",
        load_key_prefix = "ロードキー: ",
        holding_buttons = "%d 個のボタンを押しています...",
        press_key_combo = "キーを押す / パッド同時押し...",
        reset_defaults = "初期設定に戻す (F5/F9)",
        keys_reset_msg = "キーをF5/F9にリセットしました",
        select_slot_label = "スロット選択 (チェックで有効化):",
        slot_prefix = "スロット %d",
        slot_note_hint = "スロット %d メモ",
        delete_error_min_slots = "削除不可：最低3スロット必要です！",
        add_slot_btn = "スロット追加 (+)",
        max_slots_reached = "上限到達 (10)",
        save_to_slot_btn = "スロットに保存 ",
        load_from_slot_btn = "スロットからロード ",
        cached_pos_prefix = "座標: ",
        slot_empty = "空きスロット",
        teleport_done = "テレポート完了！",
        teleporting = "テレポート中...",
        slot_empty_msg = "空きスロット!",
        error_invalid_data = "エラー：無効なデータ",
        binding_cancelled = "バインドキャンセル",
        bound_gamepad = "パッド設定完了",
        error_gamepad_min_buttons = "エラー：パッドは3ボタン以上必要！",
        bound_keyboard = "キーボード設定完了: ",
        saved_to = "保存完了: ",
        player_not_found = "プレイヤーが見つかりません！"
    },
    ko = {
        window_title = "VaultSave: Phantom Portal",
        settings = "설정",
        language = "언어",
        follow_system = "시스템 설정 따르기",
        bind_instruction = "버튼을 클릭하여 바인딩. 키를 누르세요. ESC 취소.",
        gamepad_instruction = "게임패드: 3개 이상의 버튼을 동시에 누르세요!",
        save_key_prefix = "저장 키: ",
        load_key_prefix = "로드 키: ",
        holding_buttons = "%d 버튼 누름...",
        press_key_combo = "키 누르기 / 게임패드 콤보...",
        reset_defaults = "기본값 복원 (F5/F9)",
        keys_reset_msg = "키가 F5/F9로 초기화됨",
        select_slot_label = "슬롯 선택 (체크하여 활성화):",
        slot_prefix = "슬롯 %d",
        slot_note_hint = "슬롯 %d 메모",
        delete_error_min_slots = "삭제 불가: 최소 3개 슬롯 필요!",
        add_slot_btn = "슬롯 추가 (+)",
        max_slots_reached = "최대 슬롯 도달 (10)",
        save_to_slot_btn = "저장: 슬롯 ",
        load_from_slot_btn = "로드: 슬롯 ",
        cached_pos_prefix = "저장된 좌표: ",
        slot_empty = "빈 슬롯",
        teleport_done = "텔레포트 완료!",
        teleporting = "텔레포트 중...",
        slot_empty_msg = "빈 슬롯!",
        error_invalid_data = "오류: 잘못된 데이터",
        binding_cancelled = "바인딩 취소됨",
        bound_gamepad = "게임패드 콤보 바인딩됨",
        error_gamepad_min_buttons = "오류: 게임패드는 3개 이상의 버튼 필요!",
        bound_keyboard = "키보드 바인딩됨: ",
        saved_to = "저장됨: ",
        player_not_found = "플레이어를 찾을 수 없음!"
    }
}

local current_language = "en"
local system_language_cache = nil

-- Language Detection Helpers (Adapted from re9_inf_items.lua)
local function get_system_language()
    if system_language_cache then return system_language_cache end

    local mgr = sdk.get_managed_singleton("app.GameOptionManager")
    if not mgr then return "English" end

    local managed_values = mgr:get_field("_ManagedOptionValues")
    if not managed_values then return "English" end

    local t = sdk.find_type_definition("app.GameOptionID")
    if not t then return "English" end

    local values = t:get_field("<Values>k__BackingField"):get_data(nil)
    if not values then return "English" end

    local index = nil
    local count = values:call("get_Count()")
    
    -- Find TextLanguage index
    for i = 0, count - 1 do
        local item = values:call("get_Item(System.Int32)", i)
        local ok, name = pcall(function() return item:call("ToString()") end)
        if ok and name == "TextLanguage" then
            index = i
            break
        end
    end

    if not index then return "English" end

    local ok, val = pcall(function() return managed_values:call("Get", index) end)
    if ok and val then
        system_language_cache = val:get_field("_FixedValueStr") or "English"
        return system_language_cache
    end
    
    return "English"
end

local function update_language(cfg_lang)
    local target = cfg_lang or "System"
    
    if target == "System" then
        local sys = get_system_language()
        if sys == "SimplelifiedChinese" then current_language = "zh_cn"
        elseif sys == "TransitionalChinese" then current_language = "zh_tw"
        elseif sys == "Japanese" then current_language = "ja"
        elseif sys == "Korean" then current_language = "ko"
        else current_language = "en" end
    else
        current_language = target
    end
end

local function get_text(key)
    if not i18n[current_language] then return i18n["en"][key] or key end
    return i18n[current_language][key] or i18n["en"][key] or key
end

local snapshot = nil
local last_action_time = 0
local message_timer = 0
local current_message = ""

local function write_log(text)
    -- Ensure directory exists implicitly by path structure (Lua io won't create it, but REFramework environment usually handles the data folder)
    -- If io.open fails, we can't do much, but we silence the error to avoid UI spam.
    local f = io.open(LOG_FILE_PATH, "a+")
    if f then
        f:write("[" .. os.date("%Y-%m-%d %H:%M:%S") .. "] " .. text .. "\n")
        f:close()
    end
    -- STRICTLY NO log.info to REFramework console
end

local VK_F5 = 116
local VK_F9 = 120

-- Key Mapping Table
local KBKeys = {
    KEY_0 = 0x30, KEY_1 = 0x31, KEY_2 = 0x32, KEY_3 = 0x33, KEY_4 = 0x34,
    KEY_5 = 0x35, KEY_6 = 0x36, KEY_7 = 0x37, KEY_8 = 0x38, KEY_9 = 0x39,
    A = 0x41, B = 0x42, C = 0x43, D = 0x44, E = 0x45, F = 0x46, G = 0x47,
    H = 0x48, I = 0x49, J = 0x4A, K = 0x4B, L = 0x4C, M = 0x4D, N = 0x4E,
    O = 0x4F, P = 0x50, Q = 0x51, R = 0x52, S = 0x53, T = 0x54, U = 0x55,
    V = 0x56, W = 0x57, X = 0x58, Y = 0x59, Z = 0x5A,
    F1 = 0x70, F2 = 0x71, F3 = 0x72, F4 = 0x73, F5 = 0x74, F6 = 0x75,
    F7 = 0x76, F8 = 0x77, F9 = 0x78, F10 = 0x79, F11 = 0x7A, F12 = 0x7B,
    NUMPAD_0 = 0x60, NUMPAD_1 = 0x61, NUMPAD_2 = 0x62, NUMPAD_3 = 0x63,
    NUMPAD_4 = 0x64, NUMPAD_5 = 0x65, NUMPAD_6 = 0x66, NUMPAD_7 = 0x67,
    NUMPAD_8 = 0x68, NUMPAD_9 = 0x69,
    SHIFT = 0x10, CTRL = 0x11, ALT = 0x12,
    LEFT = 0x25, UP = 0x26, RIGHT = 0x27, DOWN = 0x28,
    INSERT = 0x2D, DELETE = 0x2E, HOME = 0x24, END = 0x23,
    PAGE_UP = 0x21, PAGE_DOWN = 0x22,
    ENTER = 0x0D, ESC = 0x1B, TAB = 0x09, SPACE = 0x20, BACKSPACE = 0x08,
}

local VK_Code_To_Name = {}
for name, code in pairs(KBKeys) do VK_Code_To_Name[code] = name end

local function get_key_name(code)
    return VK_Code_To_Name[code] or string.format("VK_%d", code)
end

-- Binding State
local binding_mode = nil 

-- Default Config
local default_config = {
    save_key = { device = "keyboard", code = VK_F5 },
    load_key = { device = "keyboard", code = VK_F9 },
    current_slot_index = 1,
    language = "System"
}

local default_slots = {
    { position = nil, rotation = nil, note = "Slot 1 note" },
    { position = nil, rotation = nil, note = "Slot 2 note" },
    { position = nil, rotation = nil, note = "Slot 3 note" }
}

local max_slots = 10


-- Runtime cache for high-precision in-session restoration (mirrors Snapshot_Mod_NoError.lua behavior)
local runtime_slots = {}

local config = default_config
local slots = default_slots

local function save_data()
    local data = {
        current_slot_index = config.current_slot_index,
        save_key = config.save_key,
        load_key = config.load_key,
        language = config.language,
        slots = slots
    }
    
    if json then
        -- Attempt to dump to the relative path
        -- REFramework should handle the directory creation for the path provided
        local success = json.dump_file(RELATIVE_DATA_PATH, data)
        if not success then
            write_log("Error: json.dump_file returned false for path: " .. RELATIVE_DATA_PATH)
        end
    else
        write_log("Error: json module not available for saving")
    end
end

local function load_data()
    if not json then 
        write_log("Error: json module not available for loading")
        return 
    end

    local data = json.load_file(RELATIVE_DATA_PATH)
    if data then
        if data.current_slot_index then config.current_slot_index = data.current_slot_index end
        if data.save_key then config.save_key = data.save_key end
        if data.load_key then config.load_key = data.load_key end
        if data.language then config.language = data.language end
        if data.slots then 
            -- Reconstruct slots from saved data to support dynamic count
            slots = {}
            for i, loaded_slot in ipairs(data.slots) do
                table.insert(slots, {
                    position = loaded_slot.position,
                    rotation = loaded_slot.rotation,
                    note = loaded_slot.note or ("Slot " .. i .. " note") -- Fallback note
                })
            end
            
            -- Ensure minimum 3 slots (default behavior)
            while #slots < 3 do
                table.insert(slots, { note = "Slot " .. (#slots + 1) .. " note" })
            end
        end
        update_language(config.language)
        write_log("Data loaded from " .. RELATIVE_DATA_PATH)
    else
        write_log("No data found at " .. RELATIVE_DATA_PATH .. " (First run or path error). Creating default config.")
        save_data() -- Create directory and default file
        update_language(config.language)
    end
end

-- Load data on startup
load_data()

local key_state = {}

-- Gamepad Button Enum
local GPButtons = {
    LUp = 1, LDown = 2, LLeft = 4, LRight = 8,
    RUp = 16, RDown = 32, RLeft = 64, RRight = 128,
    LTrigTop = 256, LTrigBottom = 512,
    RTrigTop = 1024, RTrigBottom = 2048,
    Select = 4096, Start = 8192,
    LStickPush = 16384, RStickPush = 32768,
    Decide = 65536, Cancel = 131072
}

local function count_set_bits(n)
    local count = 0
    while n > 0 do
        n = n & (n - 1)
        count = count + 1
    end
    return count
end

local function get_gamepad_button_name(mask)
    local names = {}
    for name, val in pairs(GPButtons) do
        if (mask & val) == val then
            table.insert(names, name)
        end
    end
    if #names == 0 then return "None" end
    table.sort(names)
    return table.concat(names, " + ")
end

local function get_input_display_name(input_def)
    if not input_def then return "None" end
    if input_def.device == "keyboard" then
        return get_key_name(input_def.code)
    elseif input_def.device == "gamepad" then
        return get_gamepad_button_name(input_def.code)
    end
    return "Unknown"
end

local function check_input_down(input_def)
    if not input_def then return false end
    
    if input_def.device == "keyboard" then
        if not reframework then return false end
        local is_down = reframework:is_key_down(input_def.code)
        
        local key_code = input_def.code
        if key_state[key_code] == nil then key_state[key_code] = false end
        
        if is_down and not key_state[key_code] then
            key_state[key_code] = true
            return true
        elseif not is_down then
            key_state[key_code] = false
        end
        return false
        
    elseif input_def.device == "gamepad" then
        local gamepad = sdk.get_native_singleton("via.hid.GamePad")
        if not gamepad then return false end
        local type_def = sdk.find_type_definition("via.hid.GamePad")
        local device = sdk.call_native_func(gamepad, type_def, "get_LastInputDevice")
        if not device then return false end
        
        local current_buttons = device:call("get_Button") or 0
        local is_down = (current_buttons & input_def.code) == input_def.code
        
        local key_id = "gp_" .. input_def.code
        if key_state[key_id] == nil then key_state[key_id] = false end
        
        if is_down and not key_state[key_id] then
            key_state[key_id] = true
            return true
        elseif not is_down then
            key_state[key_id] = false
        end
        return false
    end
    return false
end

local pending_components = {} 
local pending_filter_infos = {} 
local pending_position = nil
local pending_rotation = nil
local pending_unstuck_fix = false
local unstuck_fix_timer = 0
local is_noclipping = false
local is_noclipping_auto_stop = 0
local noclip_start_time = 0
local last_noclip_log_time = 0

local game_name = reframework:get_game_name()
local player_manager_name = "app.PlayerManager"
if game_name == "re2" or game_name == "re3" then
    player_manager_name = "app.SurvivorManager"
end

local function get_player_context()
    local character_manager = sdk.get_managed_singleton("app.CharacterManager")
    if not character_manager then return nil, "CharacterManager not found" end

    local context = character_manager:call("getPlayerContextRefFast")
    if context then return context, "getPlayerContextRefFast" end

    context = character_manager:call("getPlayerContextRefFast185379")
    if context then return context, "getPlayerContextRefFast185379" end

    context = character_manager:call("get_PlayerContextFast")
    if context then return context, "get_PlayerContextFast" end

    context = character_manager:call("get_PlayerContextFast185242")
    if context then return context, "get_PlayerContextFast185242" end

    return nil, "Context not found"
end

local function get_local_player()
    local context = get_player_context()
    if context then return context end

    local player_manager = sdk.get_managed_singleton(sdk.game_namespace(player_manager_name))
    if not player_manager then return nil end
    
    local player = player_manager:call("get_Player")
    if not player then
        player = player_manager:get_field("_Player")
    end
    return player
end

local function get_transform(player_or_context)
    if not player_or_context then return nil end
    local transform = player_or_context:call("get_Transform")
    if transform then return transform end
    local game_object = player_or_context:call("get_GameObject")
    if game_object then
        return game_object:call("get_Transform")
    end
    return nil
end

local function disable_physics_components(game_object)
    if not game_object then return end
    
    local collider_type = sdk.typeof("via.physics.Collider")
    local success, colliders = pcall(function() 
        return game_object:call("getComponents(System.Type)", collider_type) 
    end)
    
    if success and colliders then
        local count = colliders:get_Count()
        for i = 0, count - 1 do
            pcall(function()
                local component = colliders:get_Item(i)
                if component and component:call("get_Enabled") then
                    local filter_info = component:call("get_FilterInfo")
                    if filter_info then
                        local old_mask = filter_info:call("get_MaskBits")
                        if old_mask ~= 0 then
                            table.insert(pending_filter_infos, { info = filter_info, mask = old_mask })
                            filter_info:call("set_MaskBits", 0)
                        end
                    else
                        component:call("set_Enabled", false)
                        table.insert(pending_components, component)
                    end
                end
            end)
        end
    end

    pcall(function()
        local cc_type = sdk.typeof("via.physics.CharacterController")
        local cc = game_object:call("getComponent(System.Type)", cc_type)
        if cc and cc:call("get_Enabled") then
            cc:call("set_Enabled", false)
            table.insert(pending_components, cc)
        end
    end)

    pcall(function()
        local driver_type = sdk.typeof("app.PlayerFPSMovementDriver")
        local driver = game_object:call("getComponent(System.Type)", driver_type)
        if driver and driver:call("get_Enabled") then
            driver:call("set_Enabled", false)
            table.insert(pending_components, driver)
        end
    end)

    pcall(function()
        local movement_type = sdk.typeof("app.PlayerMovement")
        local movements = game_object:call("getComponents(System.Type)", movement_type)
        if movements then
            local count = movements:get_Count()
            for i = 0, count - 1 do
                local movement = movements:get_Item(i)
                if movement and movement:call("get_Enabled") then
                    movement:call("set_Enabled", false)
                    table.insert(pending_components, movement)
                end
            end
        end
    end)
end

local last_update_log_time = 0
local function update_context_position(pos)
    local context, method_used = get_player_context()
    if not context then return end

    local success, err = pcall(function() 
        local set_method = context:get_type_definition():get_method("set_PositionFast")
        if not set_method then
            set_method = context:get_type_definition():get_method("set_PositionFast232259")
        end

        if set_method then
            set_method:call(context, pos)
        else
            context:set_field("<PositionFast>k__BackingField", pos)
        end
    end)
    
    if not success and os.clock() - last_update_log_time > 1.0 then
        write_log("update_context_position error: " .. tostring(err))
        last_update_log_time = os.clock()
    end
end



local function start_noclip_teleport(pos, rot)
    if is_noclipping then return end
    write_log(">>> Starting noclip sequence...")
    
    is_noclipping = true
    noclip_start_time = os.clock()
    last_noclip_log_time = os.clock()
    pending_position = pos
    pending_rotation = rot

    local player = get_local_player()
    if not player then 
        write_log("start_noclip: Player not found")
        return 
    end

    local transform = get_transform(player)
    if not transform then 
        write_log("start_noclip: Transform not found")
        return 
    end
    
    -- write_log("Target: " .. tostring(pos))

    local game_object = nil
    if player.call then game_object = player:call("get_GameObject") end
    if not game_object and transform then game_object = transform:call("get_GameObject") end

    if not game_object then
        write_log("start_noclip: GameObject not found")
        return
    end

    disable_physics_components(game_object)

    -- Safe set_Position call
    local success, err = pcall(function()
        -- Debug type before call
        write_log("Teleporting to target coordinates...")
        transform:set_Position(pos)
        transform:set_Rotation(rot)
    end)
    
    if not success then
        write_log("CRITICAL ERROR: transform:set_Position failed: " .. tostring(err))
        -- Attempt fallback or abort
    end

    update_context_position(pos)
    
    pcall(function()
        local cc_type = sdk.typeof("via.physics.CharacterController")
        local cc = game_object:call("getComponent(System.Type)", cc_type)
        if cc then
            local set_pos_method = cc:get_type_definition():get_method("set_Position")
            if set_pos_method then set_pos_method:call(cc, pos) end
            
            local warp_method = cc:get_type_definition():get_method("warp")
            if warp_method then warp_method:call(cc) end
        end
    end)
end

local function stop_noclip_teleport()
    if not is_noclipping then return end
    write_log("<<< Stopping noclip sequence...")

    local player = get_local_player()
    local transform = get_transform(player)
    if transform then
         write_log("Teleport sequence finished.")
    end
    
    for _, item in ipairs(pending_filter_infos) do
        pcall(function() item.info:call("set_MaskBits", item.mask) end)
    end
    
    for i = #pending_components, 1, -1 do
        local component = pending_components[i]
        pcall(function() component:call("set_Enabled", true) end)
    end

    pending_components = {}
    pending_filter_infos = {}
    pending_position = nil
    pending_rotation = nil
    is_noclipping = false
    
    if pending_unstuck_fix then
        unstuck_fix_timer = os.clock() + 1.0  -- Increased delay to 0.5s to wait for physics
        pending_unstuck_fix = false
        write_log("Scheduling Auto-Unstuck (Delay 1.0s)...")
    else
        unstuck_fix_timer = 0
    end
    
    write_log("Noclip ENDED.")
end

local function check_input_held(input_def)
    if not input_def then return false end
    
    if input_def.device == "keyboard" then
        if not reframework then return false end
        return reframework:is_key_down(input_def.code)
        
    elseif input_def.device == "gamepad" then
        local gamepad = sdk.get_native_singleton("via.hid.GamePad")
        if not gamepad then return false end
        local type_def = sdk.find_type_definition("via.hid.GamePad")
        local device = sdk.call_native_func(gamepad, type_def, "get_LastInputDevice")
        if not device then return false end
        
        local current_buttons = device:call("get_Button") or 0
        return (current_buttons & input_def.code) == input_def.code
    end
    
    return false
end

local function save_to_current_slot()
    local slot_idx = config.current_slot_index
    local slot = slots[slot_idx]
    if not slot then return end

    local player = get_local_player()
    local transform = get_transform(player)
    if transform then
        local pos = transform:get_Position()
        local rot = transform:get_Rotation()
        
        -- 1. Save to Runtime Cache (High Precision / Native Type)
        -- This ensures in-session loads are bit-perfect identical to NoError version
        runtime_slots[slot_idx] = {
            position = pos,
            rotation = rot
        }

        -- 2. Save to Persistence (JSON)
        -- Use string format to preserve precision and avoid float truncation
        if pos then
            slot.position = { 
                x = string.format("%.9f", pos.x), 
                y = string.format("%.9f", pos.y), 
                z = string.format("%.9f", pos.z) 
            }
        end
        if rot then
            slot.rotation = { 
                x = string.format("%.9f", rot.x), 
                y = string.format("%.9f", rot.y), 
                z = string.format("%.9f", rot.z), 
                w = string.format("%.9f", rot.w) 
            }
        end
        
        save_data()
        current_message = "Saved to " .. slot.note
        message_timer = 2.0
        write_log("Saved to Slot " .. slot_idx)
    else
        current_message = get_text("player_not_found")
        message_timer = 2.0
    end
end

-- Robust Vector/Quaternion Creation with Fallbacks
local function create_vector3f(x, y, z)
    x, y, z = tonumber(x), tonumber(y), tonumber(z)
    if not x or not y or not z then return nil end

    -- Safety Check for NaN/Inf
    if x ~= x or y ~= y or z ~= z then return nil end -- NaN check
    if math.abs(x) > 1000000 or math.abs(y) > 1000000 or math.abs(z) > 1000000 then return nil end -- Bound check

    -- 1. Try Vector3f (GLM) - Priority: Stable & Native to REFramework
    if Vector3f and Vector3f.new then return Vector3f.new(x, y, z) end

    -- 2. Try via.vec3 ValueType (SDK) - Fallback
    local t = sdk.find_type_definition("via.vec3")
    if t then
        local v = ValueType.new(t)
        -- Try property access first, then set_field
        local success = pcall(function() v.x = x; v.y = y; v.z = z end)
        if not success then
            success = pcall(function() 
                v:set_field("x", x)
                v:set_field("y", y)
                v:set_field("z", z)
            end)
        end
        if success then return v end
    end

    write_log("Error: Could not create Vector3f")
    return nil
end

local function create_quaternion(x, y, z, w)
    x, y, z, w = tonumber(x), tonumber(y), tonumber(z), tonumber(w)
    if not x or not y or not z or not w then return nil end

    -- Safety Check
    if x ~= x or y ~= y or z ~= z or w ~= w then return nil end

    -- Normalization (Critical for Physics Stability)
    local len_sq = x*x + y*y + z*z + w*w
    if len_sq > 0.000001 and math.abs(len_sq - 1.0) > 0.000001 then
        local len = math.sqrt(len_sq)
        x, y, z, w = x/len, y/len, z/len, w/len
    end

    -- 1. Try Quaternion (GLM) - Priority: Stable & Native to REFramework
    if Quaternion and Quaternion.new then return Quaternion.new(x, y, z, w) end
    if Vector4f and Vector4f.new then return Vector4f.new(x, y, z, w) end

    -- 2. Try via.quaternion ValueType (SDK) - Fallback
    local t = sdk.find_type_definition("via.quaternion") or sdk.find_type_definition("via.Quaternion")
    if t then
        local val = ValueType.new(t)
        -- Try direct property access
        pcall(function() val.x = x; val.y = y; val.z = z; val.w = w end)
        return val
    end

    write_log("Error: Could not create Quaternion")
    return nil
end

local function load_from_current_slot_oneshot()
    local slot_idx = config.current_slot_index
    local slot = slots[slot_idx]
    local runtime_slot = runtime_slots[slot_idx]
    
    if not slot or not slot.position or not slot.rotation then 
        current_message = "Slot Empty!"
        message_timer = 2.0
        return 
    end
    
    local target_pos = nil
    local target_rot = nil

    -- Priority 1: Use Runtime Cache (Exact Native Object)
    if runtime_slot and runtime_slot.position and runtime_slot.rotation then
        target_pos = runtime_slot.position
        target_rot = runtime_slot.rotation
        pending_unstuck_fix = false
        write_log("Loading from Runtime Cache (High Precision)")
    else
        -- Priority 2: Reconstruct from JSON Data
        -- NOTE: Removed +0.41999 offset as it causes issues. Relying on correct engine type creation.
        target_pos = create_vector3f(slot.position.x, slot.position.y, slot.position.z)
        target_rot = create_quaternion(slot.rotation.x, slot.rotation.y, slot.rotation.z, slot.rotation.w)
        pending_unstuck_fix = true
        write_log("Loading from JSON Data (Reconstructed)")
    end

    if target_pos and target_rot then
        start_noclip_teleport(target_pos, target_rot)
        is_noclipping_auto_stop = os.clock() + 0.1
        current_message = get_text("teleporting")
        message_timer = 2.0
    else
        write_log("Cannot load: Failed to create target position/rotation objects")
        current_message = get_text("error_invalid_data")
        message_timer = 2.0
    end
end

re.on_draw_ui(function()
    if message_timer > 0 then
        local display_size = imgui.get_display_size()
        local screen_w = display_size.x
        imgui.set_next_window_pos({screen_w / 2 - 100, 50}, 1, {0.5, 0.5})
        imgui.set_next_window_size({200, 50})
        imgui.begin_window("SnapshotMsg", true, 1 | 2 | 4 | 8 | 32 | 64)
        imgui.text(current_message)
        imgui.end_window()
        message_timer = message_timer - (1.0 / 60.0)
    end

    -- Main UI
    if imgui.tree_node(get_text("window_title")) then
        
        -- Key Binding Configuration
        imgui.set_next_item_open(true, 1) -- 1 = ImGuiCond_FirstUseEver
        if imgui.tree_node(get_text("settings")) then
            -- Language Selector (Using tree_node as fallback for missing begin_combo)
            local lang_display_name = config.language
            if config.language == "System" then lang_display_name = get_text("follow_system")
            elseif config.language == "en" then lang_display_name = "English"
            elseif config.language == "zh_cn" then lang_display_name = "简体中文"
            elseif config.language == "zh_tw" then lang_display_name = "繁体中文"
            elseif config.language == "ja" then lang_display_name = "日本語"
            elseif config.language == "ko" then lang_display_name = "한국어"
            end

            if imgui.tree_node(get_text("language") .. ": " .. lang_display_name) then
                local changed, _
                
                changed, _ = imgui.checkbox(get_text("follow_system"), config.language == "System")
                if changed then
                    config.language = "System"
                    update_language("System")
                    save_data()
                end

                changed, _ = imgui.checkbox("English", config.language == "en")
                if changed then
                    config.language = "en"
                    update_language("en")
                    save_data()
                end

                changed, _ = imgui.checkbox("简体中文", config.language == "zh_cn")
                if changed then
                    config.language = "zh_cn"
                    update_language("zh_cn")
                    save_data()
                end

                changed, _ = imgui.checkbox("繁体中文", config.language == "zh_tw")
                if changed then
                    config.language = "zh_tw"
                    update_language("zh_tw")
                    save_data()
                end

                changed, _ = imgui.checkbox("日本語", config.language == "ja")
                if changed then
                    config.language = "ja"
                    update_language("ja")
                    save_data()
                end

                changed, _ = imgui.checkbox("한국어", config.language == "ko")
                if changed then
                    config.language = "ko"
                    update_language("ko")
                    save_data()
                end

                imgui.tree_pop()
            end

            imgui.text(get_text("bind_instruction"))
            imgui.push_style_color(0, 0xFF0000FF)
            imgui.text(get_text("gamepad_instruction"))
            imgui.pop_style_color(1)
            
            -- Save Key Binding
            local save_btn_text = get_text("save_key_prefix") .. get_input_display_name(config.save_key)
            if binding_mode and binding_mode.target == "save_key" then
                if binding_mode.has_gp_input then
                    save_btn_text = string.format(get_text("holding_buttons"), (binding_mode.max_gp_count or 0))
                else
                    save_btn_text = get_text("press_key_combo")
                end
            end
            
            if imgui.button(save_btn_text) then
                binding_mode = { target = "save_key", start_time = os.clock() }
            end
            
            -- Load Key Binding
            local load_btn_text = get_text("load_key_prefix") .. get_input_display_name(config.load_key)
            if binding_mode and binding_mode.target == "load_key" then
                if binding_mode.has_gp_input then
                    load_btn_text = string.format(get_text("holding_buttons"), (binding_mode.max_gp_count or 0))
                else
                    load_btn_text = get_text("press_key_combo")
                end
            end
            
            if imgui.button(load_btn_text) then
                binding_mode = { target = "load_key", start_time = os.clock() }
            end
            
            if imgui.button(get_text("reset_defaults")) then
                config.save_key = { device = "keyboard", code = VK_F5 }
                config.load_key = { device = "keyboard", code = VK_F9 }
                save_data()
                current_message = get_text("keys_reset_msg")
                message_timer = 2.0
            end

            imgui.tree_pop()
        end

        imgui.separator()

        -- Slot Selection
        imgui.text(get_text("select_slot_label"))
        local slot_to_remove = nil

        for i, slot in ipairs(slots) do
            local label = string.format(get_text("slot_prefix"), i)
            local is_selected = (config.current_slot_index == i)
            
            local changed, new_val = imgui.checkbox(label, is_selected)
            if changed then
                config.current_slot_index = i
            end
            
            imgui.same_line()
            -- Ensure slot.note is not nil
            local safe_note = slot.note or ""
            imgui.push_item_width(150)
            
            local note_changed, new_note = false, safe_note
            local hint = string.format(get_text("slot_note_hint"), i)
            
            -- Strategy: Always ensure visible text.
            -- 1. If note is empty, fill it with default text "Slot X note"
            if safe_note == "" then
                safe_note = hint
                slot.note = hint 
            end

            -- 2. Use standard input_text
            -- We abandon input_text_with_hint because it seems unreliable in this environment or user prefers actual text value
            note_changed, new_note = imgui.input_text("##" .. i, safe_note, 0)
            
            imgui.pop_item_width()
            if note_changed then 
                slot.note = new_note 
            end

            imgui.same_line()
            if imgui.button("-##" .. i) then
                slot_to_remove = i
            end
        end

        if slot_to_remove then
            if #slots > 3 then
                -- 1. Remove from persistent slots
                table.remove(slots, slot_to_remove)
                
                -- 2. Sync runtime_slots (handle sparse table shifting)
                local new_runtime = {}
                for k, v in pairs(runtime_slots) do
                    if type(k) == "number" then
                        if k < slot_to_remove then
                            new_runtime[k] = v
                        elseif k > slot_to_remove then
                            new_runtime[k - 1] = v
                        end
                    end
                end
                runtime_slots = new_runtime

                -- 3. Adjust current selection
                if config.current_slot_index > slot_to_remove then
                    config.current_slot_index = config.current_slot_index - 1
                elseif config.current_slot_index >= #slots + 1 then
                    config.current_slot_index = #slots
                end
                save_data()
            else
                current_message = get_text("delete_error_min_slots")
                message_timer = 3.0
            end
        end

        if #slots < max_slots then
            if imgui.button(get_text("add_slot_btn")) then
                table.insert(slots, { position = nil, rotation = nil, note = string.format(get_text("slot_note_hint"), (#slots + 1)) })
                save_data()
            end
        else
            imgui.push_style_color(0, 0xFF888888) -- Text Color (Grey)
            imgui.text(get_text("max_slots_reached"))
            imgui.pop_style_color(1)
        end

        imgui.separator()

        if imgui.button(get_text("save_to_slot_btn") .. config.current_slot_index) then
            save_to_current_slot()
        end
        
        if imgui.same_line() then end
        
        if imgui.button(get_text("load_from_slot_btn") .. config.current_slot_index) then
            load_from_current_slot_oneshot()
        end

        local current_slot = slots[config.current_slot_index]
        if current_slot and current_slot.position then
            imgui.text(get_text("cached_pos_prefix") .. string.format("%.2f, %.2f, %.2f", current_slot.position.x, current_slot.position.y, current_slot.position.z))
        else
            imgui.text(get_text("slot_empty"))
        end
        
        imgui.tree_pop()
    end
end)

local function handle_load_snapshot_input()
    local slot = slots[config.current_slot_index]
    
    -- Auto-stop logic (for UI button trigger)
    if is_noclipping_auto_stop > 0 then
        if os.clock() >= is_noclipping_auto_stop then
            stop_noclip_teleport()
            is_noclipping_auto_stop = 0
            current_message = get_text("teleport_done")
            message_timer = 2.0
        else
            if not is_noclipping then
                 -- Retry start if dropped during auto-sequence
                 if slot and slot.position then
                     local target_pos = nil
                     local target_rot = nil
                     local runtime_slot = runtime_slots[config.current_slot_index]

                     if runtime_slot and runtime_slot.position and runtime_slot.rotation then
                         target_pos = runtime_slot.position
                         target_rot = runtime_slot.rotation
                         pending_unstuck_fix = false
                     else
                         target_pos = create_vector3f(slot.position.x, slot.position.y, slot.position.z)
                         target_rot = create_quaternion(slot.rotation.x, slot.rotation.y, slot.rotation.z, slot.rotation.w)
                         pending_unstuck_fix = true
                     end

                     if target_pos and target_rot then start_noclip_teleport(target_pos, target_rot) end
                 end
            end
            return
        end
    end

    -- Manual Input Logic
    local is_load_held = check_input_held(config.load_key)
    
    if is_load_held then
        if not slot or not slot.position then 
            if check_input_down(config.load_key) then
                current_message = get_text("slot_empty_msg")
                message_timer = 2.0
            end
            return 
        end

        if not is_noclipping then
            local target_pos = nil
            local target_rot = nil
            local runtime_slot = runtime_slots[config.current_slot_index]

            if runtime_slot and runtime_slot.position and runtime_slot.rotation then
                target_pos = runtime_slot.position
                target_rot = runtime_slot.rotation
                pending_unstuck_fix = false
            else
                -- Removed offset
                target_pos = create_vector3f(slot.position.x, slot.position.y, slot.position.z)
                target_rot = create_quaternion(slot.rotation.x, slot.rotation.y, slot.rotation.z, slot.rotation.w)
                pending_unstuck_fix = true
            end
            
            if target_pos and target_rot then
                start_noclip_teleport(target_pos, target_rot)
                current_message = get_text("teleporting")
                message_timer = 0.1
            end
        end
    else
        -- Released
        if is_noclipping and is_noclipping_auto_stop == 0 then
            stop_noclip_teleport()
            current_message = get_text("teleport_done")
            message_timer = 2.0
        end
    end
end

local function get_components_of_type(player_or_context, type_name)
    if not player_or_context then return {} end
    
    local game_object = nil
    
    -- Try to get GameObject from Component
    -- Use pcall because player_or_context might be the GameObject itself or something else
    local success, go = pcall(function() return player_or_context:call("get_GameObject") end)
    
    if success and go then
        game_object = go
    else
        -- Fallback: assume the input is the GameObject
        game_object = player_or_context
    end
    
    if not game_object then return {} end

    -- Verify game_object is valid for calls
    if type(game_object) ~= "userdata" or not game_object.call then
        -- write_log("Debug: game_object is not a callable userdata")
        return {}
    end

    local type_obj = sdk.typeof(type_name)
    if not type_obj then 
        -- write_log("Error: Component type not found")
        return {} 
    end

    local result = {}
    
    -- 1. Try getComponents (Array)
    local success_comps, components = pcall(function() 
        return game_object:call("getComponents(System.Type)", type_obj) 
    end)

    if success_comps and components then
        local count = components:get_Count()
        for i = 0, count - 1 do
            local comp = components:get_Item(i)
            if comp then
                table.insert(result, comp)
            end
        end
    end
    
    -- 2. If empty, try getComponent (Single) - Fallback
    if #result == 0 then
        local success_single, component = pcall(function()
            return game_object:call("getComponent(System.Type)", type_obj)
        end)
        if success_single and component then
             table.insert(result, component)
        end
    end

    return result
end

re.on_frame(function()
    -- Auto-Unstuck Logic (TODO: Future Implementation)
    -- Current attempts to auto-unstuck via CharacterController (Disable->SetPos->Enable, Warp, PurgeCache)
    -- result in visual jitter but no actual displacement, likely due to persistent physics state or frame timing.
    -- Manual jump by player works, so leaving this for future research.
    
    --[[ 
    if unstuck_fix_timer > 0 and os.clock() > unstuck_fix_timer then
        write_log(">>> Auto-Unstuck Triggered")
        unstuck_fix_timer = 0
        
        local player = get_local_player()
        if not player then
            write_log("Auto-Unstuck Aborted: Player not found")
        else
            local transform = get_transform(player)
            if not transform then
                write_log("Auto-Unstuck Aborted: Transform not found")
            else
                local pos = transform:get_Position()
                if not pos then
                    write_log("Auto-Unstuck Aborted: Position is nil")
                else
                     -- ... (Previous logic removed for stability) ...
                end
            end
        end
    end
    ]]

    -- Key Binding Logic
    if binding_mode then
        if os.clock() - binding_mode.start_time > 0.2 then
             if reframework:is_key_down(KBKeys.ESC) then
                 binding_mode = nil
                 current_message = get_text("binding_cancelled")
                 message_timer = 2.0
                 return
             end
             
             local bound = false
             
             -- Check Gamepad (Priority)
             local gamepad = sdk.get_native_singleton("via.hid.GamePad")
             local gp_active = false
             
             if gamepad then
                 local type_def = sdk.find_type_definition("via.hid.GamePad")
                 local device = sdk.call_native_func(gamepad, type_def, "get_LastInputDevice")
                 if device then
                     local current_buttons = device:call("get_Button") or 0
                     
                     if current_buttons ~= 0 then
                         gp_active = true
                         binding_mode.has_gp_input = true
                         
                         local count = count_set_bits(current_buttons)
                         if count > (binding_mode.max_gp_count or 0) then
                             binding_mode.max_gp_count = count
                             binding_mode.best_gp_mask = current_buttons
                         end
                         
                         current_message = string.format(get_text("holding_buttons"), count)
                         message_timer = 0.1
                     elseif binding_mode.has_gp_input then
                         local final_count = binding_mode.max_gp_count or 0
                         if final_count >= 3 then
                             config[binding_mode.target] = { device = "gamepad", code = binding_mode.best_gp_mask }
                             save_data()
                             current_message = get_text("bound_gamepad")
                             message_timer = 3.0
                             binding_mode = nil
                             bound = true
                         else
                             current_message = get_text("error_gamepad_min_buttons")
                             message_timer = 3.0
                             binding_mode.has_gp_input = false
                             binding_mode.max_gp_count = 0
                             binding_mode.best_gp_mask = 0
                         end
                         gp_active = true
                     end
                 end
             end
             
             if not bound and not gp_active and not binding_mode.has_gp_input then
                 for name, code in pairs(KBKeys) do
                     if name ~= "MOUSE_LEFT" and name ~= "MOUSE_RIGHT" and name ~= "MOUSE_MIDDLE" then
                         if reframework:is_key_down(code) then
                             config[binding_mode.target] = { device = "keyboard", code = code }
                             key_state[code] = true
                             current_message = get_text("bound_keyboard") .. name
                             message_timer = 2.0
                             save_data()
                             binding_mode = nil
                             bound = true
                             break
                         end
                     end
                 end
             end
        end
        return -- Block other input
    end

    if is_noclipping and pending_position and pending_rotation then
        local player = get_local_player()
        if player then
            local transform = get_transform(player)
            if transform then
                pcall(function()
                    transform:set_Position(pending_position)
                    transform:set_Rotation(pending_rotation)
                end)
                update_context_position(pending_position)
                
                -- Force Warp CharacterController Every Frame with Robust Re-disable Logic
                pcall(function()
                    local game_object = transform:call("get_GameObject")
                    if game_object then
                        local cc_type = sdk.typeof("via.physics.CharacterController")
                        local cc = game_object:call("getComponent(System.Type)", cc_type)
                        if cc then
                            if cc:call("get_Enabled") then 
                                cc:call("set_Enabled", false) 
                                
                                -- CRITICAL FIX: Ensure re-disabled component is tracked for restoration
                                local found = false
                                for _, c in ipairs(pending_components) do
                                    if c == cc then found = true; break end
                                end
                                if not found then
                                    table.insert(pending_components, cc)
                                end
                            end
                            
                            local set_pos_method = cc:get_type_definition():get_method("set_Position")
                            if set_pos_method then set_pos_method:call(cc, pending_position) end
                            local warp_method = cc:get_type_definition():get_method("warp")
                            if warp_method then warp_method:call(cc) end
                        end
                    end
                end)
            end
        end
    end

    -- Trigger Logic
    if check_input_down(config.save_key) then
        save_to_current_slot()
    end
    
    handle_load_snapshot_input()
end)
