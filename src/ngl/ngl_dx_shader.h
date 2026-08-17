// ============================================================================
// ngl_dx_shader.h - vertex shader constant setup + shader registration.
// Source: src/dx/ngl_dx_shader.cpp (ngl_xboxr)
// ============================================================================
#ifndef COD3_NGL_NGL_DX_SHADER_H
#define COD3_NGL_NGL_DX_SHADER_H

class nglMeshNode;
struct nglMeshSection;

// ngl_dx_shader.o (functions, defined in ngl_dx_shader.cpp)
void nglDxRegisterVShader(unsigned int* VS, const unsigned int* Microcode);
void nglDxRegisterPShader(unsigned int** PS, const unsigned int* Microcode);
void nglDxSetBonesWorld(int p, nglMeshNode* MeshNode, nglMeshSection* Section);
void nglDxSetupVShaderFog(int VSReg, nglMeshNode* MeshNode, float FogNear, float FogFar,
                          float FogMin, float FogMax);
void nglDxSetupVShaderLights(int VSReg, nglMeshNode* MeshNode);
void nglDxRegisterInternalShaders();
void nglDxInitShaders(bool RegisterShaders);
void nglDxSetupVShaderBones(int VSReg, nglMeshNode* MeshNode, nglMeshSection* Section);
void nglDxSetBonesLocal(int p, nglMeshNode* MeshNode, nglMeshSection* Section);

#endif // COD3_NGL_NGL_DX_SHADER_H
