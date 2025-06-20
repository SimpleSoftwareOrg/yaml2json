cmake_minimum_required(VERSION 3.15)
project(yaml2json VERSION 1.0.0 DESCRIPTION "High-performance YAML to JSON converter")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set default build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Production optimization flags
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    # Aggressive optimisation flags tuned for single-file workloads
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang")
        # Clang-specific optimizations
        set(OPT_FLAGS "-Ofast -DNDEBUG -flto=thin -ffast-math -funroll-loops -fstrict-aliasing -fomit-frame-pointer -fvisibility=hidden -ffunction-sections -fdata-sections")
        
        # Platform-specific optimizations
        if(NOT CMAKE_CROSSCOMPILING)
            set(OPT_FLAGS "${OPT_FLAGS} -march=native -mtune=native")
        endif()
        
        set(CMAKE_CXX_FLAGS_RELEASE "${OPT_FLAGS}")
        set(CMAKE_C_FLAGS_RELEASE "${OPT_FLAGS}")
        
        # Linker flags for Clang
        if(APPLE)
            set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -Wl,-dead_strip")
        else()
            set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -Wl,--gc-sections")
        endif()
        
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        # GCC-specific optimizations
        set(OPT_FLAGS "-O3 -DNDEBUG -flto -ffast-math -funroll-loops -fstrict-aliasing -fomit-frame-pointer -fvisibility=hidden -ffunction-sections -fdata-sections")
        
        if(NOT CMAKE_CROSSCOMPILING)
            set(OPT_FLAGS "${OPT_FLAGS} -march=native -mtune=native")
        endif()
        
        set(CMAKE_CXX_FLAGS_RELEASE "${OPT_FLAGS}")
        set(CMAKE_C_FLAGS_RELEASE "${OPT_FLAGS}")
        set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -Wl,--gc-sections")
        
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        # MSVC-specific optimizations
        set(CMAKE_CXX_FLAGS_RELEASE "/O2 /DNDEBUG /GL /Gy")
        set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /LTCG /OPT:REF /OPT:ICF")
    endif()
    
    # Enable Link Time Optimization if supported
    include(CheckIPOSupported)
    check_ipo_supported(RESULT ipo_supported OUTPUT ipo_error)
    if(ipo_supported)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
    endif()
endif()

# Include FetchContent module for downloading dependencies
include(FetchContent)

# Fetch rapidyaml with specific optimizations
FetchContent_Declare(
    rapidyaml
    GIT_REPOSITORY https://github.com/biojppm/rapidyaml.git
    GIT_TAG        v0.9.0  # Use stable version
    GIT_SHALLOW    TRUE
)

# Configure rapidyaml options before making it available
set(RYML_BUILD_TOOLS OFF CACHE BOOL "Disable rapidyaml tools")
set(RYML_BUILD_TESTS OFF CACHE BOOL "Disable rapidyaml tests")
set(RYML_BUILD_BENCHMARKS OFF CACHE BOOL "Disable rapidyaml benchmarks")

FetchContent_MakeAvailable(rapidyaml)

# Create executable
add_executable(yaml2json src/main.cpp)

# Link rapidyaml
target_link_libraries(yaml2json PRIVATE ryml)

# Set target properties
set_target_properties(yaml2json PROPERTIES
    OUTPUT_NAME yaml2json
)

# Include directories
target_include_directories(yaml2json PRIVATE src/)

# Install target
install(TARGETS yaml2json
    RUNTIME DESTINATION bin
) 