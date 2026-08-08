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
    src/broc/mp_anim_wad.cpp
    src/broc/mp_level.cpp
    src/broc/mp_util_wad.cpp
    src/broc/stubs.cpp
)
add_library(cod3_physics STATIC
    src/physics/phys_constraint_solver_multithreaded.cpp
    src/physics/physics_system.cpp
    src/physics/physics_system_internal.cpp
    src/physics/rb_ragdoll_model.cpp
    src/physics/rbc_def_custom.cpp
    src/physics/rbc_def_vehicle.cpp
    src/physics/rbc_def_contact.cpp
    src/physics/phys_util.cpp
    src/physics/rigid_body.cpp
    src/physics/rbc_def_generic.cpp
    src/physics/rbc_def_ragdoll.cpp
    src/physics/phys_contact_manifold.cpp
    src/physics/phys_gjk.cpp
    src/physics/phys_collision.cpp
    src/physics/stubs.cpp
)
add_library(cod3_ngl STATIC
    src/ngl/ngl_dx_draw.cpp
    src/ngl/ngl_dx_fsaa.cpp
    src/ngl/ngl_xb_tex_load.cpp
    src/ngl/ngl_xb_internal.cpp
    src/ngl/ngl_gpu_texture.cpp
    src/ngl/ngl_gpu_debug.cpp
    src/ngl/ngl_dx_quad.cpp
    src/ngl/ngl_quad.cpp
    src/ngl/ngl_gpu_meshedit.cpp
    src/ngl/ngl_gpu.cpp
    src/ngl/ngl_meshedit.cpp
    src/ngl/ngl_internal.cpp
    src/ngl/ngl_texture.cpp
    src/ngl/ngl_font.cpp
    src/ngl/ngl_morph.cpp
    src/ngl/ngl_xb_palette.cpp
    src/ngl/ngl_scenedump.cpp
    src/ngl/ngl_dx_scenedump.cpp
    src/ngl/ngl_dx_debug.cpp
    src/ngl/ngl_dx_texture.cpp
    src/ngl/ngl_dx_tex_create.cpp
    src/ngl/ngl_dx_state.cpp
    src/ngl/ngl_dx_core.cpp
    src/ngl/ngl_debug.cpp
    src/ngl/ngl_dx_buf.cpp
    src/ngl/ngl_dx_filters.cpp
    src/ngl/ngl_mesh.cpp
    src/ngl/ngl_lighting.cpp
    src/ngl/ngl_dx_shader.cpp
    src/ngl/ngl_scene.cpp
    src/ngl/ngl_dx_scene.cpp
    src/ngl/ngl_dx_mesh.cpp
    src/ngl/ngl_dx_font.cpp
    src/ngl/ngl_debugdraw.cpp
    src/ngl/stubs.cpp
)
add_library(cod3_render STATIC
    src/render/ShaderCommon.cpp
    src/render/ngl_aux.cpp
    src/render/cdDebugVertexDef.cpp
    src/render/cdScratchVertexDef.cpp
    src/render/cdDynamicDecalVertexDef.cpp
    src/render/cdSimpleColorShader.cpp
    src/render/cdWorldColorShader.cpp
    src/render/cdSimpleShader.cpp
    src/render/cdDecalShader.cpp
    src/render/cdGunShader.cpp
    src/render/cdCharShader.cpp
    src/render/cdPrelitShader.cpp
    src/render/cdWorldShader.cpp
    src/render/cdSimpleAlphaShader.cpp
    src/render/cdSimplePrelitShader.cpp
    src/render/cdSimpleSpecularShader.cpp
    src/render/cdSimpleUVAnimShader.cpp
    src/render/cdBackgroundShader.cpp
    src/render/cdDebugShader.cpp
    src/render/cdWheelMarkShader.cpp
    src/render/cdFlagShader.cpp
    src/render/cdDynamicDecalShader.cpp
    src/render/cdPropellerShader.cpp
    src/render/cdGunSightShader.cpp
    src/render/cdGunSightSpecularShader.cpp
    src/render/cdCharSpecularShader.cpp
    src/render/cdAirplaneMetalShader.cpp
    src/render/cdScratchShader.cpp
    src/render/cdWorldVertexLitShader.cpp
    src/render/cdWorldPointLitShader.cpp
    src/render/cdWorldBlendShader.cpp
    src/render/cdWorldBlendPointLitShader.cpp
    src/render/cdWaterShader.cpp
    src/render/cdOceanShader.cpp
    src/render/cdRiverShader.cpp
    src/render/cdGlassShader.cpp
    src/render/cdHeatHazeShader.cpp
    src/render/cdSimpleInstance.cpp
    src/render/cdGlowShader.cpp
    src/render/cdSkyShader.cpp
    src/render/cdOceanGlobals.cpp
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
    src/aeps/apsVFC.cpp
    src/aeps/apsError.cpp
    src/aeps/apsRenderer.cpp
    src/aeps/apsDebug.cpp
    src/aeps/apsVertexBuffer.cpp
    src/aeps/apsSimpleMeshRenderer.cpp
    src/aeps/apsShrimpRenderer.cpp
    src/aeps/apsBillboardRenderer.cpp
    src/aeps/apsColorBillboardRenderer.cpp
    src/aeps/apsUVARenderer.cpp
    src/aeps/apsRectangleRenderer.cpp
    src/aeps/apsColorRectangleRenderer.cpp
    src/aeps/apsUVARectangleRenderer.cpp
    src/aeps/apsColorUVARenderer.cpp
    src/aeps/apsColorUVARectangleRenderer.cpp
    src/aeps/apsBillboardNode.cpp
    src/aeps/apsRectangleNode.cpp
    src/aeps/apsColorBillboardNode.cpp
    src/aeps/apsColorRectangleNode.cpp
    src/aeps/apsUVANode.cpp
    src/aeps/apsUVARectangleNode.cpp
    src/aeps/apsColorUVANode.cpp
    src/aeps/apsColorUVARectangleNode.cpp
    src/aeps/apsMath.cpp
    src/aeps/apsCommon.cpp
    src/aeps/apsInternal.cpp
    src/aeps/apsAction.cpp
    src/aeps/apsActionList.cpp
    src/aeps/apsMemory.cpp
    src/aeps/apsGroupMgr.cpp
    src/aeps/apsSuppliedDomains.cpp
    src/aeps/apsEffectTemplate.cpp
    src/aeps/apsRegister.cpp
    src/aeps/apsEffect.cpp
    src/aeps/apsSimpleMeshNode.cpp
    src/aeps/apsShrimpNode.cpp
    src/aeps/apsPFD.cpp
    src/aeps/stubs.cpp
)
add_library(cod3_bd STATIC
    src/bd/bdReference/bdReferencable.h
    src/bd/bdUtilities/bdBitOperations.h
    src/bd/bdMemory/bdAlignedOffsetMemory.cpp
    src/bd/bdTiming/bdShortTimer.h
    src/bd/bdStopwatch.h
    src/bd/bdStopwatch.cpp
    src/bd/bdSequenceNumber.cpp
    src/bd/bdInAddr.cpp
    src/bd/bdInetAddr.cpp
    src/bd/bdAddr.cpp
    src/bd/bdPlatform.cpp
    src/bd/bdPlatformSocket.cpp
    src/bd/bdPlatformSocket-win32.cpp
    src/bd/bdSocket.cpp
    src/bd/bdQoSProbe.cpp
    src/bd/bdSecurityKeyMap.cpp
    src/bd/bdSocketRouter.cpp
    src/bd/bdUnicastConnection.cpp
    src/bd/bdConnectionStore.cpp
    src/bd/bdNet.cpp
    src/bd/bdGameInfo.cpp
    src/bd/bdAddressMap.cpp
    src/bd/bdDiscoveryListener.cpp
    src/bd/bdDiscoveryServer.cpp
    src/bd/bdDiscoveryClient.cpp
    src/bd/bdConnectionStatistics.cpp
    src/bd/bdConnection.cpp
    src/bd/bdConnectionListener.cpp
    src/bd/bdDispatcher.cpp
    src/bd/bdLoopbackConnection.cpp
    src/bd/bdChunk.cpp
    src/bd/bdMessage.cpp
    src/bd/bdReceivedMessage.cpp
    src/bd/bdSAckChunk.cpp
    src/bd/bdHeartbeatChunk.cpp
    src/bd/bdShutdownChunk.cpp
    src/bd/bdCookie.cpp
    src/bd/bdInitChunk.cpp
    src/bd/bdDataChunk.cpp
    src/bd/bdPacket.cpp
    src/bd/bdReliableSendWindow.cpp
    src/bd/bdReliableReceiveWindow.cpp
    src/bd/bdUnreliableSendWindow.cpp
    src/bd/bdUnreliableReceiveWindow.cpp
    src/bd/bdSessionInfo.cpp
    src/bd/bdSession.cpp
    src/bd/bdSessionListener.cpp
    src/bd/bdSessionHandler.cpp
    src/bd/bdSessionInterceptor.cpp
    src/bd/bdDispatchInterceptor.cpp
    src/bd/bdQoSProbeListener.cpp
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
add_library(cod3_game_logic STATIC src/game/logic/stubs_game.cpp src/game/CallFunctor.cpp)
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
add_library(cod3_game_sv STATIC
    src/game/sv/stubs_game.cpp
    src/game/sv/sv_globals.cpp
    src/game/sv/sv_ccmds.cpp
    src/game/sv/sv_init.cpp
    src/game/sv/sv_game.cpp
    src/game/sv/sv_main.cpp
    src/game/sv/sv_client.cpp
    src/game/sv/sv_snapshot.cpp
    src/game/sv/sv_misc.cpp
    src/game/sv/sv_world.cpp
)
add_library(cod3_game_platform_xbox STATIC
    src/game/platform_xbox/MemoryUnit.cpp
    src/game/platform_xbox/XBoxStorage.cpp
    src/game/platform_xbox/GameXbox.cpp
    src/game/platform_xbox/XboxLive.cpp
    src/game/platform_xbox/MPLiveEngine.cpp
    src/game/platform_xbox/XboxLiveMenus.cpp
)
add_library(cod3_game_nextgen STATIC
    src/game/nextgen/stubs_game.cpp
    src/game/nextgen/nextgen.cpp
)

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
