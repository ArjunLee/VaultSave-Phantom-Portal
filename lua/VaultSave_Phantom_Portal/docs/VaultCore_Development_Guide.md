# VaultCore (C++) 插件化开发指引

本此文档旨在指导开发者将 `VaultSave_Phantom_Portal.lua` 的核心逻辑迁移至 C++ DLL (`VaultCore.dll`)，以实现源码保护（闭源）与性能优化。

## 1. 开发环境准备

### 1.1 必要工具
- **Visual Studio 2022** (安装 "Desktop development with C++" 工作负载)
- **CMake** (3.20 或更高版本)
- **Git**

### 1.2 依赖库
- **REFramework SDK**: 用于与 RE 引擎交互的核心 API。
- **ImGui**: 用于绘制 UI (需与 REFramework 的渲染后端 D3D11/D3D12 兼容)。
- **MinHook** (可选): 如果需要 Hook 引擎底层函数。
- **nlohmann/json**: 用于替代 Lua 的 json 读写。

## 2. 项目初始化

### 2.1 目录结构推荐
```
VaultCore/
├── CMakeLists.txt          # 构建脚本
├── src/
│   ├── Main.cpp            # 插件入口 (reframework_plugin_initialize)
│   ├── Plugin.cpp          # 核心逻辑实现
│   ├── Plugin.hpp
│   ├── UI.cpp              # ImGui 绘制逻辑
│   └── Utils.cpp           # 辅助工具 (日志, 字符串转换)
├── include/
│   └── reframework/        # 复制自 REFramework/include/reframework
└── lib/                    # 依赖库
```

### 2.2 CMake 配置示例
需要在 `CMakeLists.txt` 中定义 DLL 项目，并链接 REFramework API。

```cmake
cmake_minimum_required(VERSION 3.20)
project(VaultCore)

set(CMAKE_CXX_STANDARD 20)

add_library(VaultCore SHARED src/Main.cpp src/Plugin.cpp src/UI.cpp)

# 包含 REFramework 头文件
target_include_directories(VaultCore PRIVATE include)

# 链接库 (如需要)
# target_link_libraries(VaultCore PRIVATE ...)
```

## 3. 核心迁移指南 (Lua -> C++)

### 3.1 插件入口 (Main.cpp)
C++ 插件必须导出一个名为 `reframework_plugin_initialize` 的函数。

```cpp
#include <reframework/API.hpp>

extern "C" __declspec(dllexport) void reframework_plugin_initialize(const REFrameworkPluginInitializeParam* param) {
    // 1. 初始化 API
    reframework::API::initialize(param);

    // 2. 注册回调
    const auto functions = param->functions;
    functions->on_frame_gui(on_draw_ui); // UI 绘制
    functions->on_frame(on_frame);       // 逻辑帧
}
```

### 3.2 API 调用对照表

| 功能 | Lua (Snapshot_Mod.lua) | C++ (VaultCore) |
| :--- | :--- | :--- |
| **获取单例** | `sdk.get_managed_singleton("app.Foo")` | `reframework::API::get()->get_managed_singleton("app.Foo")` |
| **查找类型** | `sdk.find_type_definition("app.Bar")` | `reframework::API::get()->tdb()->find_type("app.Bar")` |
| **调用方法** | `obj:call("MethodName", args)` | `obj->call("MethodName", args)` (需封装 invoke) |
| **获取字段** | `obj:get_field("FieldName")` | `field->get_data<T>(obj)` |
| **UI 绘制** | `imgui.text("Hello")` | `ImGui::Text("Hello")` |
| **JSON** | `json.load_file(...)` | `nlohmann::json::parse(...)` |

### 3.3 关键逻辑移植：获取玩家坐标

**Lua:**
```lua
local player = sdk.get_managed_singleton("app.PlayerManager"):call("get_CurrentPlayer")
local transform = player:call("get_Transform")
local pos = transform:call("get_Position")
```

**C++ (伪代码):**
```cpp
auto api = reframework::API::get();
auto player_manager = api->get_managed_singleton("app.PlayerManager");
auto get_player_method = player_manager->get_type_definition()->find_method("get_CurrentPlayer");
auto player = get_player_method->invoke(player_manager);

if (player) {
    auto transform = player->call<reframework::API::ManagedObject*>("get_Transform");
    auto pos = transform->call<Vector3f>("get_Position");
    // 处理 pos...
}
```

### 3.4 UI 移植 (ImGui)
REFramework 的 C++ 示例插件中包含了完整的 D3D11/D3D12 Hook 代码 (`examples/example_plugin/rendering/`)。
你需要：
1.  复制 `rendering` 文件夹到你的项目。
2.  在 `reframework_plugin_initialize` 中调用 `initialize_imgui()`。
3.  在 `on_draw_ui` 回调中使用标准的 ImGui C++ API 绘制窗口。

**注意**：Lua 中的 `imgui.tree_node` 在 C++ 中对应 `ImGui::TreeNode`，需注意 `ImGui::TreePop()` 的配对使用。

## 4. 源码保护与发布

### 4.1 编译
1.  在 Visual Studio 中选择 **Release** 模式。
2.  生成解决方案。
3.  输出文件：`VaultCore.dll`。

### 4.2 混淆与加固 (可选 - 进阶保护)
虽然编译为 DLL 已经是机器码（汇编），比 Lua 脚本安全得多，但仍可被逆向。
- **符号剥离**: 确保 Release 编译不生成 PDB，或不发布 PDB 文件。
- **字符串加密**: 使用编译期字符串加密库（如 `skCrypt`）隐藏 "app.PlayerManager" 等关键字符串，防止通过字符串搜索定位逻辑。
- **控制流平坦化 (OLLVM)**: 增加逆向分析难度。

### 4.3 安装方式
用户只需将 `VaultCore.dll` 放入游戏目录下的 `reframework/plugins/` 文件夹即可，无需任何 Lua 脚本。

## 5. 常见问题 (FAQ)

- **Q: 为什么 C++ 版本比 Lua 版本崩得更快？**
  - A: C++ 是强类型且内存不安全的。空指针解引用 (`nullptr`) 会直接导致游戏崩溃。**必须**在每次调用 API 返回指针后检查 `if (ptr == nullptr)`。

- **Q: 如何调试？**
  - A: 使用 `OutputDebugString` 输出日志，配合 **DebugView** 查看。或者在 Debug 模式下附加到游戏进程进行断点调试。

- **Q: 现有存档 (data.json) 兼容吗？**
  - A: 只要 C++ 版本读取相同的 `reframework/data/VaultSavePhantomTravel/data.json` 路径，并使用相同的 JSON 结构，即可完全兼容。
