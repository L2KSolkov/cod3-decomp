#pragma once

struct IDirect3DDevice9;
struct IDirect3DVertexShader9;

// Translate an Xbox NV2A vertex-program token stream to a native D3D9 shader.
// The returned interface is owned by the caller and remains valid while the
// device is alive.  A null result means the token stream is unsupported.
IDirect3DVertexShader9* nullD3DCompileNV2AVertexShader(
    IDirect3DDevice9* device, const unsigned int* microcode);
