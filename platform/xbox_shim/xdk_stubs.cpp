// ============================================================================
// xdk_stubs.cpp - XDK (Xbox) API link stubs (NEVER reconstruct; Phase 6
// backends replace these). Generated from the linker unresolved list.
// NOTE: this toolchain only accepts calling-convention keywords AFTER the
// return type on free functions.
// ============================================================================

#include <stdint.h>

extern "C" {

void __fastcall D3DDevice_SetRenderState_Simple(unsigned int a0, unsigned int a1) {}
void __fastcall D3DDevice_SetVertexShaderConstant1Fast(unsigned int a0, unsigned int a1) {}
void __fastcall D3DDevice_SetVertexShaderConstantNotInlineFast(unsigned int a0, unsigned int a1, unsigned int a2) {}
void __cdecl compress2(void) {}
unsigned int __stdcall D3DBaseTexture_GetLevelCount(unsigned int a0) { return 0; }
void __stdcall D3DCubeTexture_GetCubeMapSurface2(unsigned int a0, unsigned int a1, unsigned int a2) {}
void __stdcall D3DDevice_BeginPush(unsigned int a0) {}
void __stdcall D3DDevice_BlockOnFence(unsigned int a0) {}
void __stdcall D3DDevice_BlockUntilIdle(void) {}
void __stdcall D3DDevice_Clear(unsigned int a0, unsigned int a1, unsigned int a2, unsigned int a3, unsigned int a4, unsigned int a5) {}
void* __stdcall D3DDevice_CreateIndexBuffer2(unsigned int a0) { return nullptr; }
void* __stdcall D3DDevice_CreateSurface2(unsigned int a0, unsigned int a1, unsigned int a2, unsigned int a3) { return nullptr; }
void* __stdcall D3DDevice_CreateTexture2(unsigned int a0, unsigned int a1, unsigned int a2, unsigned int a3, unsigned int a4, unsigned int a5, unsigned int a6) { return nullptr; }
void* __stdcall D3DDevice_CreateVertexBuffer2(unsigned int a0) { return nullptr; }
void __stdcall D3DDevice_DrawIndexedVertices(unsigned int a0, unsigned int a1, unsigned int a2) {}
void __stdcall D3DDevice_DrawVerticesUP(unsigned int a0, unsigned int a1, unsigned int a2, unsigned int a3) {}
void __stdcall D3DDevice_EndPush(unsigned int a0) {}
void* __stdcall D3DDevice_GetBackBuffer2(unsigned int a0) { return nullptr; }
void* __stdcall D3DDevice_GetDepthStencilSurface2(void) { return nullptr; }
void __stdcall D3DDevice_GetDeviceCaps(unsigned int a0) {}
void __stdcall D3DDevice_InsertCallback(unsigned int a0, unsigned int a1, unsigned int a2) {}
unsigned int __stdcall D3DDevice_InsertFence(void) { return 0; }
void __stdcall D3DDevice_LoadVertexShaderProgram(unsigned int a0, unsigned int a1) {}
void __stdcall D3DDevice_PersistDisplay(void) {}
void __stdcall D3DDevice_Reset(unsigned int a0) {}
void __stdcall D3DDevice_SetGammaRamp(unsigned int a0, unsigned int a1) {}
void __stdcall D3DDevice_SetIndices(unsigned int a0, unsigned int a1) {}
void __stdcall D3DDevice_SetPalette(unsigned int a0, unsigned int a1) {}
void __stdcall D3DDevice_SetRenderState_MultiSampleAntiAlias(unsigned int a0) {}
void __stdcall D3DDevice_SetRenderState_RopZCmpAlwaysRead(unsigned int a0) {}
void __stdcall D3DDevice_SetRenderState_StencilEnable(unsigned int a0) {}
void __stdcall D3DDevice_SetRenderState_YuvEnable(unsigned int a0) {}
void __stdcall D3DDevice_SetRenderState_ZEnable(unsigned int a0) {}
void __stdcall D3DDevice_SetRenderTarget(unsigned int a0, unsigned int a1) {}
void __stdcall D3DDevice_SetShaderConstantMode(unsigned int a0) {}
void __stdcall D3DDevice_SetTexture(unsigned int a0, unsigned int a1) {}
void __stdcall D3DDevice_SetTextureState_ParameterCheck(unsigned int a0, unsigned int a1, unsigned int a2) {}
void __stdcall D3DDevice_SetVertexShader(unsigned int a0) {}
void __stdcall D3DDevice_SetVerticalBlankCallback(unsigned int a0) {}
void __stdcall D3DDevice_SetViewport(unsigned int a0) {}
void __stdcall D3DDevice_Swap(unsigned int a0) {}
void __stdcall D3DDevice_SwitchTexture(unsigned int a0, unsigned int a1, unsigned int a2) {}
void __stdcall D3DPalette_Lock2(unsigned int a0, unsigned int a1) {}
void __stdcall D3DResource_BlockUntilNotBusy(unsigned int a0) {}
void __stdcall D3DResource_Register(unsigned int a0, unsigned int a1) {}
void __stdcall D3DSurface_GetDesc(unsigned int a0, unsigned int a1) {}
void __stdcall D3DSurface_LockRect(unsigned int a0, unsigned int a1, unsigned int a2, unsigned int a3) {}
void __stdcall D3DTexture_GetLevelDesc(unsigned int a0, unsigned int a1, unsigned int a2) {}
void __stdcall D3DTexture_GetSurfaceLevel2(unsigned int a0, unsigned int a1) {}
void __stdcall D3DTexture_LockRect(unsigned int a0, unsigned int a1, unsigned int a2, unsigned int a3, unsigned int a4) {}
void __stdcall D3DVertexBuffer_Lock2(unsigned int a0, unsigned int a1) {}
void __stdcall D3DXCreateTextureFromFileInMemoryEx(unsigned int a0, unsigned int a1, unsigned int a2, unsigned int a3, unsigned int a4, unsigned int a5, unsigned int a6, unsigned int a7, unsigned int a8, unsigned int a9, unsigned int a10, unsigned int a11, unsigned int a12, unsigned int a13, unsigned int a14) {}
const char* __stdcall D3DXGetErrorStringA(unsigned int a0, unsigned int a1, unsigned int a2) { return nullptr; }
int __stdcall Direct3D_CreateDevice(unsigned int a0, unsigned int a1, unsigned int a2, unsigned int a3, unsigned int a4, unsigned int a5) { return 0; }
void __stdcall Direct3D_SetPushBufferSize(unsigned int a0, unsigned int a1) {}
void* __stdcall Direct3DCreate8(unsigned int a0) { return nullptr; }
void __stdcall LiveEngine_DoWork(unsigned int a0, unsigned int a1) {}
void __stdcall LiveEngine_EnableFeature(unsigned int a0, unsigned int a1) {}
void __stdcall LiveEngine_EndFeature(unsigned int a0) {}
void __stdcall LiveEngine_GetExitInfo(unsigned int a0, unsigned int a1) {}
void* __stdcall LiveEngine_GetFeatureInterface(unsigned int a0, unsigned int a1, unsigned int a2, unsigned int a3) { return nullptr; }
void __stdcall LiveEngine_GetNotifications(unsigned int a0, unsigned int a1, unsigned int a2, unsigned int a3) {}
void __stdcall LiveEngine_LogOff(unsigned int a0) {}
void __stdcall LiveEngine_NotificationSetState(unsigned int a0, unsigned int a1, unsigned int a2, unsigned int a3, unsigned int a4, unsigned int a5, unsigned int a6) {}
void __stdcall LiveEngine_Reboot(unsigned int a0, unsigned int a1) {}
void __stdcall LiveEngine_Release(unsigned int a0) {}
void __stdcall LiveEngine_Render(unsigned int a0, unsigned int a1) {}
void __stdcall LiveEngine_SetInput(unsigned int a0, unsigned int a1, unsigned int a2) {}
void __stdcall LiveEngine_SetProperty(unsigned int a0, unsigned int a1, unsigned int a2) {}
void __stdcall LiveEngine_SetUIPlugin(unsigned int a0, unsigned int a1) {}
void __stdcall LiveEngine_StartFeature(unsigned int a0, unsigned int a1, unsigned int a2) {}
void __stdcall LiveEngine_UseVoiceMail(unsigned int a0, unsigned int a1) {}
void* __stdcall UIXCreateLiveEngine(unsigned int a0, unsigned int a1, unsigned int a2) { return nullptr; }
void* __stdcall UIXCreateUIPlugin(unsigned int a0, unsigned int a1) { return nullptr; }
void __cdecl uncompress(void) {}
unsigned int __stdcall XGetLanguage(void) { return 0; }
unsigned int __cdecl XGetVideoFlags(void) { return 0; }
unsigned int __stdcall XGIsSwizzledFormat(unsigned int a0) { return 0; }
void __stdcall XGSetPaletteHeader(unsigned int a0, unsigned int a1, unsigned int a2) {}
void __stdcall XGSetTextureHeader(unsigned int a0, unsigned int a1, unsigned int a2, unsigned int a3, unsigned int a4, unsigned int a5, unsigned int a6, unsigned int a7, unsigned int a8) {}
void __stdcall XGSwizzleRect(unsigned int a0, unsigned int a1, unsigned int a2, unsigned int a3, unsigned int a4, unsigned int a5, unsigned int a6, unsigned int a7) {}
void __stdcall XGWriteSurfaceToFile(unsigned int a0, unsigned int a1) {}
void __stdcall XHVEngine_DoWork(unsigned int a0) {}
void __stdcall XHVEngine_EnableProcessingMode(unsigned int a0, unsigned int a1) {}
void __stdcall XHVEngine_IsTalking(unsigned int a0, unsigned int a1, unsigned int a2, unsigned int a3) {}
void __stdcall XHVEngine_RegisterLocalTalker(unsigned int a0, unsigned int a1) {}
void __stdcall XHVEngine_RegisterRemoteTalker(unsigned int a0, unsigned int a1, unsigned int a2, unsigned int a3) {}
void __stdcall XHVEngine_Release(unsigned int a0) {}
void __stdcall XHVEngine_SetCallbackInterface(unsigned int a0, unsigned int a1) {}
void __stdcall XHVEngine_SetMixBinMapping(unsigned int a0, unsigned int a1, unsigned int a2, unsigned int a3, unsigned int a4, unsigned int a5) {}
void __stdcall XHVEngine_SetPlaybackPriority(unsigned int a0, unsigned int a1, unsigned int a2, unsigned int a3, unsigned int a4, unsigned int a5) {}
void __stdcall XHVEngine_SetProcessingMode(unsigned int a0, unsigned int a1, unsigned int a2) {}
void __stdcall XHVEngine_SubmitIncomingVoicePacket(unsigned int a0, unsigned int a1, unsigned int a2, unsigned int a3, unsigned int a4, unsigned int a5) {}
void __stdcall XHVEngine_UnregisterRemoteTalker(unsigned int a0, unsigned int a1, unsigned int a2, unsigned int a3) {}
void* __stdcall XHVEngineCreate(unsigned int a0, unsigned int a1) { return nullptr; }
void __stdcall XInputGetState(unsigned int a0, unsigned int a1) {}
void __stdcall XInputSetState(unsigned int a0, unsigned int a1) {}
int __stdcall XNetQosListen(unsigned int a0, unsigned int a1, unsigned int a2, unsigned int a3, unsigned int a4) { return 0; }
void __stdcall XNetRegisterKey(unsigned int a0, unsigned int a1) {}
void __stdcall XNetUnregisterKey(unsigned int a0) {}
unsigned int __stdcall XOnlineGetLogonUsers(void) { return 0; }
void __stdcall XOnlineMatchSessionCreate(unsigned int a0, unsigned int a1, unsigned int a2, unsigned int a3, unsigned int a4, unsigned int a5, unsigned int a6, unsigned int a7) {}
void __stdcall XOnlineMatchSessionDelete(unsigned int a0, unsigned int a1, unsigned int a2, unsigned int a3) {}
void __stdcall XOnlineMatchSessionGetInfo(unsigned int a0, unsigned int a1, unsigned int a2) {}
void __stdcall XOnlineMatchSessionUpdate(unsigned int a0, unsigned int a1, unsigned int a2, unsigned int a3, unsigned int a4, unsigned int a5, unsigned int a6, unsigned int a7, unsigned int a8, unsigned int a9) {}
void __stdcall XOnlineMutelistGet(unsigned int a0, unsigned int a1, unsigned int a2, unsigned int a3, unsigned int a4, unsigned int a5) {}
void __stdcall XOnlineSaveLogonState(unsigned int a0) {}
void __stdcall XOnlineStartup(unsigned int a0) {}
void __stdcall XOnlineTaskClose(unsigned int a0) {}
void __stdcall XOnlineTaskContinue(unsigned int a0) {}

}  // extern "C"

// LNK2001-only imports surfaced after the first pass
extern "C" {
void __stdcall D3DDevice_SelectVertexShaderDirect(unsigned int a0, unsigned int a1) {}
void __stdcall D3DDevice_SetPixelShaderProgram(unsigned int a0) {}
void __stdcall D3DDevice_SetRenderState_CullMode(unsigned int a0) {}
int __stdcall D3DDevice_SetRenderState_ParameterCheck(unsigned int a0, unsigned int a1) { return 0; }
void __stdcall D3DDevice_SetVertexShaderInputDirect(unsigned int a0, unsigned int a1, unsigned int a2) {}
void __stdcall D3DResource_Release(unsigned int a0) {}
int __cdecl __fpclass(double a0) { (void)a0; return 0; }
}
