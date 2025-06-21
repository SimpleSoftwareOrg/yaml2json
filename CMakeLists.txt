cmake_minimum_required(VERSION 3.15)
project(yaml2json DESCRIPTION "High-performance YAML to JSON converter")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Generate build date for version information
string(TIMESTAMP BUILD_DATE "%Y-%m-%d %H:%M:%S")
add_compile_definitions(BUILD_DATE="${BUILD_DATE}")

# Set default build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(-O3)
        # Only use -march=native when not cross-compiling
        if(NOT CMAKE_CROSSCOMPILING)
            add_compile_options(-march=native)
        endif()
    endif()
elseif(MSVC)
    add_compile_options(/W4)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(/O2)
    endif()
endif()

# Fetch rapidyaml
include(FetchContent)

# Fetch CLI11
FetchContent_Declare(
    CLI11
    GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
    GIT_TAG        v2.4.1
    GIT_SHALLOW    TRUE
)

FetchContent_Declare(
    rapidyaml
    GIT_REPOSITORY https://github.com/biojppm/rapidyaml.git
    GIT_TAG        v0.9.0
    GIT_SHALLOW    TRUE
)

# Fetch Google Test
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
    GIT_SHALLOW    TRUE
)

# Configure rapidyaml options
set(RYML_BUILD_TOOLS OFF CACHE BOOL "Disable rapidyaml tools")
set(RYML_BUILD_TESTS OFF CACHE BOOL "Disable rapidyaml tests")

# For Windows, prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(CLI11 rapidyaml googletest)

# Create library with core functionality
add_library(yaml2json_lib STATIC
    src/lib/FileReader.cpp
    src/lib/YamlToJsonConverter.cpp
    src/lib/JsonFormatter.cpp
    src/lib/ErrorHandler.cpp
)

target_include_directories(yaml2json_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/lib
)

target_link_libraries(yaml2json_lib PUBLIC
    ryml::ryml
)

# Main executable
add_executable(yaml2json src/main.cpp)

target_link_libraries(yaml2json PRIVATE
    yaml2json_lib
    CLI11::CLI11
)

# Enable testing (only for native builds)
if(NOT CMAKE_CROSSCOMPILING)
    enable_testing()

    # Test executable
    add_executable(yaml2json_test
        tests/test_main.cpp
        tests/FileReaderTest.cpp
        tests/YamlToJsonConverterTest.cpp
        tests/JsonFormatterTest.cpp
        tests/ErrorHandlerTest.cpp
        tests/IntegrationTest.cpp
        tests/CliCompatibilityTest.cpp
    )

    target_link_libraries(yaml2json_test PRIVATE
        yaml2json_lib
        GTest::gtest
        GTest::gtest_main
        GTest::gmock
    )

    # Add test discovery
    include(GoogleTest)
    gtest_discover_tests(yaml2json_test)

    # Copy test data to build directory
    file(COPY tests/test_data DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/tests/)
endif()

# Install target
install(TARGETS yaml2json DESTINATION bin) 