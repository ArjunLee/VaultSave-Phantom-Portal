include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Collect sources for testing (excluding Main.cpp)
file(GLOB_RECURSE TEST_SOURCES
    "../src/GameLogic.cpp"
    "../src/config/*.cpp"
    "../src/core/*.cpp"
    "../src/ui/*.cpp"
    "../src/imgui/*.cpp"
)

add_executable(vspp_tests
  main_test.cpp
  ${TEST_SOURCES}
)

target_include_directories(vspp_tests PRIVATE 
  ../include
  ../src
)

target_link_libraries(vspp_tests PRIVATE
  GTest::gtest_main
  shlwapi
)

# Definitions for tests
target_compile_definitions(vspp_tests PRIVATE NOMINMAX TEST_BUILD)
