# COD3 — Reconstructed Source Tree

Based on source-path strings in the binary plus linker map Lib:Object attribution.

## Game module (c:\cod\code\game\)

```
c:\cod\code\game\
├── g_active.cpp              actor processing
├── g_actor_prone.cpp         prone behavior
├── g_client.cpp              client-side game logic
├── g_combat.cpp              damage / combat system
├── g_hudelem.cpp             HUD elements
├── g_items.cpp               items / pickups
├── g_main.cpp                game init + main loop
├── g_misc.cpp                misc systems
├── g_mover.cpp               moving entities
├── g_spawn.cpp               entity spawning
├── g_syscalls.cpp            VM system-call boundary
├── g_trigger.cpp             trigger system
├── g_weapon.cpp              weapons
├── g_utils.cpp               utilities
├── g_scr_main.cpp            script integration
├── g_scr_mover.cpp           scripted movers
├── g_scr_vehicle.cpp         scripted vehicles
├── g_vehicle_path.cpp        vehicle pathfinding
├── bg_pmove.cpp              player movement (shared)
├── cvar.cpp                  console variables
├── PakFile.cpp               asset packaging
├── DecodePakFile.cpp         asset decompression
├── util_str.cpp              string utilities
├── vehicle_collision.cpp     vehicle collision bridge
│
├── EntityManager.h           entity lifecycle
├── sentient.h                AI sentient base
├── actor.h                   actor / NPC
├── InteractionController.h   use / interact system
├── RumbleEffect.h            controller rumble effect
├── RumbleManager.h           rumble effect manager
├── PathNodeMgr.h             path node manager
├── FreeList.h                free-list allocator
├── gamepause.h               pause system
└── g_local.h                 game-local types
```

## Tech Library (c:\cod\code\tl\)

```
c:\cod\code\tl\
├── physics\
│   ├── include\
│   │   ├── rigid_body.h
│   │   ├── phys_mem.h
│   │   ├── phys_math.h
│   │   ├── phys_array_base.inc
│   │   ├── phys_memory_pool_base.inc
│   │   └── collision\
│   │       ├── phys_gjk_cache_system.h
│   │       └── phys_contact_manifold.h
│   └── source\
│       ├── phys_gjk.cpp
│       ├── phys_collision.cpp
│       ├── phys_contact_manifold.cpp
│       ├── phys_util.cpp
│       ├── rigid_body.cpp
│       ├── rbc_def_generic.cpp
│       ├── rbc_def_ragdoll.cpp
│       ├── rbc_def_vehicle.cpp
│       ├── rbc_def_contact.cpp
│       ├── rbc_def_custom.cpp
│       ├── rb_ragdoll_model.cpp
│       ├── physics_system.cpp
│       ├── physics_system_internal.cpp
│       └── phys_constraint_solver_multithreaded.cpp
├── cdl\
│   └── source\
│       ├── cdl_common.cpp
│       ├── cdl_gjk.cpp
│       └── cdl_base.cpp
```

## BD Networking Library (Treyarch internal)

```
bd\
├── bdCore\
│   ├── bdString.cpp         bdAddr.cpp        bdInetAddr.cpp
│   ├── bdBitBuffer.cpp      bdBytePacker.cpp  bdByteBuffer.cpp
│   ├── bdMemory.cpp         bdMallocMemory.cpp bdAlignedOffsetMemory.cpp
│   ├── bdSocket.cpp         bdCommonAddr-xbox.cpp bdSecurityKey-xbox.cpp
│   ├── bdStopwatch.cpp      bdRandom.cpp      bdTrulyRandom.cpp
│   ├── bdHMacSHA1.cpp       bdHMac.cpp
│   ├── bdSequenceNumber.cpp bdBitOperations.cpp
│   ├── bdMutex.cpp          bdReferencable.cpp
│   ├── bdLog.cpp            bdLogChannel.cpp  bdLogSubscriber.cpp
│   ├── bdShortTimer.cpp     bdCore.cpp
├── bdConnection\
│   ├── bdConnection.cpp     bdConnectionStore.cpp bdConnectionStatistics.cpp
│   ├── bdConnectionListener.cpp
│   ├── bdUnicastConnection.cpp bdLoopbackConnection.cpp
│   ├── bdPacket.cpp         bdMessage.cpp     bdDataChunk.cpp
│   ├── bdChunk.cpp          bdInitChunk.cpp   bdInitAckChunk.cpp
│   ├── bdSAckChunk.cpp      bdCookie.cpp      bdCookieAckChunk.cpp
│   ├── bdCookieEchoChunk.cpp
│   ├── bdHeartbeatChunk.cpp bdHeartbeatAckChunk.cpp
│   ├── bdShutdownChunk.cpp  bdShutdownAckChunk.cpp bdShutdownCompleteChunk.cpp
│   ├── bdReliableSendWindow.cpp bdReliableReceiveWindow.cpp
│   ├── bdUnreliableSendWindow.cpp bdUnreliableReceiveWindow.cpp
│   └── bdReceivedMessage.cpp
├── bdSocket\
│   ├── bdQoSProbe-xbox.cpp  bdQoSProbeListener.cpp
│   ├── bdAddressMap-xbox.cpp bdSocketRouter-xbox.cpp
├── bdNet\
│   ├── bdNet-xbox.cpp       bdGameInfo.cpp    bdGameInfoFactory.cpp
│   ├── bdDiscoveryServer.cpp bdDiscoveryClient.cpp
│   ├── bdDiscoveryListener.cpp
│   ├── bdDispatcher.cpp     bdDispatchInterceptor.cpp
├── bdPeer\
│   ├── bdSession.cpp        bdSessionListener.cpp
│   ├── bdSessionInfo.cpp    bdSessionHandler.cpp bdSessionInterceptor.cpp
├── bdPlatform\
│   ├── bdPlatformSocket-win32.cpp bdPlatformSocket.cpp
│   ├── bdPlatformMutex-xbox.cpp   bdPlatformTiming-xbox.cpp
│   ├── bdPlatformTrulyRandom-xbox.cpp bdInAddr.cpp
```

## Other Treyarch engine libraries

```
ngl_xboxr\          NGL graphics library
├── ngl_scene.cpp        ngl_debug.cpp       ngl_font.cpp
├── ngl_mesh.cpp         ngl_quad.cpp        ngl_texture.cpp
├── ngl_gpu.cpp          ngl_morph.cpp       ngl_scenedump.cpp
├── ngl_meshedit.cpp     ngl_gpu_meshedit.cpp ngl_gpu_texture.cpp
├── ngl_gpu_debug.cpp    ngl_debugdraw.cpp
├── ngl_lighting.cpp     ngl_internal.cpp
├── ngl_xb_palette.cpp   ngl_xb_tex_load.cpp ngl_xb_internal.cpp
└── ngl_dx_*.cpp         (D3D backend — to be replaced by D3D11)

render_xboxr\       Shader/material system
├── ShaderCommon.cpp     ngl_aux.cpp
├── cdWorldShader.cpp        cdWorldVertexLitShader.cpp   cdWorldPointLitShader.cpp
├── cdWorldBlendShader.cpp   cdWorldBlendPointLitShader.cpp cdWorldColorShader.cpp
├── cdCharShader.cpp         cdCharSpecularShader.cpp   cdPrelitShader.cpp
├── cdSimpleShader.cpp       cdSimplePrelitShader.cpp   cdSimpleAlphaShader.cpp
├── cdSimpleSpecularShader.cpp cdSimpleColorShader.cpp  cdSimpleUVAnimShader.cpp
├── cdSimpleInstance.cpp     cdScratchShader.cpp  cdScratchVertexDef.cpp
├── cdDecalShader.cpp        cdDynamicDecalShader.cpp cdDynamicDecalVertexDef.cpp
├── cdGlassShader.cpp        cdSkyShader.cpp      cdWaterShader.cpp
├── cdOceanShader.cpp        cdOceanGlobals.cpp   cdRiverShader.cpp
├── cdFlagShader.cpp         cdGlowShader.cpp     cdHeatHazeShader.cpp
├── cdGunShader.cpp          cdGunSightShader.cpp cdGunSightSpecularShader.cpp
├── cdPropellerShader.cpp    cdDebugShader.cpp    cdDebugVertexDef.cpp
├── cdBackgroundShader.cpp   cdWheelMarkShader.cpp cdAirplaneMetalShader.cpp

nal_xboxr\          Animation engine
├── nal_generic.cpp     nal_anim.cpp       nal_init.cpp
├── nal_skeleton.cpp    nal_stream.cpp     nal_sceneanim.cpp
├── nal_cache.cpp       nal_ik.cpp         nal_generic_component.cpp
└── nal_initlist.cpp

nsl_xboxr\          Sound library
├── nslSource.cpp      nslWaveBank.cpp    nslMaster.cpp
├── nslCompat.cpp      nslWave.cpp        nslDriverXBOXDSOUND.cpp
├── nslGroup.cpp       nslListener.cpp    nslInit.cpp
├── nslPriority.cpp    nslAram.cpp        nslVoice.cpp
├── nslWaveBankLoader.cpp  nslWaveBankSort.cpp nslWaveBankFixup.cpp
├── nslUpdate.cpp      nslMemory.cpp

nfl_xboxr\          File/streaming library
├── nfl_system.cpp     nfl_driver.cpp     nfl_win32_driver.cpp
├── nfl_common.cpp     nfl_debug.cpp      nfl_xbox_driver.cpp
├── tx.cpp             txPath.cpp        txSlotPool.cpp
└── tx_tl.cpp

aeps_xboxr\         Particle/effects system ("APS")
├── apsRegister.cpp       apsMemory.cpp       apsEffect.cpp
├── apsEffectTemplate.cpp apsSuppliedActions.cpp apsSuppliedDomains.cpp
├── apsCommon.cpp         apsInternal.cpp     apsMath.cpp
├── apsGroup.cpp          apsGroupMgr.cpp     apsAction.cpp
├── apsActionList.cpp     apsRenderer.cpp     apsDebug.cpp
├── apsError.cpp          apsVFC.cpp          apsPFD.cpp
├── apsVertexBuffer.cpp
├── (Many renderer implementations: SimpleMesh, Billboard, Rectangle,
│    UVARectangle, UVA, Color variants, Shrimp)

core_xboxr\         Core utility library
├── PoolAllocator.cpp     ae_fixed_string.cpp
├── AeAssert.cpp          ae_controller.cpp   AeHash.cpp
```

## Other libraries

```
tl_xboxr\           tl_system.cpp, tl_instbank.cpp, tl_initlist.cpp
cdl_xboxr\          cdl_common.cpp, cdl_gjk.cpp, cdl_base.cpp
mem_mp_xboxr\       mem_lib.cpp, dlmalloc.cpp, ae_heap.cpp, mem_lib_platform.cpp
controller_xboxr\   controller_xbox.cpp, controller.cpp
jobqueue_xboxr\     jobqueue.cpp, jobqueue_legacy.cpp
peripherals_xboxr\  XboxMemoryUnit.cpp, MemoryUnit.cpp
apk_xboxr\          apk.cpp
inplace_xboxr\      PtrFixupTable.cpp, InplaceTree.cpp, InplaceFileBuilder.cpp
zlib_xboxr\         (use modern zlib instead)
```

## Note

This tree is reconstructed from linker map `Lib:Object` entries and source-path
strings found in the binary. Exact original paths will be refined during Phase 1
when PDB source-file data is extracted via IDA Python.
