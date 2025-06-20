cmake_minimum_required(VERSION 3.15)
project(yaml2json DESCRIPTION "High-performance YAML to JSON converter")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

FetchContent_MakeAvailable(rapidyaml)

# Create executable and link libraries
add_executable(yaml2json src/main.cpp)
target_link_libraries(yaml2json PRIVATE ryml)

# Install target
install(TARGETS yaml2json RUNTIME DESTINATION bin) 