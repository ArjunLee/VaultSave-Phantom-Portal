cmake_minimum_required(VERSION 3.10)
project(VaultSavePhantomPortal)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/utf-8)
endif()

# Include directories
include_directories(include)
include_directories(src)

# Source files
file(GLOB_RECURSE SOURCES 
    "src/Main.cpp"
    "src/GameLogic.cpp"
    "src/config/*.cpp"
    "src/core/*.cpp"
    "src/ui/*.cpp"
    "src/imgui/*.cpp"
)

# Exclude rendering folder if not needed (assuming it is for standalone hooks)
# If compilation fails due to missing symbols from rendering/, we can add them back.

# Create Shared Library (DLL)
add_library(${PROJECT_NAME} SHARED ${SOURCES})

# Definitions
target_compile_definitions(${PROJECT_NAME} PRIVATE NOMINMAX)

# Libraries
target_link_libraries(${PROJECT_NAME} PRIVATE shlwapi) # For path operations if needed

# Copy DLL to game folder (Optional, but good for dev)
# set(GAME_MOD_DIR "E:/SteamLibrary/steamapps/common/RESIDENT EVIL requiem BIOHAZARD requiem/reframework/plugins")
# add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${PROJECT_NAME}> "${GAME_MOD_DIR}/${PROJECT_NAME}.dll"
# )

# Testing
enable_testing()
add_subdirectory(tests)
