cmake_minimum_required(VERSION 3.20)
project(VaultSave_RE9_PhantomPortal)

set(CMAKE_CXX_STANDARD 20)

# 包含目录
include_directories(include src)

# 源文件列表
file(GLOB_RECURSE SOURCES "src/*.cpp")

# 生成动态链接库 (DLL)
# 产物名称统一为 VaultSave_RE9_PhantomPortal.dll
add_library(VaultSave_RE9_PhantomPortal SHARED ${SOURCES})

# 链接选项 (Windows特有)
if(WIN32)
    target_link_libraries(VaultSave_RE9_PhantomPortal PRIVATE kernel32 user32 gdi32 d3d11 d3d12 dxgi d3dcompiler)
endif()
