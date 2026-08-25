#pragma once

struct IDirect3DDevice9;
struct IDirect3DVertexShader9;
struct IDirect3DPixelShader9;
struct _D3DPixelShaderDef;

// Translate an Xbox NV2A vertex-program token stream to a native D3D9 shader.
// The returned interface is owned by the caller and remains valid while the
// device is alive.  A null result means the token stream is unsupported.
IDirect3DVertexShader9* nullD3DCompileNV2AVertexShader(
    IDirect3DDevice9* device, const unsigned int* microcode);

bool nullD3DProgramUsesHomogeneousDivide(const unsigned int* microcode);

// Translate the lightmapped cdWorldPixel combiner definition used by the
// world material path.  Other Xbox register-combiner programs remain on the
// fixed-function fallback until their IDA definitions are mapped.
IDirect3DPixelShader9* nullD3DCompileNV2AWorldPixelShader(
    IDirect3DDevice9* device, const _D3DPixelShaderDef* definition);

IDirect3DPixelShader9* nullD3DCompileNV2AFallbackPixelShader(
    IDirect3DDevice9* device, unsigned int textureMask);
