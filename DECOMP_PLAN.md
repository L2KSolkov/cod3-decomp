# Call of Duty 3 (Xbox) — Win32 Decompilation Plan

**Read this whole file before doing any work. Follow it exactly. When in doubt, STOP and escalate — do not improvise.**

## 1. What this project is

Call of Duty 3 multiplayer for the original Xbox (Treyarch, October 2006). We have the **release XBE** `codmp_xboxr.xbe` with **embedded PDB symbols and full linker map** — 98.9% of the 74,958 functions in IDA are named, and the map file attributes every public symbol to its `Lib:Object`. The goal is a complete, compilable C++ reimplementation, targeting **Win32 first**, with no plan to preserve Xbox compatibility.

**Critical difference from similar projects**: this is a **release build** with full compiler optimizations (`/O2`). Hex-Rays output will have inlined functions, register reuse, reordered instructions, loop transformations, and strength-reduced arithmetic. This is a *reconstruction* project — your job is to interpret the optimized disassembly back to the original C++ intent, not transcribe verbatim. Disassembly is the ground truth; Hex-Rays is a starting point. When they disagree, disassembly wins.

## 2. Ground-truth assets (never modify these)

| Asset | Path | Use |
|---|---|---|
| Release XBE | `C:\cod\c3_bin\codmp_xboxr.xbe` | loaded in IDA (the single connected instance — no debug build) |
| IDA database | The one IDA instance running `codmp_xboxr.xbe` | decompilation source of truth |
| Linker map | `C:\cod\c3_bin\codmp_xboxr.map` (7.2 MB, 68,753 lines) | **symbol → Lib:Object attribution** |
| PDB | `C:\cod\c3_bin\codmp_xboxr.xbe.pdb` | function names, types, RTTI, source-file paths |
| Complete ida decompiler dump with C code and types | 'c3_bin\codmp_xboxr.xbe.c| C file code snd type dump generated from IDA. Should be used for types and functions unless it does not make sense.
|Demonware Source| `D:\cod_code\Demonware` | Demonware source code. Use for reference to decompile game functions only. 
|NGL Source Code| 'c:\cod_3p' | NGL/NVL/NAL Source Code. Should only be used as a reference to verify against diassembly. 
Derived analysis (to be generated with `tools/*.py` in Phase 0):

| File | Contents |
|---|---|
| `analysis/MANIFEST.tsv` | **THE work list.** Every function: `ea, size, name, lib, obj, class, source_cpp, static, inline` |
| `analysis/WORKLIST.tsv` | Per-source-cpp rollup: function counts + code bytes |
| `analysis/class_hierarchy.tsv` | C++ class → parent → vtable → member functions |
| `analysis/shim_needs.tsv` | Xbox API callees → caller → proposed Win32 replacement |
| `analysis/ida_types.json` | IDA local-type export (structs, enums, typedefs) |

### Build information

| Field | Value |
|---|---|
| Timestamp | `4527c700` (Sun Oct 07 2006 17:55:52 PDT) |
| Preferred load address | `0x00400000` |
| Image base in IDA | `0x10000` (XBE disk address; subtract `0xF0000` for runtime VA) |
| Compiler | MSVC 13.10 (VC7.1, Xbox XDK Dec 2003 / Jan 2005 era) |
| Optimization | `/O2` (release; size + speed) |

### Segments

| # | Name | Start (map) | Length | VA start (base 0x400000) | Class |
|---|---|---|---|---|---|
| 1 | `.textbss` | `0001:00000000` | `0x3fafe1` | `0x401000` | DATA (BSS in code section) |
| 2 | `.text` | `0002:00000000` | `0x63e828` | `0x40c000` | CODE |
| 2 | `.text$x` | `0002:0063e830` | `0x1786e` | `0x000000` | SEH handlers |
| 2 | `.text$yc` | `0002:006560a0` | `0xc579` | `0x000000` | Dynamic initializers |
| 2 | `.text$yd` | `0002:00662620` | `0x2421` | `0x000000` | Dynamic terminators |
| 3 | `XONLINE` | `0003:00000000` | `0x860ae` | `0xa70a60` | Xbox Live library |
| 4 | `XNET` | `0004:00000000` | `0x5b779` | `0xaf6b20` | Xbox networking |
| 5 | `D3D` | `0005:00000000` | `0x3e4b2` | `0xb642a0` | Direct3D |
| 6 | `D3DX` | `0006:00000000` | `0x29c5d` | `0xbcb940` | D3DX utility |
| 7 | `XGRPH` | `0007:00000000` | `0x35a2d` | `0xbfb0a0` | Xbox Graphics |
| 8 | `DSOUND` | `0008:00000000` | `0x28994` | `0xc3cc00` | DirectSound |
| 9 | `MP_LELCS` | `0009:00000000` | `0x1f093` | `0xc8f720` | Level asset LCS |
| 10 | XPP/XID | `000a:*` | various | — | Peripheral drivers |
| 11 | `.rdata` | `000b:00000000` | `0xda4e3` | `0xcd3ce0` | Read-only data |
| 12 | `.data` | `000c:00000000` | `0xcf881` | `0xdd32c0` | Initialized data |
| 12 | `.bss` | `000c:000d1ba0` | `0x2568e8` | — | Zero-initialized data |
| 13 | `.idata` | `000d:*` | — | `0x10fb760` | Import table |
| 14 | `XON_RD` | `000e:00000000` | `0x29a3d` | `0x10fc0a0` | Xbox Online read data |
| 15 | `.data1` | `000f:00000000` | `0x20c` | `0x1125ae0` | Additional data |
| 16 | `DOLBY` | `0010:00000000` | `0x8933` | `0x1125d00` | Dolby encoder |
| 17 | `.XBLD$/tls` | `0011/0012:*` | — | — | Xbox loader / TLS |

**Rule**: Only functions in segments 2 (`.text`) and 9 (`MP_LELCS`) are game/engine code. All other segments are XDK libraries — never reconstruct, shim only.

## 3. Classification (from linker map)

The map file's last column is `Lib:Object`. Symbols without `:` are game objects (no library prefix). Here is the full breakdown:

### Game objects — no lib prefix (reconstruct)

| Map object | Non-inline funcs | Likely source files |
|---|---|---|
| `shell.o` | 1,175 | Main init, console, cmd system, UI bootstrap |
| `scr.o` | 846 | Script integration (`g_scr_main.cpp`, `g_scr_mover.cpp`, `g_scr_vehicle.cpp`) |
| `mp.o` | 770 | Multiplayer game modes, scoring, team logic |
| `g.o` | 732 | Core game logic (`g_client.cpp`, `g_combat.cpp`, `g_active.cpp`, `g_mover.cpp`) |
| `mp_shell.o` | 680 | Multiplayer shell / UI / server browser |
| `game.o` | 663 | Game init + utilities (`g_main.cpp`, `g_items.cpp`, `g_misc.cpp`, `g_utils.cpp`) |
| `core.o` | 541 | Core systems (`cvar.cpp`, command buffer, file I/O) |
| `game2.o` | 521 | Weapons + HUD + spawn (`g_weapon.cpp`, `g_hudelem.cpp`, `g_spawn.cpp`) |
| `render.o` | 426 | Renderer integration layer |
| `anim.o` | 408 | Animation integration layer |
| `mp_actors.o` | 380 | Multiplayer actor/AI behaviors |
| `cg.o` | 350 | Client game (`cg_main`, `cg_draw`, `cg_view`) |
| `cl.o` | 313 | Client engine (`cl_main`, `cl_parse`) |
| `streamer.o` | 268 | Asset streaming / paging |
| `physics.o` | 224 | Physics integration + vehicle collision |
| `game_xbox.o` | 210 | Xbox-specific game platform (paths, memory overrides) |
| `sv.o` | 118 | Server engine (`sv_main`, `sv_world`) |
| `mp_xbox.o` | 61 | Xbox-specific multiplayer |
| `nextgen.o` | 8 | "Next-gen" rendering shims |
| `CallFunctor.o` | 1 | Callable functor utility |
| **Subtotal game** | **~8,735** | |
| `f i` (inline/COMDAT) | ~16,531 | Header methods, templates — **port into class headers** |

### Treyarch engine libraries (reconstruct)

| Library | Key objects | Funcs (est.) | Purpose |
|---|---|---|---|
| `MPBrocCore_xboxd` | `mp_util_wad.o` (523), `mp_anim_wad.o` (332), `Broc.o` (112) | ~970 | Broc entity/script/animation core |
| `mp_level.xboxd` | `mp_level_wad.o` | ~20 | Level asset management |
| `phys_xboxr` | `physics_system.o` (87), `rb_ragdoll_model.o` (31), `phys_constraint_solver_multithreaded.o` (18), `physics_system_internal.o` (14), `rbc_def_*.o`, `phys_gjk.o`, `phys_collision.o`, `phys_contact_manifold.o`, `phys_util.o`, `rigid_body.o` | ~250 | Custom rigid-body + GJK physics |
| `render_xboxr` | `ngl_aux.o` (40), `ShaderCommon.o` (21), `cdOceanGlobals.o` (14), ~20 shader `.o` files | ~150 | Shader / material system |
| `ngl_xboxr` | `ngl_scene.o` (67), `ngl_debug.o` (36), `ngl_font.o` (19), `ngl_mesh.o` (17), `ngl_quad.o` (17), `ngl_*.o` | ~300 | NGL graphics library |
| `nal_xboxr` | `nal_generic.o` (38), `nal_anim.o` (24), `nal_init.o` (11), `nal_skeleton.o` (9), `nal_*.o` | ~120 | Animation engine |
| `nsl_xboxr` | `nslSource.o` (68), `nslWaveBank.o` (48), `nslMaster.o` (16), `nsl*.o` | ~230 | Sound / audio library |
| `nfl_xboxr` | `nfl_system.o` (52), `nfl_driver.o`, `nfl_win32_driver.o`, `nfl_common.o`, `nfl_debug.o`, `nfl_xbox_driver.o` | ~120 | File / streaming library |
| `nvl_xboxr` | `nvl_base.o` (21), `nvl_afmv.o` (15), `nvl_xbox.o` (6) | ~45 | Video decoding library |
| `aeps_xboxr` | `apsSuppliedActions.o` (88), `apsMemory.o` (48), `apsEffect.o` (37), ~20 renderer/node `.o` files | ~350 | Particle / effects system ("APS") |
| `mem_mp_xboxr` | `mem_lib.o` (43), `dlmalloc.o` (15), `ae_heap.o` (6), `mem_lib_platform.o` (11) | ~75 | Memory allocators |
| `core_xboxr` | `PoolAllocator.o` (17), `ae_fixed_string.o` (9), `AeAssert.o` (7), `ae_controller.o` (2), `AeHash.o` (1) | ~36 | Core utility library |
| `tl_xboxr` | `tl_system.o` (23), `tl_instbank.o` (9), `tl_initlist.o` (1) | ~33 | Tech library |
| `cdl_xboxr` | `cdl_common.o` (9), `cdl_gjk.o` (5), `cdl_base.o` (2) | ~16 | Core data library |
| `controller_xboxr` | `controller_xbox.o` (14), `controller.o` (11) | ~25 | Input / controller |
| `jobqueue_xboxr` | `jobqueue.o` (14), `jobqueue_legacy.o` (11) | ~25 | Multithreading job system |
| `peripherals_xboxr` | `XboxMemoryUnit.o` (46), `MemoryUnit.o` (35) | ~81 | Save/load peripherals |
| `apk_xboxr` | `apk.o` (23) | ~23 | Archive / packaging |
| `inplace_xboxr` | `PtrFixupTable.o`, `InplaceTree.o`, `InplaceFileBuilder.o` | ~3 | In-place file loading |
| **bdCore** | `bdString.obj` (28), `bdMemory.obj` (23), `bdInetAddr.obj` (25), `bdBitBuffer.obj` (18), `bdAddr.obj` (18), `bdBytePacker.obj` (8), `bdSocket.obj` (10), `bdSecurityKey-xbox.obj` (6), `bdStopwatch.obj` (7), `bdRandom.obj` (7), `bdTrulyRandom.obj` (3), `bdHMacSHA1.obj` (4), `bdSequenceNumber.obj` (14), `bdBitOperations.obj` (4), `bdMutex.obj` (4), `bdMallocMemory.obj` (5), `bdLog.obj` (9), `bdLogChannel.obj` (6), `bdLogSubscriber.obj` (3), `bdShortTimer.obj` (4), `bdCore.obj` (3), `bdByteBuffer.obj` (3), `bdAlignedOffsetMemory.obj` (3), `bdReferencable.obj` (1), `bdHMac.obj` (1), `bdCommonAddr-xbox.obj` (19) | ~220 | BD networking core |
| **bdConnection** | `bdUnicastConnection.obj` (36), `bdPacket.obj` (9), `bdMessage.obj` (9), `bdReliableSendWindow.obj` (10), `bdReliableReceiveWindow.obj` (6), `bdDataChunk.obj` (12), `bdConnection.obj` (15), `bdConnectionStore.obj` (12), `bdConnectionStatistics.obj` (16), `bdChunk.obj` (7), `bdLoopbackConnection.obj` (8), `bdReceivedMessage.obj` (4), `bdShutdownChunk.obj` (6), `bdShutdownAckChunk.obj` (6), `bdShutdownCompleteChunk.obj` (6), `bdHeartbeatChunk.obj` (6), `bdHeartbeatAckChunk.obj` (6), `bdInitChunk.obj` (9), `bdInitAckChunk.obj` (10), `bdCookie.obj` (10), `bdCookieAckChunk.obj` (6), `bdCookieEchoChunk.obj` (8), `bdSAckChunk.obj` (13), `bdConnectionListener.obj` (5), `bdUnreliableSendWindow.obj` (5), `bdUnreliableReceiveWindow.obj` (5) | ~250 | BD connection protocol |
| **bdNet** | `bdNet-xbox.obj` (14), `bdGameInfo.obj` (13), `bdDiscoveryServer.obj` (9), `bdDiscoveryClient.obj` (8), `bdDiscoveryListener.obj` (5), `bdGameInfoFactory.obj` (5), `bdDispatcher.obj` (4), `bdDispatchInterceptor.obj` (2) | ~60 | BD network discovery |
| **bdPeer** | `bdSession.obj` (44), `bdSessionListener.obj` (10), `bdSessionInfo.obj` (3), `bdSessionHandler.obj` (3), `bdSessionInterceptor.obj` (2) | ~62 | BD peer-to-peer sessions |
| **bdSocket** | `bdQoSProbe-xbox.obj` (11), `bdAddressMap-xbox.obj` (7), `bdSocketRouter-xbox.obj` (10), `bdQoSProbeListener.obj` (2) | ~30 | BD socket layer |
| **bdPlatform** | `bdPlatformSocket-win32.obj` (8), `bdPlatformSocket.obj` (5), `bdPlatformMutex-xbox.obj` (4), `bdPlatformTiming-xbox.obj` (5), `bdPlatformTrulyRandom-xbox.obj` (1), `bdInAddr.obj` (5) | ~28 | BD cross-platform abstractions |
| `zlib_xboxr` | `inflate.o` (11), `deflate.o` (12), `unzip.o` (18), `trees.o` (5), `zutil.o` (5), `crc32.o` (3), `compress.o` (3), `adler32.o` (2), `uncompr.o` (1), `inftrees.o` (1), `inffast.o` (1) | ~62 | Zlib (use modern zlib instead) |
| **Subtotal engine libs** | | **~2,610** | |

### XDK / Microsoft libraries (NEVER reconstruct — shim)

| Library | Key objects | Funcs (est.) | Win32 replacement |
|---|---|---|---|
| `dsoundd` | `dsapi.obj` (256), `dscommon.obj` (87), `mcpbuf.obj` (34), `mcpapu.obj` (32), `mcpvoice.obj` (31), `mcpxcore.obj` (19), `mcpstrm.obj` (27), `ac97.obj` (32), `hrtf.obj` (21), `i3dl2.obj` (15), `wavexmo.obj` (16), `dsmath.obj` (11), `gpdsp.obj` (13), `epdsp.obj` (12), `ac97xmo.obj` (11), `heap.obj` (17), `cipher.obj` (6), `dspdma.obj` (6), `dsperf.obj` (5) | ~650 | XAudio2 |
| `d3d8d` | `precomp.obj` (480), `stats.obj` (467), `d3dbase.obj` (87), `state.obj` (52), `pusher.obj` (51), `timing.obj` (46), `recorder.obj` (41), `shadersnapshot.obj` (39), `vshader.obj` (36), `pushres.obj` (36), `resource.obj` (18), `pixeljar.obj` (14), `drawprim.obj` (14), `rdi.obj` (13), `texture.obj` (11), `surface.obj` (11), `mphal.obj` (22), `mpcore.obj` (22), `mpintr.obj` (15), `mpmode.obj` (4), `mpdac.obj` (2), `math.obj` (8), `buffer.obj` (8), `block.obj` (10), `lazy.obj` (10), `enum.obj` (10), `memory.obj` (9), `dxgcreate.obj` (8), `present.obj` (4), `cleari.obj` (1), `combiner.obj` (1), `floatmath.obj` (4), `debug.obj` (4), `pshader.obj` (19), `d3ddev.obj` (2) | ~1,500 | D3D9 backend |
| `xonlinesd` | `xonline.obj` (165), `ip.obj` (102), `messaging.obj` (89), `logon.obj` (61), `presence.obj` (85), `ipcfg.obj` (36), `base.obj` (58), `sock.obj` (62), `presmsg.obj` (64), `xnet.obj` (84), `kerberos.obj` (41), `msasn1.obj` (158), `storul.obj` (44), `stats.obj` (33), `upnpnat.obj` (19), `socktcp.obj` (47), `localcache.obj` (85), `accounts.obj` (41), `contdl.obj` (40), `stordl.obj` (9), `download.obj` (26), `upload.obj` (27), `titlecache.obj` (15), `match.obj` (21), `ipdns.obj` (16), `ipqos.obj` (33), `nicx.obj` (29), `billing.obj` (28), `users.obj` (13), `contenum.obj` (14), `contutil.obj` (13), `memutil.obj` (13), `baseio.obj` (18), `sockudp.obj` (7), `dvdload.obj` (7), `dirops.obj` (23), `xrlutil.obj` (22), `xcontent.obj` (22), `xrltask.obj` (9), `reboot.obj` (9), `pathname.obj` (9), `enet.obj` (19), `xbosutil.obj` (2), `advnotif.obj` (15), `troubleshoot.obj` (6), `feedback.obj` (6), `string.obj` (12), `livesign.obj` (8), `halx.obj` (8), `xontask.obj` (4), `cfcache.obj` (5), `storutil.obj` (19), `zipcode.obj` (2), `maketbl.obj` (2), `krb5.obj` (2), `ipicmp.obj` (2), `symmdec.obj` (4), `decin.obj` (4), `decxlat.obj` (2), `decuncmp.obj` (2), `dectree.obj` (2), `decout.obj` (1), `decblk.obj` (1), `decalvb.obj` (1), `md5.obj` (3), `xonlzx.obj` (8), `tcpipxsum.obj` (1), `service.obj` (1) | ~1,800 | Stub or Steamworks |
| `xgraphicsd` | `interp.obj` (190), `api.obj` (109), `instructiongraph.obj` (103), `fsglue.obj` (81), `scale.obj` (63), `fontmath.obj` (41), `sfntaccs.obj` (40), `xfont.obj` (35), `scanlist.obj` (34), `fscaler.obj` (34), `pshdrval.obj` (31), `vshdrval.obj` (25), `swizzler.obj` (13), `sbit.obj` (26), `cd3dxassembler.obj` (18), `pixelshaderverifier.obj` (22), `valbase.obj` (20), `pixelshader.obj` (20), `scendpt.obj` (12), `scentry.obj` (10), `header.obj` (11), `scbitmap.obj` (7), `cd3dxstack.obj` (10), `scmemory.obj` (3), `truetype.obj` (3), `scspline.obj` (4), `scline.obj` (4), `debug.obj` (4), `pixelshaderoptimizer.obj` (2), `bitmap.obj` (2), `surfacetofile.obj` (1), `subpixel.obj` (1), `preprocessor.obj` (1), `pixelshaderopt.obj` (1), `painttext.obj` (1) | ~1,050 | D3D9 texture utils |
| `xvoiced` | `xhvengine.obj` (66), `localtalker.obj` (50), `wmsauxcode.obj` (49), `voicechatmode.obj` (34), `remotetalker.obj` (26), `wmavoicedec.obj` (7), `wms*.obj` (~120), `ps*.obj` (~40), `dec*.obj` (~20), `enc*.obj` (~15), various codec `.obj` files | ~450 | NOP for now |
| `d3dx8d` | `d3dxmath.obj` (69), `cd3dxcodec.obj` (62), `cd3dxblt.obj` (29), `d3dx8tex.obj` (26), `png.obj` (15), `pngrtran.obj` (21), `pngrutil.obj` (18), `pngread.obj` (12), `pngget.obj` (9), `pngset.obj` (7), `pngrio.obj` (2), `pngtrans.obj` (6), `pngmem.obj` (6), `pngerror.obj` (6), `cd3dxrendertoenvmap.obj` (20), `cd3dxrendertosurface.obj` (10), `cd3dximage.obj` (10), `cd3dxsprite.obj` (11), `cd3dxfile.obj` (4), `d3dx8core.obj` (6), `d3dx8dbg.obj` (4), `s3tc.obj` (7), `s3tchelp.obj` (2), JPEG `.obj` (~30) | ~380 | D3DX11 / stb_image |
| `xapilibd` | `heap.obj` (36), `tree.obj` (30), `xid.obj` (27), `usbdev.obj` (25), `hub.obj` (25), `disk.obj` (24), `thread.obj` (22), `synch.obj` (22), `contsig.obj` (22), `xapiheap.obj` (20), `ohcd.obj` (17), `filehops.obj` (17), `xcalcsig.obj` (15), `pathmisc.obj` (13), `datetime.obj` (13), `xsaveapi.obj` (11), `mrb.obj` (11), `filemisc.obj` (11), `bootutil.obj` (11), `transfer.obj` (10), `xidinp.obj` (9), `virtual.obj` (9), `isr.obj` (12), `xemodule.obj` (6), `roothub.obj` (6), `physical.obj` (6), `mountmu.obj` (6), `lcompata.obj` (6), `lcompat.obj` (6), `xmem.obj` (3), `xapiterm.obj` (3), `fileopcr.obj` (3), `filefind.obj` (3), `xpp.obj` (5), `usbd.obj` (5), `process.obj` (5), `pool.obj` (1), `widechar.obj` (4), `usbinit.obj` (4), `tls.obj` (4), `powerdwn.obj` (4), `launch.obj` (4), `xapi0dat.obj` (2), `xapi0.obj` (2), `usbmem.obj` (2), `typeinfo.obj` (2), `support.obj` (2), `perfctr.obj` (2), `krnlptch.obj` (2), `handle.obj` (2), `format.obj` (2), `dir.obj` (2), `devsys.obj` (2), `cancelio.obj` (2), `xclndrv.obj` (2), `xapiinit.obj` (7), `schedule.obj` (7), `kthunks.obj` (7), `isoch.obj` (7), `error.obj` (7), `xqueryvalue.obj` (1), `xget*.obj` (~6), `outputdebugstring*.obj` (2), `mu.obj` (8), `muldiv.obj` (1), `delaybnd.obj` (1), `compstr*.obj` (2), `xapidlla.obj` (1), `bitmapa.obj` (1) | ~500 | Win32 equivalents |
| `uixd` | `engine.obj` (126), `friends.obj` (62), `uixlogon.obj` (57), `players.obj` (47), `plugin.obj` (22), `skin.obj` (16), `uiobj.obj` (17), `voicemail.obj` (27), `memory.obj` (8), `input.obj` (8), `util.obj` (5), `devstate.obj` (3) | ~398 | stub (COD3 has own FE* UI) |
| `dmusicd` | `xguids.obj` (217) | ~217 | stub |
| `xboxkrnl` | `xboxkrnl.exe` (167) | ~167 | win32_shim |
| `xbdm` | `xbdm.dll` (1) | ~1 | ignore |
| `uuid` | — | ~4 | use `UuidCreate` |
| **CRT** | `LIBCMTD:*` (~300), `LIBCMT:*` (~3), `libcpmtd:*` (~2) | ~305 | modern MSVC CRT |
| **Subtotal XDK** | | **~7,460** | |

### Grand totals

| Category | Non-inline funcs | Inline funcs | Policy |
|---|---|---|---|
| Game objects | ~8,735 | ~16,531 | **reconstruct** |
| Treyarch engine libs | ~2,610 | included in game inline count | **reconstruct** |
| XDK / Microsoft | ~7,460 | — | **NEVER reconstruct** — shim |
| Linker thunks | ~735 (`j_` prefix) | — | ignore |
| **Total** | **~19,540** | **~16,531** | |

## 4. Original codebase layout

Derived from 434 embedded source path strings, PDB module info, and assert-string cross-references:

```yaml
c:\cod\code\                          <- game + tech library root
├── game\                              <- game module (id Tech 3 heritage)
│   ├── g_active.cpp                   actor processing
│   ├── g_actor_prone.cpp              prone behavior
│   ├── g_client.cpp                   client-side game logic
│   ├── g_combat.cpp                   damage / combat system
│   ├── g_hudelem.cpp                  HUD elements
│   ├── g_items.cpp                    items / pickups
│   ├── g_main.cpp                     game init + main loop
│   ├── g_misc.cpp                     misc systems
│   ├── g_mover.cpp                    moving entities
│   ├── g_spawn.cpp                    entity spawning
│   ├── g_syscalls.cpp                 VM system-call boundary
│   ├── g_trigger.cpp                  trigger system
│   ├── g_weapon.cpp                   weapons
│   ├── g_utils.cpp                    utilities
│   ├── g_scr_main.cpp                 script integration
│   ├── g_scr_mover.cpp                scripted movers
│   ├── g_scr_vehicle.cpp              scripted vehicles
│   ├── g_vehicle_path.cpp             vehicle pathfinding
│   ├── bg_pmove.cpp                   player movement (shared)
│   ├── cvar.cpp                       console variables
│   ├── PakFile.cpp                    asset packaging
│   ├── DecodePakFile.cpp              asset decompression
│   ├── util_str.cpp                   string utilities
│   ├── vehicle_collision.cpp          vehicle collision bridge
│   ├── EntityManager.h                entity lifecycle
│   ├── sentient.h                     AI sentient base
│   ├── actor.h                        actor / NPC
│   ├── InteractionController.h        use / interact system
│   ├── RumbleEffect.h / RumbleManager.h  controller rumble
│   ├── PathNodeMgr.h                  path node manager
│   ├── FreeList.h                     free-list allocator
│   ├── gamepause.h                    pause system
│   └── g_local.h                      game-local types
│
├── tl\                                <- Tech Library
│   ├── physics\                       <- custom physics engine
│   │   ├── include\*.h
│   │   └── source\*.cpp               (filenames inferred from obj names)
│   └── cdl\                           <- Core Data Library
│       └── source\cdl_mem.h
│
└── bd\                                <- Bandwidth/Discovery networking
    ├── bdContainers\                  <- bdBitBuffer, serialization
    ├── bdSocket\                      <- Socket abstraction
    ├── bdWindow\                      <- Reliable send/receive windows
    ├── bdMemory\                      <- Custom memory allocators
    └── bdUtilities\                   <- Bit operations, crypto
```

Additional Treyarch library code exists for `ngl_*`, `nal_*`, `nsl_*`, `nfl_*`, `nvl_*`, `aeps_*`, `core_*`, `tl_*`, `cdl_*`, `controller_*`, `jobqueue_*`, `mem_mp_*`, `apk_*`, `inplace_*`, and `peripherals_*` — exact source paths to be recovered from the PDB in Phase 0.

## 5. Target repo layout

```css
cod3_decomp/
  DECOMP_PLAN.md          (this file)
  PROGRESS.tsv            (per-file status tracker — §10)
  analysis/               (generated data, regeneratable)
  tools/                  (IDA MCP scripts, generators, analyzers)
  CMakeLists.txt
  src/
    game/                  ~8,735 funcs — g_client.cpp, g_combat.cpp, ...
    broc/                  ~970 funcs — MPBrocCore_xboxd (BrocSys core)
    physics/               ~250 funcs — phys_xboxr
    ngl/                   ~300 funcs — ngl_xboxr
    render/                ~150 funcs — render_xboxr
    animation/             ~120 funcs — nal_xboxr
    sound/                 ~230 funcs — nsl_xboxr
    filesystem/            ~120 funcs — nfl_xboxr (+ apk_xboxr + inplace_xboxr)
    nvl/                   ~45 funcs — nvl_xboxr
    aeps/                  ~350 funcs — aeps_xboxr
    bd/                    ~650 funcs — bdCore + bdConnection + bdNet + bdPeer + bdSocket + bdPlatform
    core/                  ~170 funcs — core_xboxr + tl_xboxr + cdl_xboxr + mem_mp_xboxr
    input/                 ~25 funcs — controller_xboxr
    threading/             ~25 funcs — jobqueue_xboxr
    platform_xbox/         ~350 funcs — game_xbox.o + mp_xbox.o + peripherals_xboxr (shim layer)
  platform/
    win32/                 Win32 entry point, D3D9 backend, XAudio2, WinSock
    xbox_shim/             Thin wrappers for all XDK API calls
  thirdparty/
    zlib/                  modern zlib (replaces zlib_xboxr)
```

`MANIFEST.tsv`'s `source_cpp` column names the destination file for every function. Headers live next to their cpp files in the same directory. Cross-lib includes match the original layout (inferred from types used + IDA include paths).

## 6. Build strategy

- **CMake + MSVC 2022, 32-bit (x86) only.** The code assumes 4-byte pointers throughout. No x64.
- **C++14 maximum.** Write in the 2005-era idiom: no `auto`, no lambdas, no range-for, no `nullptr` (use `NULL`/`0`), no `override`/`final`, no braced-init. This keeps a future exact-match VC7.1 build possible.
- One static lib target per top-level `src/` directory, one exe target `cod3mp`.
- **`/fp:precise`**, default struct packing unless IDA size disagrees → `#pragma pack(push, N)` + `static_assert(sizeof(X) == N)`.
- **Every ported struct gets `static_assert(sizeof(X) == N)`** against IDA. Hot structs (`gentity_s`, `gclient_s`, `playerState_s`, `usercmd_s`, `pmove_t`, `rigid_body`, any `phys_*`) also get `static_assert(offsetof(X, member) == N)`.
- Win32 build must compile **zero errors** at every commit. Unported callees are satisfied by generated stubs (§6.1).

### 6.1 Stub scaffolding (Phase 1 deliverable)

Generate, from MANIFEST, one `stubs_<lib>.cpp` per target library containing every not-yet-ported function as a stub:

```cpp
#define COD3_UNIMPLEMENTED(msg) do { \
    OutputDebugStringA("COD3 UNIMPLEMENTED: " msg); \
    __debugbreak(); \
    return {}; \
} while(0)
```

As functions are ported, they move from the stub cpp to their real cpp. **The link is green from day one**; progress = shrinking stub files.

## 7. The reconstruction protocol (per file — THIS IS THE CORE LOOP)

The unit of work is **one target cpp file** (one row of WORKLIST.tsv). Never work on scattered individual functions.

1. **Select** the next file per the phase order (§11) from `PROGRESS.tsv` with status `TODO`. Set it `IN_PROGRESS`.
2. **Confirm IDA routing**: the project uses a **single** IDA instance running the release `codmp_xboxr.xbe`. Re-verify the connection periodically (routing can flip back silently) before batch decompilation. There is no debug build.
3. **List the file's functions**: filter MANIFEST rows by `source_cpp`. Sort by `ea`. This ordering approximates original source order — keep it in the cpp.
4. **For each function** (skip `j_` thunks entirely; inline/COMDAT rows `f i` are ported into *headers* per §7.6, not one-by-one into the cpp):
   a. `decompile(ea)`. Evaluate the output for optimization artifacts:
   - **Inlined callees**: recognize the pattern; reconstruct as a separate inline helper in the header
   - **Register reuse**: split when a variable holds two logically distinct values
   - **Loop transformations**: unrolled loops → reconstruct as `for`, induction-variable changes → restore
   - **Strength reduction**: reverse `(x * 0xCCCCCCCD) >> 3` → `x / 5`
   - **Tail-call optimization**: restore as `return sub_call(...)`
   b. If Hex-Rays output is ambiguous, get `disasm` and cross-check. Disassembly is authoritative.
   c. Reconstruct the **original C++ intent**, not the assembly transcription:
   - Use IDA's named locals (from PDB symbols) wherever available — they are ground truth
   - When IDA shows `vNN`, derive a meaningful name from usage context
   - Keep original parameter names (IDA shows demangled names with param names from PDB)
   - Re-derive loop structures, if/else chains, and switch statements from the control flow
   d. Demangle the name for the C++ signature. Statics (`static` column) are file-local — declare `static` in the cpp.
   e. `$E`-named statics (compiler-generated dynamic-initializer/atexit fragments): do NOT port as functions. Reconstruct the file-scope global object + its constructor instead.
5. **Types**: pull struct/class layouts from IDA local types (the PDB populated them). Define each type once in the correct header. **Never guess a field** — if IDA's type information is incomplete, use `char pad_XX[N]` with a TODO comment.
6. **Templates/COMDATs** (`inline=i` rows / `f i` flag):
   - `std::` instantiations (Dinkumware VC7) → use modern `std::` equivalents. Never transcribe Dinkumware internals. **Exception**: if a struct embeds a `std::` type by value, modern `std::` won't match the old layout — assert only offsets of members *before* the std member, add `// LAYOUT-DIVERGES(std)` comment, never hand-pad.
   - Game-engine / Treyarch templates (e.g. `PoolAllocator<T>`, `phys_memory_pool<T>`, `ae_fixed_string<N>`, `DbLinkedHandle<...>`, `bdReference<T>`) → implement the template ONCE in its header from a representative instantiation, then all other instantiations are free.
   - Header inline methods → implement in the class declaration in the header. **~16,500 functions are header code** — they are real logic, just COMDAT-emitted.
7. **Xbox API calls** (D3D/XTL/DSOUND/XNet/XOnline/XGRPH/XAPILIB/XVOICE — any function from XDK segments): call it as-is; declare it in `platform/xbox_shim/*.h`. Do NOT implement the shim while porting game code — record needed shims in `platform/xbox_shim/NEEDED.md`.
8. **Compile gate**: the file (and whole solution) must build cleanly. Remove ported functions from the stub cpp. 32-bit MSVC builds occasionally hang — use a 5-min timeout, kill `cl.exe` + `mspdbsrv.exe`, retry once.
9. **Mark done**: set `PROGRESS.tsv` status `PORTED`, note function count. Commit with message `[<lib>] port <source_cpp> (<N> funcs)`.

### 7.1 Hard rules (violations have burned us before)

- **This is a RELEASE build.** There are no `/RTC1` artifacts to strip (no `0xCCCCCCCC` fills, no `_RTC_*` calls). There ARE optimization artifacts to reverse (see §7.4a).
- **Use the PDB's original parameter names.** Hex-Rays shows demangled names with parameter names from the PDB (e.g. `worldOrigin`, `hitLocation`). Keep them exactly.
- **NEVER invent code.** If a callee/type/global is unknown, stub it and log it in a file-header comment. Only code derived FROM IDA goes in.
- **NEVER "simplify" or "clean up"** decompiled logic. Byte-wrap casts, redundant-looking clamps, float-to-int bit casts (`LODWORD(f)`) are load-bearing. These are the release build's equivalent of "the compiler kept this for a reason."
- **Disassembly beats Hex-Rays** when they disagree.
- Consecutive float locals passed as `&v12` are usually a `float[3]` — check disasm before declaring three scalars.
- One Hex-Rays variable can hold two source locals (slot reuse across a destructive call) — split when the second half's value is dead-read.
- Keep an `// ea: 0x004XXXXX` comment above every reconstructed function so reviewers can re-diff against IDA. Use runtime VA (map address + 0x400000).
- **Assert strings are ground truth.** Release builds kept `__FILE__` + line number strings. Reproduce assert condition strings **byte-exactly** — they verify you're in the right file, and line numbers let you sanity-check function ordering.

### 7.2 Escalate (stop and ask the supervising session) when:

- Hex-Rays output is garbage / function is entirely inlined into nothing
- A function is `__asm`-heavy (SSE/MMX intrinsics, GPU push-buffer writes)
- SEH-heavy functions (`.text$x` handlers)
- A struct layout conflicts between two IDA sources
- A file needs >5 unknown types defined at once (probably wrong porting order)
- D3D push-buffer encoding functions (NV097 register writes)

## 8. Platform strategy (Win32 only — no Xbox build path preserved)

Unlike some sibling projects, we are **not preserving an Xbox build path**. Target is Win32 only. Game code calls Xbox APIs verbatim; the shim owns all divergence.

| Island | Xbox API | Win32 replacement |
|---|---|---|
| Graphics | Xbox D3D8.1 (push buffers, NV097 GPU registers, XGRPH swizzled textures) | Custom D3D9 backend behind NGL API; initially a **null renderer** so the game loop runs headless |
| Audio | DSOUND Xbox flavor + Dolby encoder (`DOLBY` segment) | XAudio2; Dolby → multichannel PCM |
| Files | `D:\` / `T:\` / `U:\` / `Z:\` paths, Nt* kernel file I/O | Path-translation shim to `gamedata/`, Win32 `CreateFileW` |
| Input | Xbox controller (XInput legacy XDK flavor) + XID drivers | Modern XInput + DirectInput for keyboard/mouse |
| Networking | XNet/XOnline (`XONLINE` + `XNET` segments) + `bd*` library | Replace XNet/XOnline with ENet or Steamworks; `bd*` protocol layer kept intact |
| Memory | `XMemAlloc`/`XMemFree`, `MmAllocateContiguousMemory` (physical), `ExAllocatePoolWithTag` | `VirtualAlloc` / `malloc` wrappers in `xbox_shim` |
| CRT | LIBCMT / LIBCMTD / libcpmtd (Dec03 XDK) | Modern MSVC CRT — **do not port** |
| Video | Bink (linked separately, not statically in this XBE) | Bink SDK for Windows (if video playback needed) |
| Voice | XVoice codec + XHV engine | NOP / stub for initial release |
| Kernel | `Xtl.h`, `Ke*` synchronization, `Nt*` I/O, `Hal*` hardware | Thin wrappers in `xbox_shim/` |

Rule: game code calls Xbox APIs **verbatim**; the shim layer owns all divergence. Keep `#ifdef COD3_XBOX` usage inside `platform/` only. No `#ifdef` in game code.

### Memory notes

`XMemAlloc`/`XMemSize`/`XMemFree` are overridden in `game_xbox.o` (likely in `IkeMain.obj` or equivalent entry point). Port them onto `malloc`/`_msize`/`free`. The `mem_mp_xboxr:dlmalloc.o` (15 funcs) is Doug Lea's malloc — this is already a cross-platform allocator; keep it or replace with system allocator.

## 9. Physics engine (phys_xboxr, ~250 functions)

The binary links a **custom rigid-body physics engine** — NOT Havok. It's Treyarch's own system built on GJK collision detection (`phys_gjk.o`, `cdl_gjk.o`). Key components:

- **Rigid body**: `rigid_body`, `rb_ragdoll_model`, `rb_vehicle`
- **Constraints**: `rbc_def_generic`, `rbc_def_ragdoll`, `rbc_def_vehicle`, `rbc_def_contact`, `rbc_def_custom`, `phys_constraint_solver_multithreaded`
- **Collision**: `phys_gjk.o`, `phys_collision.o`, `phys_contact_manifold.o`
- **System**: `physics_system`, `physics_system_internal`
- **Math**: `phys_math.h`, `phys_vec3`

This is a **bit-faithful** subsystem — rounding errors in physics propagate. Reconstruct precisely.

## 10. Progress tracking

`PROGRESS.tsv` columns: `source_cpp | lib | class | funcs | status | notes`. Statuses:
- `TODO` — not started
- `IN_PROGRESS` — actively being reconstructed
- `PORTED` — code written, compiles, stubs removed
- `VERIFIED` — reviewed against IDA by a stronger session, or exercised at runtime

Seed from WORKLIST.tsv. One file = one row = one commit. The supervising session audits `PORTED → VERIFIED`.

## 11. Phase order

### Phase 0 — repo skeleton + analysis generation (supervised, read-only IDA work)

**Deliverables:**
1. Initialize repo at `cod3_decomp/`, git, `.gitignore`.
2. Parse the 68,753-line map file → `MANIFEST.tsv` with columns: `ea`, `size`, `name`, `lib`, `obj`, `flags`, `class`.
3. Classify every symbol: run name-pattern + lib/obj rules → assign `class` = `game` / `engine` / `xdk` / `crt`.
4. Cross-reference MANIFEST with IDA functions (`func_profile` or `func_query` for ea matching).
5. Resolve `source_cpp` per function: (a) PDB source-file query via IDA Python, (b) assert-string xrefs for `__FILE__`, (c) name prefix heuristics for fallback.
6. Generate `WORKLIST.tsv` (rollup per `source_cpp`).
7. Generate `FOLDER_TREE.md` from resolved source paths.
8. Write CMake skeleton: one `add_library(... STATIC)` per top-level `src/` dir, one `add_executable(cod3mp)`.
9. Generate stubs for ALL functions → one `stubs_<lib>.cpp` per library.
10. Write the ~50 most-called XDK shim declarations in `platform/xbox_shim/`.
11. Seed `PROGRESS.tsv` from WORKLIST.tsv with all status `TODO`.
12. Initial green-link build.

**Exit gate**: `cmake --build .` produces `cod3mp.exe` that prints `"cod3mp: UNIMPLEMENTED at main"` and exits.

### Phase 1 — type foundation (supervised)

**Deliverables:** Core headers from IDA local types, every struct size-asserted.

1. **Math types**: `math::Dir3`, `math::Mat43`, `math::Position3`, `math::TranMat43`, `math::Color`
2. **Physics types**: `phys_vec3`, `rigid_body`, `rigid_body_constraint_*`, GJK cache types, collision manifold types
3. **Game core types**: `gentity_s`, `gclient_s`, `playerState_s`, `usercmd_s`, `pmove_t`, `trace_t`, `trajectory_t`
4. **Engine types**: `Broc::string`, `Broc::entity`, `HashString`, `InplaceString`, `Cvar` struct
5. **Network types**: `bdBitBuffer`, `bdPacket`, `bdReference<T>`, `bdConnection`, `bdMessage`, `bdDataChunk`
6. **Memory types**: `mem_heap`, `ae_heap_wrapper`, `PoolAllocator<T>`, `FreeList<T>`
7. **UI types**: `FEMenu`, `FEText`, `PanelFile` types

### Phase 2 — leaf subsystems (gruntwork begins)

Port smallest-first by code bytes within each group:

1. `core_xboxr` (36 funcs) — PoolAllocator, ae_fixed_string, AeAssert, ae_controller, AeHash
2. `cdl_xboxr` (16) — cdl_common, cdl_gjk, cdl_base
3. `mem_mp_xboxr` (75) — mem_lib, dlmalloc, ae_heap, mem_lib_platform
4. `tl_xboxr` (33) — tl_system, tl_instbank, tl_initlist
5. `zlib_xboxr` (62) — **skip**: use modern zlib linked as thirdparty
6. `inplace_xboxr` (3) — In-place file builder
7. `jobqueue_xboxr` (25) — jobqueue, jobqueue_legacy
8. `controller_xboxr` (25) — controller
9. `apk_xboxr` (23) — archive packaging

### Phase 3 — bd networking core (no platform deps)

1. `bdCore` (220) — bdString, bdMemory, bdInetAddr, bdBitBuffer, bdAddr, bdBytePacker, bdSocket, bdSecurityKey, bdStopwatch, bdRandom, bdTrulyRandom, bdHMacSHA1, bdSequenceNumber, bdBitOperations, bdMutex, bdLog*, bdShortTimer, bdByteBuffer, bdAlignedOffsetMemory, bdReferencable, bdCommonAddr
2. `bdPlatform` (28) — bdPlatformSocket-win32 (already Win32!), bdPlatformMutex, bdPlatformTiming, bdPlatformTrulyRandom, bdInAddr
3. `bdConnection` (250) — All chunks, windows, connection types
4. `bdSocket` (30) — QoS probe, address map, socket router
5. `bdNet` (60) — Discovery, game info
6. `bdPeer` (62) — Sessions, listeners, handlers

### Phase 4 — engine libraries

1. `nfl_xboxr` (120) — File/streaming library
2. `nvl_xboxr` (45) — Video decoding
3. `nal_xboxr` (120) — Animation engine
4. `nsl_xboxr` (230) — Sound library
5. `aeps_xboxr` (350) — Particle/effects system
6. `phys_xboxr` (250) — Physics engine (bit-faithful)
7. `MPBrocCore_xboxd` (970) — Broc entity/script/animation core

### Phase 5 — game logic (largest phase)

Port smallest-first by code bytes within each game object:

1. `core.o` (541) — cvar, cmd, file I/O, console
2. `cl.o` (313) — client engine
3. `sv.o` (118) — server engine
4. `cg.o` (350) — client game
5. `g.o` (732) — g_client, g_combat, g_active, g_mover
6. `game.o` (663) — g_main, g_items, g_misc, g_utils
7. `game2.o` (521) — g_weapon, g_hudelem, g_spawn
8. `scr.o` (846) — g_scr_main, g_scr_mover, g_scr_vehicle
9. `physics.o` (224) — physics integration, vehicle collision
10. `anim.o` (408) — animation integration
11. `render.o` (426) — renderer integration
12. `streamer.o` (268) — asset streaming
13. `mp.o` (770) — multiplayer game modes
14. `mp_actors.o` (380) — multiplayer actors
15. `mp_shell.o` (680) — multiplayer shell/UI
16. `shell.o` (1175) — main init, console, UI bootstrap
17. `game_xbox.o` (210) — Xbox-specific game (port onto Win32 shim)
18. `mp_xbox.o` (61) — Xbox-specific multiplayer (port onto Win32 shim)
19. `mp_level.xboxd` (20) — level loading
20. `peripherals_xboxr` (81) — memory unit save/load (Win32 → file system)
21. `nextgen.o` (8) — next-gen preview stubs

### Phase 6 — rendering + audio backends

1. NGL D3D9 backend — implement `ngl_*` shim on D3D9
2. Shader system — HLSL shaders replacing compiled Xbox shaders (`render_xboxr:cd*Shader.o`)
3. Font rendering — `ngl_font` on D3D9
4. Audio backend — XAudio2 implementing `nsl*` shim
5. Video playback — Bink SDK (if needed; may be cutscene-only)

### Phase 7 — networking integration

1. bdNet + bdPeer runtime on WinSock
2. Game networking — server/client state sync, entity replication
3. Match session — game info, discovery, joining
4. XONLINE/XNET stubs → "offline only" or Steamworks backend

### Phase 8 — game main + integration

1. `g_main.cpp` — game init, `Com_Init`
2. `Com_Frame` — main loop wired to D3D9 + XAudio2 + input
3. **First frame** — null renderer → D3D9 → visible output
4. **First game** — local listen server, player movement, shooting
5. **First network** — two-process client/server communication

### Phase 9 — polish + verification

- Runtime behavior comparison against Xbox reference (test suite / capture replay)
- Performance profiling (733MHz Xbox → modern CPU, factor 50x headroom)
- Widescreen, >60fps, FoV slider, input rebinding
- Dedicated server binary
- Remaining shim implementations
- Full `PORTED → VERIFIED` audit pass

## 12. Tooling (to be written in Phase 0)

```graphql
tools/
├── parse_map.py              # Parse codmp_xboxr.map → MANIFEST.tsv
├── classify.py               # Classify each function (game/engine/xdk/crt)
├── resolve_sources.py        # Resolve per-function source_cpp from PDB + asserts
├── generate_stubs.py         # Generate stubs_<lib>.cpp from MANIFEST
├── export_types.py           # Dump IDA local types → C++ headers + ida_types.json
├── export_classes.py         # RTTI vtable analysis → class_hierarchy.tsv
├── generate_progress.py      # WORKLIST.tsv → PROGRESS.tsv seed
├── build_shim_needs.py       # Xbox-import callee scan → shim_needs.tsv
├── verify_build.py           # Run CMake build, report errors per file
└── stub_watch.py             # Monitor stubs_*.cpp for functions ready to move
```

### Key IDA MCP calls used by tools

| Script | IDA MCP calls |
|---|---|
| `parse_map.py` | No IDA calls — pure map-file parsing |
| `classify.py` | `entity_query` for strings, `find` for xrefs, pattern-matching on lib/obj |
| `resolve_sources.py` | `py_eval` for PDB source queries, `find(type='string')` for assert-string xrefs |
| `export_types.py` | `type_query(count=0, include_members=true)`, `type_inspect` |
| `export_classes.py` | `py_eval` for RTTI/vtable traversal |
| `build_shim_needs.py` | `callees(addrs=..., limit=500)` on game functions, filter for XDK segments |
| Per-function decompilation | `decompile(addr)` + `disasm(addr)` when ambiguous |

## 13. Key differences from other kisak projects

| Aspect | GR2 (Stonewall) | COD3 (this project) |
|---|---|---|
| Build type | Debug (`/Od`, `/RTC1`) | Release (`/O2`) |
| Optimization artifacts | None (verbatim transcription) | Heavily optimized (reconstruction required) |
| Function count | ~46,000 | ~74,958 (IDA) / ~34,661 (map publics) |
| File attribution | PDB module table + linker map with clean .cpp names | Map `Lib:Object` (game objects are amalgamated `.o` files — need PDB for per-.cpp mapping) |
| Physics | Havok 2.3.0 (7,210 funcs, SDK leak available) | Custom Treyarch physics (~250 funcs, must reconstruct bit-faithfully) |
| Renderer | RS Renderer + D3D8 wrapper | Custom NGL + Xbox D3D8 push buffers + render_xboxr shader system |
| Networking | Lighthouse (958 funcs) | bd* library (~650 funcs) + XONLINE/XNET (stub) |
| Script engine | No VM — direct C++ | BrocSys scripting (likely an interpreter — verify in Phase 0) |
| Asset format | Custom `.big` archives | PakFile (compressed, decoded by `DecodePakFile.cpp`) |
| Xbox path preservation | Yes (dual Win32 + Xbox build) | No (Win32 only) |
| Code style | "Ike"/"Stonewall" RS naming | id Tech 3 naming (g_*, sv_*, cl_*, cg_*) + Treyarch `Broc::` namespacing |
| Compilation model | One .cpp → one .o | Amalgamated .o files (multiple .cpp per .o) |

## 14. Open questions (to resolve in Phase 0)

1. **Per-function `.cpp` attribution**: Game objects are amalgamated (e.g. `g.o` contains functions from `g_client.cpp`, `g_combat.cpp`, `g_active.cpp`, `g_mover.cpp`). How precisely can we map each function to its original `.cpp`? The PDB should have this — verify via `py_eval` on a sample of functions.

2. **VM boundary**: Is `g_syscalls.cpp` a real VM trap boundary (like Q3's `trap_*` / QVM), or is the cgame module directly linked into the same address space? This determines whether we need a VM emulation layer or can just call functions directly.

3. **BrocSys script engine**: Is it a bytecode interpreter (like QuakeC compiled to QVM), a JIT, or just a C++ class hierarchy? The presence of `Efunc*` naming suggests an interpreter. Decompile a representative script-calling function to confirm.

4. **Source tree completeness**: Can we recover the full original source tree (2,000+ files) from the PDB alone, or will assert strings + naming conventions be the primary attribution mechanism?

5. **Xbox Live hook depth**: Are XONLINE/XNET called directly from game code, or through a clean abstraction layer? This determines shim complexity.

## 15. Session bootstrap for the gruntwork AI (paste-ready)

> You are reconstructing Call of Duty 3 (Xbox release build) to Win32 C++. Read `cod3_decomp/DECOMP_PLAN.md` fully and follow §7 exactly. Your work unit: the next `TODO` file in `PROGRESS.tsv` for the current phase. The project has a single IDA instance (release `codmp_xboxr.xbe`) — verify routing per §7 step 2. Reconstruct from Hex-Rays + disassembly; this is a release build — interpret, don't just transcribe. See §7.4 for optimization-artifact handling. Compile-gate, update PROGRESS.tsv, commit, repeat. Escalate per §7.2
