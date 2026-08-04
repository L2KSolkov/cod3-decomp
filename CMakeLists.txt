cmake_minimum_required(VERSION 3.20)
project(cod3mp VERSION 0.1.0 LANGUAGES CXX)

# 32-bit x86 only — the code assumes 4-byte pointers
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compile flags matching VC7.1 /O2 behavior
if(MSVC)
    add_compile_options(/fp:precise /GR- /Zi)
    # /GR- disables RTTI (match original); enable if needed for type reconstruction
    # Phase 0: allow unresolved externals for green-link
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /FORCE:UNRESOLVED /DEBUG")
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_SOURCE_DIR}/src/bd)
include_directories(${CMAKE_SOURCE_DIR}/platform/win32)
include_directories(${CMAKE_SOURCE_DIR}/platform/xbox_shim)

# ============================================================================
# Engine libraries (Treyarch)
# ============================================================================
add_library(cod3_broc STATIC
    src/broc/Broc.cpp
    src/broc/mp_level.cpp
    src/broc/stubs.cpp
)
add_library(cod3_physics STATIC
    src/physics/stubs.cpp
)
add_library(cod3_ngl STATIC
    src/ngl/stubs.cpp
)
add_library(cod3_render STATIC
    src/render/stubs.cpp
)
add_library(cod3_animation STATIC
    src/animation/nal.cpp
    src/animation/stubs.cpp
)
add_library(cod3_sound STATIC
    src/sound/nsl.cpp
    src/sound/stubs.cpp
)
add_library(cod3_filesystem STATIC
    src/filesystem/apk.cpp
    src/filesystem/inplace.cpp
    src/filesystem/nfl.cpp
    src/filesystem/stubs.cpp
)
add_library(cod3_nvl STATIC
    src/nvl/nvl.cpp
    src/nvl/stubs.cpp
)
add_library(cod3_aeps STATIC
    src/aeps/apsSuppliedActions.cpp
    src/aeps/apsGroup.cpp
    src/aeps/stubs.cpp
)
add_library(cod3_bd STATIC
    src/bd/bdReference/bdReferencable.h
    src/bd/bdUtilities/bdBitOperations.h
    src/bd/bdMemory/bdAlignedOffsetMemory.cpp
    src/bd/bdTiming/bdShortTimer.h
    src/bd/bdPlatform.cpp
    src/bd/stubs.cpp
)
add_library(cod3_core STATIC
    src/core/PoolAllocator.cpp
    src/core/ae_string_support.cpp
    src/core/ae_assert.cpp
    src/core/ae_hash.cpp
    src/core/ae_controller.cpp
    src/core/cdl_common.cpp
    src/core/cdl_gjk.cpp
    src/core/cdl_base.cpp
    src/core/tl_system.cpp
    src/core/tl_instbank.cpp
    src/core/memory/mem_heap.cpp
    src/core/memory/stubs.cpp
)
add_library(cod3_input STATIC
    src/input/controller.cpp
    src/input/stubs.cpp
)
add_library(cod3_threading STATIC
    src/threading/jobqueue.cpp
    src/threading/stubs.cpp
)

# ============================================================================
# Game code (amalgamated objects — split per subdir)
# ============================================================================
add_library(cod3_game_main STATIC src/game/main/stubs_game.cpp)
add_library(cod3_game_logic STATIC src/game/logic/stubs_game.cpp)
add_library(cod3_game_script STATIC src/game/script/stubs_game.cpp)
add_library(cod3_game_mp STATIC src/game/mp/stubs_game.cpp)
add_library(cod3_game_mpshell STATIC src/game/mpshell/stubs_game.cpp)
add_library(cod3_game_game STATIC src/game/game/stubs_game.cpp)
add_library(cod3_game_core STATIC src/game/core/stubs_game.cpp)
add_library(cod3_game_game2 STATIC src/game/game2/stubs_game.cpp)
add_library(cod3_game_render STATIC src/game/render/stubs_game.cpp)
add_library(cod3_game_anim STATIC src/game/anim/stubs_game.cpp)
add_library(cod3_game_mpactors STATIC src/game/mpactors/stubs_game.cpp)
add_library(cod3_game_cg STATIC src/game/cg/stubs_game.cpp)
add_library(cod3_game_cl STATIC src/game/cl/stubs_game.cpp)
add_library(cod3_game_streamer STATIC src/game/streamer/stubs_game.cpp)
add_library(cod3_game_physics STATIC src/game/physics/stubs_game.cpp)
add_library(cod3_game_sv STATIC src/game/sv/stubs_game.cpp)
add_library(cod3_game_platform_xbox STATIC src/game/platform_xbox/stubs_game.cpp)
add_library(cod3_game_nextgen STATIC src/game/nextgen/stubs_game.cpp)

# ============================================================================
# Platform shim (Win32 implementations of Xbox APIs)
# ============================================================================
add_library(cod3_xbox_shim STATIC
    platform/xbox_shim/xbox_shim.cpp
)

# ============================================================================
# Main executable
# ============================================================================
add_executable(cod3mp
    platform/win32/main.cpp
)
target_link_libraries(cod3mp PRIVATE
    cod3_broc
    cod3_physics
    cod3_ngl
    cod3_render
    cod3_animation
    cod3_sound
    cod3_filesystem
    cod3_nvl
    cod3_aeps
    cod3_bd
    cod3_core
    cod3_input
    cod3_threading
    cod3_game_main
    cod3_game_logic
    cod3_game_script
    cod3_game_mp
    cod3_game_mpshell
    cod3_game_game
    cod3_game_core
    cod3_game_game2
    cod3_game_render
    cod3_game_anim
    cod3_game_mpactors
    cod3_game_cg
    cod3_game_cl
    cod3_game_streamer
    cod3_game_physics
    cod3_game_sv
    cod3_game_platform_xbox
    cod3_game_nextgen
    cod3_xbox_shim
)
target_compile_definitions(cod3mp PRIVATE COD3_WIN32=1)
