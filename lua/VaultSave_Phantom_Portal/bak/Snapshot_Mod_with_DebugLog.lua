local api = {}
local snapshot = nil
local last_action_time = 0
local message_timer = 0
local current_message = ""
local log_filename = "Snapshot_Mod_Log.txt"

local function write_log(text)
    local f = io.open(log_filename, "a+")
    if f then
        f:write("[" .. os.date("%Y-%m-%d %H:%M:%S") .. "] " .. text .. "\n")
        f:close()
    end
    log.info("[Snapshot] " .. text)
end

local VK_F5 = 116
local VK_F9 = 120

local key_state = {
    [VK_F5] = false,
    [VK_F9] = false
}

local function check_key_down_once(key_code)
    if not reframework then return false end
    local is_down = reframework:is_key_down(key_code)
    if is_down and not key_state[key_code] then
        key_state[key_code] = true
        return true
    elseif not is_down then
        key_state[key_code] = false
    end
    return false
end

local pending_components = {} 
local pending_filter_infos = {} 
local pending_position = nil
local pending_rotation = nil
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
    if not player_manager then 
        return nil 
    end
    
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

local function log_all_components(game_object)
    if not game_object then return end
    write_log("--- Component Dump Start ---")
    local components = game_object:call("get_Components")
    if components then
        local count = components:get_Count()
        for i = 0, count - 1 do
            local comp = components:get_Item(i)
            if comp then
                local type_name = "Unknown"
                pcall(function() 
                    local type_def = comp:get_type_definition()
                    if type_def then
                        type_name = type_def:get_FullName()
                    end
                end)
                write_log("Component: " .. tostring(type_name) .. " | Enabled: " .. tostring(comp:call("get_Enabled")))
            end
        end
    end
    write_log("--- Component Dump End ---")
end

local function disable_physics_components(game_object)
    if not game_object then return end
    
    log_all_components(game_object)

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
                            local name = "Unknown"
                            pcall(function() name = component:get_type_definition():get_FullName() end)
                            write_log("Disabled Collision Mask for " .. (name or "Unknown"))
                        end
                    else
                        component:call("set_Enabled", false)
                        table.insert(pending_components, component)
                        local name = "Unknown"
                        pcall(function() name = component:get_type_definition():get_FullName() end)
                        write_log("Disabled Component: " .. (name or "Unknown"))
                    end
                end
            end)
        end
    end

    pcall(function()
        local cc_type = sdk.typeof("via.physics.CharacterController")
        local cc = game_object:call("getComponent(System.Type)", cc_type)
        if cc and cc:call("get_Enabled") then
            -- Force Disable CharacterController
            cc:call("set_Enabled", false)
            table.insert(pending_components, cc)
            write_log("Disabled CharacterController")
        end
    end)

    pcall(function()
        local driver_type = sdk.typeof("app.PlayerFPSMovementDriver")
        local driver = game_object:call("getComponent(System.Type)", driver_type)
        if driver and driver:call("get_Enabled") then
            driver:call("set_Enabled", false)
            table.insert(pending_components, driver)
            write_log("Disabled PlayerFPSMovementDriver")
        end
    end)

    -- Also try generic Movement components
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
                    local name = "Unknown"
                    pcall(function() name = movement:get_type_definition():get_FullName() end)
                    write_log("Disabled PlayerMovement: " .. (name or "Unknown"))
                end
            end
        end
    end)
end

local last_update_log_time = 0
local function update_context_position(pos)
    local context, method_used = get_player_context()
    
    if not context then 
        if os.clock() - last_update_log_time > 1.0 then
             write_log("update_context_position: " .. (method_used or "Unknown error"))
             last_update_log_time = os.clock()
        end
        return 
    end

    local success, err = pcall(function() 
        local set_method = context:get_type_definition():get_method("set_PositionFast")
        local set_method_name = "set_PositionFast"
        if not set_method then
            set_method = context:get_type_definition():get_method("set_PositionFast232259")
            set_method_name = "set_PositionFast232259"
        end

        if set_method then
            set_method:call(context, pos)
            if os.clock() - last_update_log_time > 2.0 then
                write_log("update_context_position: Success using " .. method_used .. " and " .. set_method_name)
                last_update_log_time = os.clock()
            end
        else
            context:set_field("<PositionFast>k__BackingField", pos)
            if os.clock() - last_update_log_time > 2.0 then
                write_log("update_context_position: Success using BackingField")
                last_update_log_time = os.clock()
            end
        end
    end)
    
    if not success then
        if os.clock() - last_update_log_time > 1.0 then
            write_log("update_context_position: Error: " .. tostring(err))
            last_update_log_time = os.clock()
        end
    end
end

local function get_player_info_string(player, transform)
    local info = ""
    
    -- Position
    if transform then
        local pos = transform:get_Position()
        info = info .. "Pos: " .. tostring(pos)
    else
        info = info .. "Pos: Unknown"
    end

    -- Character Name (Try to get from GameObject name or specific component)
    local char_name = "Unknown"
    if player then
        pcall(function()
            local go = player:call("get_GameObject")
            if go then
                char_name = go:call("get_Name")
            end
        end)
    end
    info = info .. " | Char: " .. char_name

    -- TODO: Add HP and Ammo info here
    -- local hp = ...
    -- info = info .. " | HP: " .. tostring(hp)
    
    return info
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
    
    write_log("Target: " .. tostring(pos))
    write_log("Start State: " .. get_player_info_string(player, transform))

    local game_object = nil
    if player.call then
        game_object = player:call("get_GameObject")
    end
    if not game_object and transform then
        game_object = transform:call("get_GameObject")
    end

    if not game_object then
        write_log("start_noclip: GameObject not found")
        return
    end

    disable_physics_components(game_object)

    transform:set_Position(pos)
    transform:set_Rotation(rot)
    update_context_position(pos)
    
    pcall(function()
        local cc_type = sdk.typeof("via.physics.CharacterController")
        local cc = game_object:call("getComponent(System.Type)", cc_type)
        if cc then
            -- Try to set Position property first
            local set_pos_method = cc:get_type_definition():get_method("set_Position")
            if set_pos_method then
                set_pos_method:call(cc, pos)
            end

            local warp_method = cc:get_type_definition():get_method("warp")
            if warp_method then 
                warp_method:call(cc) 
                write_log("Warped CharacterController")
            end
        end
    end)
end

local function stop_noclip_teleport()
    if not is_noclipping then return end
    write_log("<<< Stopping noclip sequence...")

    local player = get_local_player()
    local transform = get_transform(player)
    if transform then
         write_log("End State: " .. get_player_info_string(player, transform))
    end
    
    local duration = os.clock() - noclip_start_time
    write_log(string.format("Duration: %.3fs", duration))

    for _, item in ipairs(pending_filter_infos) do
        pcall(function() item.info:call("set_MaskBits", item.mask) end)
    end
    
    for i = #pending_components, 1, -1 do
        local component = pending_components[i]
        local success, err = pcall(function() component:call("set_Enabled", true) end)
        if not success then
            write_log("Error restoring component: " .. tostring(err))
        end
    end

    pending_components = {}
    pending_filter_infos = {}
    pending_position = nil
    pending_rotation = nil
    is_noclipping = false
    write_log("Noclip ENDED.")
end

local function trigger_oneshot_teleport()
    if not snapshot then return end
    start_noclip_teleport(snapshot.position, snapshot.rotation)
    current_message = "Teleporting..."
    message_timer = 0.5
    is_noclipping_auto_stop = os.clock() + 0.5
end

local function save_snapshot()
    local player = get_local_player()
    if not player then 
        current_message = "Error: Player not found!"
        message_timer = 3.0
        write_log("Save failed: Player not found")
        return 
    end

    local transform = get_transform(player)
    if not transform then 
        write_log("Save failed: Transform not found")
        return 
    end

    snapshot = {
        position = transform:get_Position(),
        rotation = transform:get_Rotation(),
    }

    current_message = "Snapshot Saved!"
    message_timer = 2.0
    write_log("Snapshot saved at " .. tostring(snapshot.position))
end

local function handle_load_snapshot_input()
    if not snapshot then 
        if check_key_down_once(VK_F9) then
            current_message = "No Snapshot Found!"
            message_timer = 2.0
        end
        return 
    end

    local is_f9_down = false
    if reframework then
        is_f9_down = reframework:is_key_down(VK_F9)
    end

    if is_noclipping_auto_stop > 0 then
        if os.clock() >= is_noclipping_auto_stop then
            stop_noclip_teleport()
            is_noclipping_auto_stop = 0
            current_message = "Teleport Done!"
            message_timer = 2.0
        else
            if not is_noclipping then
                 start_noclip_teleport(snapshot.position, snapshot.rotation)
            end
            return
        end
    end

    if is_f9_down then
        if not is_noclipping then
            start_noclip_teleport(snapshot.position, snapshot.rotation)
            current_message = "Teleporting..."
            message_timer = 0.1
        end
    else
        if is_noclipping and is_noclipping_auto_stop == 0 then
            stop_noclip_teleport()
            current_message = "Teleport Done!"
            message_timer = 2.0
        end
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
end)

re.on_frame(function()
    if is_noclipping and pending_position and pending_rotation then
        local player = get_local_player()
        if player then
            local transform = get_transform(player)
            if transform then
                transform:set_Position(pending_position)
                transform:set_Rotation(pending_rotation)
                update_context_position(pending_position)
                
                -- Force Warp CharacterController Every Frame
                pcall(function()
                    local game_object = transform:call("get_GameObject")
                    if game_object then
                        local cc_type = sdk.typeof("via.physics.CharacterController")
                        local cc = game_object:call("getComponent(System.Type)", cc_type)
                        if cc then
                            -- Ensure disabled
                            if cc:call("get_Enabled") then 
                                cc:call("set_Enabled", false) 
                                
                                local found = false
                                for _, c in ipairs(pending_components) do
                                    if c == cc then found = true; break end
                                end
                                if not found then
                                    table.insert(pending_components, cc)
                                    write_log("Disabled (Runtime) CharacterController")
                                end
                            end
                            
                            -- Try to set Position property first
                            local set_pos_method = cc:get_type_definition():get_method("set_Position")
                            if set_pos_method then
                                set_pos_method:call(cc, pending_position)
                            end

                            local warp_method = cc:get_type_definition():get_method("warp")
                            if warp_method then 
                                warp_method:call(cc) 
                                if os.clock() - last_noclip_log_time > 1.0 then
                                     write_log("... Noclip in progress ...")
                                     last_noclip_log_time = os.clock()
                                end
                            end
                        end
                    end
                end)
            end
        end
    end

    if check_key_down_once(VK_F5) then
        save_snapshot()
    end

    handle_load_snapshot_input()
end)

re.on_draw_ui(function()
    if imgui.tree_node("Snapshot Tool") then
        if imgui.button("Save Snapshot (F5)") then
            save_snapshot()
        end
        if imgui.same_line() then end
        if imgui.button("Load Snapshot (F9)") then
            trigger_oneshot_teleport()
        end
        if snapshot then
            imgui.text("Cached Pos: " .. tostring(snapshot.position))
        else
            imgui.text("No snapshot data.")
        end
        imgui.tree_pop()
    end
end)