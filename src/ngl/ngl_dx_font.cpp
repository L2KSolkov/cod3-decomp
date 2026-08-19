// ============================================================================
// ngl_dx_font.cpp - push-buffer font string renderer (1 func, verified vs IDA).
// Source: src/dx/ngl_dx_font.cpp (ngl_xboxr)
// ============================================================================

#include "ngl/nglFont.h"
#include "ngl/ngl_dx_quad.h"
#include "ngl/ngl_dx_gpu.h"
#include "ngl/nglDebug.h"
#include "ngl/nglScene.h"
#include "d3d8.h"

#include <intrin.h>

// ============================================================================
// Cross-object externs
// ============================================================================
extern nglDebugStruct nglSyncDebug;                     // ngl_debug.o
extern nglPerfInfoStruct nglPerfInfo;                   // ngl_debug.o
extern nglDxRenderState nglDxState;                     // ngl_dx_state.o
extern nglScene* nglBuildScene;                         // ngl_scene.o
extern void nglDxSetTexture(unsigned int Stage, nglTexture* Tex,
                            unsigned int FilterFlags, unsigned int MaxAnisotropy);
extern void nglDxSetTextureU(unsigned int Stage, unsigned int Mode);
extern void nglDxSetTextureV(unsigned int Stage, unsigned int Mode);
extern void nglDxInitShaders(bool RegisterShaders);
extern void nglDxUnbindVertexBuffer();
extern float nglDxViewToScreenZ(float z, nglScene* Scene);
extern void gpuSetVertexShader(const unsigned int* shader);
extern unsigned int gpuHashPixelShader;
extern unsigned int gpuHashVertexBuffer;
extern unsigned int gpuHashVertexFormat;
extern gpuVertexFormat nglGpuPCUVVertexFmt;
extern _D3DVERTEXATTRIBUTEFORMAT gpuSetVertexShaderInputs;
extern unsigned int dword_417FC;
extern unsigned int dword_BC2D80;
extern bool _tlAssert(const char* file, int line, const char* expr, const char* desc);

// ============================================================================
// nglStringNode::Render - ea: 0x853A70
// ============================================================================
void nglStringNode::Render() {
    if (nglSyncDebug.DisableFonts != 0)
        return;
    if (this->Text == NULL || this->Font->Texture == NULL)
        return;
    unsigned __int64 FontTick = __rdtsc();
    nglTexture* Texture = this->Font->Texture;
    float ShiftU = 0.5f / (float)Texture->Width;
    float ShiftV = 0.5f / (float)Texture->Height;
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_CULLMODE, 0) == 0)
        D3DDevice_SetRenderState_CullMode(0);
    nglDxState.SetBlendMode(this->Font->BlendMode);
    if (D3DDevice_SetRenderState_ParameterCheck(D3DRS_SIMPLE_MAX, 0) == 0) {
        D3D__DirtyFlags |= 0x2000u;
        dword_BC2D80 = 0;
    }
    nglDxSetTexture(0, this->Font->Texture, this->Font->MapFlags, 3);
    nglDxSetTextureU(0, (this->Font->MapFlags & 0x40 | 0x20) >> 5);
    nglDxSetTextureV(0, (this->Font->MapFlags & 0x80 | 0x40) >> 6);
    nglDxInitShaders(false);
    gpuSetVertexShader(&nglGpuQuadPCUVVertexShader::Shader);
    if (gpuHashPixelShader != (unsigned int)nglGpuTexColPixelShader::Shader) {
        gpuHashPixelShader = (unsigned int)nglGpuTexColPixelShader::Shader;
        D3DDevice_SetPixelShaderProgram((const _D3DPixelShaderDef*)nglGpuTexColPixelShader::Shader);
    }
    float Z = nglDxViewToScreenZ(this->z, nglBuildScene);
    gpuHashVertexBuffer = 0;
    gpuHashVertexFormat = 0;
    D3DDevice_SelectVertexShaderDirect(nglGpuPCUVVertexFmt.VertexDeclaration, 0);
    int v2 = 0;
    int v6 = 0;
    nglStringSection* Section = this->Section;
    if (Section != NULL) {
        do {
            unsigned int Length = Section->Length;
            Section = Section->Next;
            v2 += Length;
            ++v6;
        } while (Section != NULL);
        if (v2 != 0) {
            nglDxUnbindVertexBuffer();
            unsigned int* v8 = D3DDevice_BeginPush(((96 * v2) >> 2) + v6 + 5);
            v8[0] = dword_417FC;
            v8[1] = 8;
            unsigned int* v9 = v8 + 2;
            nglStringSection* SectionIt = this->Section;
            do {
                nglStringSection* v10 = SectionIt;
                unsigned int Color = v10->Color;
                float Scale = v10->ScaleX;
                float ScaleY = v10->ScaleY;
                float Position = v10->x;
                float y = v10->y;
                unsigned int PBArraySize = (96 * v10->Length) >> 2;
                if (PBArraySize > 0x7FF) {
                    if (_tlAssert("src/dx/ngl_dx_font.cpp", 99,
                                  "PBArraySize <= D3DPUSH_MAX_COUNT",
                                  "Push buffer INLINE_ARRAY overflow ! (too many chars per string)"))
                        __debugbreak();
                }
                v9[0] = (PBArraySize << 18) + 1073747992;
                ++v9;
                if (v10->Length != 0) {
                    // SetMeasures writes both components of each output pair.
                    // Keep those pairs explicit; the release passes adjacent
                    // stack slots as the four float* arguments.
                    float offs[2] = {};
                    float size[2] = {};
                    float uvpos[2] = {};
                    float uvsize[2] = {};
                    float posY = y;
                    for (unsigned int i = v10->Length; i != 0; --i) {
                        unsigned char c = *this->Section->Text;
                        this->Font->SetMeasures(c, offs, size, uvpos, uvsize,
                                                Scale, ScaleY);
                        float x1 = offs[0] + Position;
                        float y1 = offs[1] + posY;
                        float u1 = uvpos[0] + ShiftU;
                        float v1 = uvpos[1] + ShiftV;
                        float x2 = size[0] + x1;
                        float y2 = size[1] + y1;
                        float u2 = uvsize[0] + u1;
                        float v2f = uvsize[1] + v1;
                        v9[0] = x1;
                        v9[1] = y1;
                        v9[2] = Z;
                        v9[3] = Color;
                        v9[4] = u1;
                        v9[5] = v2f;
                        v9[6] = x2;
                        v9[7] = y1;
                        v9[8] = Z;
                        v9[9] = Color;
                        v9[10] = u2;
                        v9[11] = v2f;
                        v9[12] = x2;
                        v9[13] = y2;
                        v9[14] = Z;
                        v9[15] = Color;
                        v9[16] = u2;
                        v9[17] = y2;
                        v9[18] = x1;
                        v9[19] = y2;
                        v9[20] = Z;
                        v9[21] = Color;
                        v9[22] = u1;
                        v9[23] = y2;
                        v9 += 24;
                        Position = (float)this->Font->GetCellWidth(c) * Scale + Position;
                        ++this->Section->Text;
                    }
                }
                SectionIt = this->Section->Next;
                this->Section = SectionIt;
            } while (SectionIt != NULL);
            v9[0] = dword_417FC;
            v9[1] = 0;
            D3DDevice_EndPush(v9 + 2);
            nglPerfInfo.FontCycles += __rdtsc() - FontTick;
        }
    }
}
