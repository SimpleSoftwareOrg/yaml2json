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

# Production optimization flags
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|AppleClang")
        set(OPT_FLAGS "-Ofast -DNDEBUG -flto=thin -ffast-math -funroll-loops -fomit-frame-pointer -fvisibility=hidden")
        
        if(NOT CMAKE_CROSSCOMPILING)
            set(OPT_FLAGS "${OPT_FLAGS} -march=native -mtune=native")
        endif()
        
        set(CMAKE_CXX_FLAGS_RELEASE "${OPT_FLAGS}")
        
        # Linker optimizations
        if(APPLE)
            set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-Wl,-dead_strip")
        else()
            set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-Wl,--gc-sections")
        endif()
        
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        set(OPT_FLAGS "-O3 -DNDEBUG -flto -ffast-math -funroll-loops -fomit-frame-pointer -fvisibility=hidden")
        
        if(NOT CMAKE_CROSSCOMPILING)
            set(OPT_FLAGS "${OPT_FLAGS} -march=native -mtune=native")
        endif()
        
        set(CMAKE_CXX_FLAGS_RELEASE "${OPT_FLAGS}")
        set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-Wl,--gc-sections")
        
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        set(CMAKE_CXX_FLAGS_RELEASE "/O2 /DNDEBUG /GL /Gy")
        set(CMAKE_EXE_LINKER_FLAGS_RELEASE "/LTCG /OPT:REF /OPT:ICF")
    endif()
    
    # Enable Link Time Optimization if supported
    include(CheckIPOSupported)
    check_ipo_supported(RESULT ipo_supported OUTPUT ipo_error)
    if(ipo_supported)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
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

# Configure rapidyaml options
set(RYML_BUILD_TOOLS OFF CACHE BOOL "Disable rapidyaml tools")
set(RYML_BUILD_TESTS OFF CACHE BOOL "Disable rapidyaml tests")
set(RYML_BUILD_BENCHMARKS OFF CACHE BOOL "Disable rapidyaml benchmarks")

FetchContent_MakeAvailable(rapidyaml CLI11)

# Create executable and link libraries
add_executable(yaml2json src/main.cpp)
target_link_libraries(yaml2json PRIVATE ryml CLI11::CLI11)

# Install target
install(TARGETS yaml2json RUNTIME DESTINATION bin) 