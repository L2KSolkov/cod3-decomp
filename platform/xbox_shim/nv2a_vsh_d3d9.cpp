#include "nv2a_vsh_d3d9.h"

#include <d3d9.h>
#include <d3dcompiler.h>
#include <windows.h>

#include <algorithm>
#include <cstdint>
#include <map>
#include <sstream>
#include <string>

namespace {

// Layout copied from the IDA local _D3DPixelShaderDef type.  d3d8.h cannot
// be included here because this translation unit also consumes the native
// D3D9 declarations, whose enum tags intentionally differ.
struct PixelShaderDefLayout {
    unsigned int PSAlphaInputs[8];
    unsigned int PSFinalCombinerInputsABCD;
    unsigned int PSFinalCombinerInputsEFG;
    unsigned int PSConstant0[8];
    unsigned int PSConstant1[8];
    unsigned int PSAlphaOutputs[8];
    unsigned int PSRGBInputs[8];
    unsigned int PSCompareMode;
    unsigned int PSFinalCombinerConstant0;
    unsigned int PSFinalCombinerConstant1;
    unsigned int PSRGBOutputs[8];
    unsigned int PSCombinerCount;
    unsigned int PSTextureModes;
    unsigned int PSDotMapping;
    unsigned int PSInputTexture;
    unsigned int PSC0Mapping;
    unsigned int PSC1Mapping;
    unsigned int PSFinalCombinerConstants;
};
static_assert(sizeof(PixelShaderDefLayout) == 0xF0, "pixel shader layout mismatch");

enum Field {
    F_ILU, F_MAC, F_CONST, F_V,
    F_A_NEG, F_A_X, F_A_Y, F_A_Z, F_A_W, F_A_R, F_A_MUX,
    F_B_NEG, F_B_X, F_B_Y, F_B_Z, F_B_W, F_B_R, F_B_MUX,
    F_C_NEG, F_C_X, F_C_Y, F_C_Z, F_C_W, F_C_R_HIGH, F_C_R_LOW, F_C_MUX,
    F_OUT_MAC_MASK, F_OUT_R, F_OUT_ILU_MASK, F_OUT_O_MASK, F_OUT_ORB,
    F_OUT_ADDRESS, F_OUT_MUX, F_A0X, F_FINAL
};

struct FieldInfo { unsigned char token; unsigned char bit; unsigned char width; };

static const FieldInfo kFields[] = {
    {1,25,3}, {1,21,4}, {1,13,8}, {1,9,4},
    {1,8,1}, {1,6,2}, {1,4,2}, {1,2,2}, {1,0,2}, {2,28,4}, {2,26,2},
    {2,25,1}, {2,23,2}, {2,21,2}, {2,19,2}, {2,17,2}, {2,13,4}, {2,11,2},
    {2,10,1}, {2,8,2}, {2,6,2}, {2,4,2}, {2,2,2}, {2,0,2}, {3,30,2}, {3,28,2},
    {3,24,4}, {3,20,4}, {3,16,4}, {3,12,4}, {3,11,1}, {3,3,8}, {3,2,1}, {3,1,1}, {3,0,1}
};

static unsigned int FieldValue(const unsigned int* token, Field field) {
    const FieldInfo& info = kFields[(unsigned int)field];
    return (token[info.token] >> info.bit) & ((1u << info.width) - 1u);
}

static int ConstantRegister(unsigned int value) {
    const int bank = (int)((value >> 5) & 7u) - 3;
    return bank * 32 + (int)(value & 31u) + 96;
}

static const char* SwizzleName(unsigned int value) {
    static const char* names[] = {"x", "y", "z", "w"};
    return names[value & 3u];
}

static std::string Swizzle(const unsigned int* token, Field first, bool forceScalar) {
    unsigned int values[4];
    for (unsigned int i = 0; i < 4; ++i)
        values[i] = FieldValue(token, (Field)((unsigned int)first + i));
    if (forceScalar)
        values[1] = values[2] = values[3] = values[0];
    if (values[0] == 0 && values[1] == 1 && values[2] == 2 && values[3] == 3)
        return std::string();
    std::string result = ".";
    for (unsigned int value : values)
        result += SwizzleName(value);
    if (values[0] == values[1] && values[1] == values[2] && values[2] == values[3])
        result.resize(2);
    else if (values[1] == values[2] && values[2] == values[3])
        result.resize(3);
    else if (values[2] == values[3])
        result.resize(4);
    return result;
}

static std::string Source(const unsigned int* token, Field muxField, Field negField,
                          Field registerField, Field swizzleField, bool forceScalar) {
    std::ostringstream result;
    if (FieldValue(token, negField) != 0)
        result << '-';
    switch (FieldValue(token, muxField)) {
    case 1:
        result << 'r' << FieldValue(token, registerField);
        break;
    case 2:
        result << 'v' << FieldValue(token, F_V);
        break;
    case 3:
        result << "c[" << ConstantRegister(FieldValue(token, F_CONST)) << "]";
        break;
    default:
        return "0.0";
    }
    result << Swizzle(token, swizzleField, forceScalar);
    return result.str();
}

static std::string InputA(const unsigned int* token) {
    return Source(token, F_A_MUX, F_A_NEG, F_A_R, F_A_X, false);
}
static std::string InputB(const unsigned int* token) {
    return Source(token, F_B_MUX, F_B_NEG, F_B_R, F_B_X, false);
}
// C uses a split 6-bit temporary-register address. Keep this separate from
// Source so the exact NV2A field packing is visible at the call site.
static std::string InputCExact(const unsigned int* token, bool forceScalar) {
    std::ostringstream result;
    if (FieldValue(token, F_C_NEG) != 0)
        result << '-';
    switch (FieldValue(token, F_C_MUX)) {
    case 1:
        result << 'r' << ((FieldValue(token, F_C_R_HIGH) << 2) |
                          FieldValue(token, F_C_R_LOW));
        break;
    case 2:
        result << 'v' << FieldValue(token, F_V);
        break;
    case 3:
        result << "c[";
        if (FieldValue(token, F_A0X) != 0)
            result << "A0+";
        result << ConstantRegister(FieldValue(token, F_CONST)) << "]";
        break;
    default:
        return "0.0";
    }
    result << Swizzle(token, F_C_X, forceScalar);
    return result.str();
}

static std::string Mask(unsigned int mask) {
    std::string result;
    if (mask & 8) result += 'x';
    if (mask & 4) result += 'y';
    if (mask & 2) result += 'z';
    if (mask & 1) result += 'w';
    return result;
}

static bool IsScalarSource(const std::string& value) {
    if (value == "0.0")
        return true;
    if (!value.empty() && value[0] == '-')
        return IsScalarSource(value.substr(1));
    if (value.size() >= 2 && value[value.size() - 2] == '.')
    {
        const char component = value[value.size() - 1];
        return component == 'x' || component == 'y'
            || component == 'z' || component == 'w';
    }
    return false;
}

static const char* MacName(unsigned int opcode) {
    static const char* names[] = {
        "", "MOV", "MUL", "ADD", "MAD", "DP3", "DPH", "DP4", "DST",
        "MIN", "MAX", "SLT", "SGE", "ARL"
    };
    return opcode < sizeof(names) / sizeof(names[0]) ? names[opcode] : "";
}

static const char* IluName(unsigned int opcode) {
    static const char* names[] = {"", "MOV", "RCP", "RCC", "RSQ", "EXP", "LOG", "LIT"};
    return opcode < sizeof(names) / sizeof(names[0]) ? names[opcode] : "";
}

static bool IsIluScalar(unsigned int opcode) {
    return opcode >= 2 && opcode <= 6;
}

static std::string BroadcastScalar(const std::string& value) {
    return "float4((" + value + "), (" + value + "), (" + value + "), (" + value + "))";
}

static std::string MacExpression(unsigned int opcode, const std::string& a,
                                 const std::string& b, const std::string& c) {
    switch (opcode) {
    case 1: return a;
    case 2: return "(" + a + " * " + b + ")";
    case 3: return "(" + a + " + " + c + ")";
    case 4: return "(" + a + " * " + b + " + " + c + ")";
    case 5: return BroadcastScalar("dot(" + a + ".xyz, " + b + ".xyz)");
    case 6: return BroadcastScalar("dot(float4(" + a + ".xyz, 1.0), " + b + ")");
    case 7: return BroadcastScalar("dot(" + a + ", " + b + ")");
    case 8: return "float4(1.0, " + a + ".y * " + b + ".y, " + a + ".z, " + b + ".w)";
    case 9: return "min(" + a + ", " + b + ")";
    case 10: return "max(" + a + ", " + b + ")";
    case 11:
        return "float4(" + a + ".x < " + b + ".x ? 1.0 : 0.0, "
             + a + ".y < " + b + ".y ? 1.0 : 0.0, "
             + a + ".z < " + b + ".z ? 1.0 : 0.0, "
             + a + ".w < " + b + ".w ? 1.0 : 0.0)";
    case 12:
        return "float4(" + a + ".x >= " + b + ".x ? 1.0 : 0.0, "
             + a + ".y >= " + b + ".y ? 1.0 : 0.0, "
             + a + ".z >= " + b + ".z ? 1.0 : 0.0, "
             + a + ".w >= " + b + ".w ? 1.0 : 0.0)";
    case 13: return a;
    default: return std::string();
    }
}

static std::string IluExpression(unsigned int opcode, const std::string& c) {
    switch (opcode) {
    case 1: return c;
    case 2: return BroadcastScalar("1.0 / " + c);
    // NV2A RCC preserves the reciprocal sign and clamps away zero/inf.  The
    // absolute-value form loses the sign of homogeneous w and mirrors the
    // position for vertices behind the camera.
    case 3: return BroadcastScalar(
        "(" + c + " >= 0.0 ? clamp(1.0 / " + c
        + ", 5.42101086e-20, 1.84467441e19) : clamp(1.0 / " + c
        + ", -1.84467441e19, -5.42101086e-20))");
    case 4: return BroadcastScalar("rsqrt(abs(" + c + "))");
    case 5: return "nv2aExp(" + c + ")";
    case 6: return "nv2aLog(" + c + ")";
    case 7: return "nv2aLit(" + c + ")";
    default: return std::string();
    }
}

static const char* OutputName(unsigned int address) {
    static const char* names[] = {
        "r12", nullptr, nullptr, "oD0", "oD1", "oFog", "oPts",
        "oB0", "oB1", "oT0", "oT1", "oT2", "oT3",
        nullptr, nullptr, "A0.x"
    };
    const unsigned int index = address & 15u;
    return index < sizeof(names) / sizeof(names[0]) ? names[index] : nullptr;
}

static void AppendWrite(std::ostringstream& source, const std::string& expression,
                        const char* destination, const std::string& mask,
                        bool scalar) {
    if (mask.empty())
        return;
    if (scalar)
    {
        if (mask.size() == 1)
            source << "  " << destination << "." << mask << " = " << expression << ";\n";
        else
            source << "  " << destination << "." << mask << " = float4((" << expression
                   << "), (" << expression << "), (" << expression << "), (" << expression
                   << "))." << mask << ";\n";
    }
    else if (mask == "xyzw")
        source << "  " << destination << " = " << expression << ";\n";
    else
        source << "  " << destination << "." << mask << " = (" << expression << ")." << mask << ";\n";
}

static void AppendAddressWrite(std::ostringstream& source, const std::string& expression,
                               bool scalar) {
    source << "  A0=(int)floor(";
    if (!scalar)
        source << "(" << expression << ").x";
    else
        source << expression;
    source << "+0.001);\n";
}

static void AppendInstruction(std::ostringstream& source, const unsigned int* token,
                              unsigned int instructionIndex) {
    const unsigned int mac = FieldValue(token, F_MAC);
    const unsigned int ilu = FieldValue(token, F_ILU);
    if (mac == 0 && ilu == 0)
        return;
    const std::string a = InputA(token);
    const std::string b = InputB(token);
    const std::string cMac = InputCExact(token, false);
    const std::string cIlu = InputCExact(token, IsIluScalar(ilu));
    const unsigned int outAddress = FieldValue(token, F_OUT_ADDRESS);
    const unsigned int outRegister = FieldValue(token, F_OUT_R);
    const bool paired = mac != 0 && ilu != 0;
    const bool isArl = mac == 13;
    const bool delayArl = isArl && paired;
    const std::string arlTemp = "nv2aArlTemp" + std::to_string(instructionIndex);
    const std::string macExpr = mac == 0 ? std::string() : MacExpression(mac, a, b, cMac);
    const std::string iluExpr = ilu == 0 ? std::string() : IluExpression(ilu, cIlu);
    const char* outputName = OutputName(outAddress);
    const bool outputIsAddress = (outAddress & 15u) == 15u;

    // Bit 11 selects the output register bank.  The alternate bank is the
    // writable constant space; it is intentionally ignored here because D3D9
    // shader constants are uploaded by the game and are not shader outputs.
    const bool writesOutput = FieldValue(token, F_OUT_ORB) != 0;
    const bool macScalar = mac == 1 ? IsScalarSource(a)
        : (mac == 2 || mac == 3 || mac == 4 || mac == 9 || mac == 10)
            ? IsScalarSource(a) && IsScalarSource(b)
            : false;
    const bool iluScalar = ilu == 1 ? IsScalarSource(cIlu) : false;
    if (isArl) {
        if (delayArl)
            source << "  int " << arlTemp << "=(int)floor(" << a << ".x+0.001);\n";
        else
            source << "  A0=(int)floor(" << a << ".x+0.001);\n";
    }
    if (isArl && delayArl) {
        if (ilu != 0 && FieldValue(token, F_OUT_ILU_MASK) != 0) {
            const std::string destination = "r1";
            AppendWrite(source, iluExpr, destination.c_str(),
                        Mask(FieldValue(token, F_OUT_ILU_MASK)), iluScalar);
        }
        source << "  A0=" << arlTemp << ";\n";
        return;
    }
    if (isArl)
        return;
    if (writesOutput && mac != 0 && FieldValue(token, F_OUT_MUX) == 0) {
        if (outputIsAddress)
            AppendAddressWrite(source, macExpr, macScalar);
        else if (outputName != nullptr)
            AppendWrite(source, macExpr, outputName, Mask(FieldValue(token, F_OUT_O_MASK)), macScalar);
    }
    if (writesOutput && ilu != 0 && FieldValue(token, F_OUT_MUX) != 0) {
        if (outputIsAddress)
            AppendAddressWrite(source, iluExpr, iluScalar);
        else if (outputName != nullptr)
            AppendWrite(source, iluExpr, outputName, Mask(FieldValue(token, F_OUT_O_MASK)), iluScalar);
    }

    // xemu delays the temporary MAC result for paired instructions so the
    // ILU reads the pre-instruction C source, even when the MAC destination
    // aliases that source.  A paired MAC targeting R1 is discarded by NV2A.
    const unsigned int macMaskValue = FieldValue(token, F_OUT_MAC_MASK);
    const bool discardPairedMac = paired && outRegister == 1;
    const bool delayMac = paired && !discardPairedMac && macMaskValue != 0;
    const std::string macMask = Mask(macMaskValue);
    const std::string macTemp = "nv2aMacTemp" + std::to_string(instructionIndex);
    if (delayMac) {
        source << "  float4 " << macTemp << "=float4(0,0,0,0);\n";
        AppendWrite(source, macExpr, macTemp.c_str(), macMask, macScalar);
    } else if (mac != 0 && macMaskValue != 0 && !discardPairedMac) {
        const std::string destination = "r" + std::to_string(outRegister);
        AppendWrite(source, macExpr, destination.c_str(), macMask, macScalar);
    }
    if (ilu != 0 && FieldValue(token, F_OUT_ILU_MASK) != 0) {
        const unsigned int iluRegister = paired ? 1 : outRegister;
        const std::string destination = "r" + std::to_string(iluRegister);
        AppendWrite(source, iluExpr, destination.c_str(), Mask(FieldValue(token, F_OUT_ILU_MASK)), iluScalar);
    }
    if (delayMac) {
        const std::string destination = "r" + std::to_string(outRegister);
        if (macMask == "xyzw")
            source << "  " << destination << "=" << macTemp << ";\n";
        else
            source << "  " << destination << "." << macMask << "=" << macTemp << "." << macMask << ";\n";
    }
}

static const unsigned int* ResolveProgram(const unsigned int* microcode) {
    if (microcode == nullptr)
        return nullptr;
    if ((microcode[0] & 0xffffu) == 0x2078u)
        return microcode;
    const unsigned int* indirect = reinterpret_cast<const unsigned int*>(
        (uintptr_t)(*(const unsigned long*)microcode));
    return indirect != nullptr && (indirect[0] & 0xffffu) == 0x2078u
        ? indirect : nullptr;
}

static bool ProgramUsesHomogeneousDivide(const unsigned int* microcode) {
    if (microcode == nullptr)
        return false;
    const unsigned int instructionCount = microcode[0] >> 16;
    for (unsigned int i = 0; i < instructionCount; ++i) {
        if (FieldValue(microcode + 1 + i * 4, F_ILU) == 3)
            return true;
    }
    return false;
}

static std::string BuildHlsl(const unsigned int* microcode) {
    const unsigned int header = microcode[0];
    const unsigned int instructionCount = header >> 16;
    const bool usesHomogeneousDivide = ProgramUsesHomogeneousDivide(microcode);
    if ((header & 0xffffu) != 0x2078u || instructionCount == 0 || instructionCount > 136)
        return std::string();

    std::ostringstream source;
    source << "struct VSIn { float4 v0 : POSITION;";
    for (unsigned int i = 1; i < 8; ++i)
        source << " float4 v" << i << " : TEXCOORD" << (i - 1) << ";";
    source << " };\n"
           << "struct VSOut { float4 oPos : POSITION; float4 oD0 : COLOR0; float4 oD1 : COLOR1;"
           << " float4 oT0 : TEXCOORD0; float4 oT1 : TEXCOORD1; float4 oT2 : TEXCOORD2;"
           << " float4 oT3 : TEXCOORD3; float4 oB0 : TEXCOORD4; float4 oB1 : TEXCOORD5;"
           << " float oFog : FOG; float oPts : PSIZE; };\n"
           << "float4 c[192] : register(c0);\n"
           << "float4 nv2aExp(float src) { float whole=floor(src);"
           << " return float4(exp2(whole),src-whole,exp2(src),1.0); }\n"
           << "float4 nv2aLog(float src) { float tmp=abs(src);"
           << " if (tmp==0.0) return float4(-1.0/0.0,1.0,-1.0/0.0,1.0);"
           << " float whole=floor(log2(tmp));"
           << " return float4(whole,tmp/exp2(whole),log2(tmp),1.0); }\n"
           << "float4 nv2aLit(float4 src) { float epsilon=1.0/256.0;"
           << " float x=max(src.x,0.0); float y=max(src.y,0.0);"
           << " float w=clamp(src.w,-(128.0-epsilon),128.0-epsilon);"
           << " return float4(1.0,x,x>0.0?exp2(w*log2(y)):0.0,1.0); }\n"
           << "float nv2aNaNToOne(float value) { return value != value ? 1.0 : value; }\n"
           << "float4 nv2aColorClamp(float4 value) { return saturate(float4("
           << "nv2aNaNToOne(value.x),nv2aNaNToOne(value.y),"
           << "nv2aNaNToOne(value.z),nv2aNaNToOne(value.w))); }\n"
           << "VSOut main(VSIn input) { VSOut output;\n"
           << "  int A0=0;\n"
           << "  float4 v0=input.v0,v1=input.v1,v2=input.v2;\n"
           << "  float4 v3=input.v3,v4=input.v4,v5=input.v5,v6=input.v6,v7=input.v7;\n";
    if (usesHomogeneousDivide) {
        // The standard world declaration puts the three packed 0x16
        // attributes after position, UV0..2, and color: v5, v6, and v7.
        // D3D9's UDEC3 fetch exposes their unsigned 10-bit lanes, so decode
        // the NV2A signed 10:10:10 representation here.  Do not normalize
        // v3/v4: those registers carry the third UV and D3DCOLOR in the
        // observed release declaration and are already in shader units.
        source << "  v5=float4((v5.x>=512.0?v5.x-1024.0:v5.x)/511.0,"
                   "(v5.y>=512.0?v5.y-1024.0:v5.y)/511.0,"
                   "(v5.z>=512.0?v5.z-1024.0:v5.z)/511.0,1.0);\n"
                   "  v6=float4((v6.x>=512.0?v6.x-1024.0:v6.x)/511.0,"
                   "(v6.y>=512.0?v6.y-1024.0:v6.y)/511.0,"
                   "(v6.z>=512.0?v6.z-1024.0:v6.z)/511.0,1.0);\n"
                   "  v7=float4((v7.x>=512.0?v7.x-1024.0:v7.x)/511.0,"
                   "(v7.y>=512.0?v7.y-1024.0:v7.y)/511.0,"
                   "(v7.z>=512.0?v7.z-1024.0:v7.z)/511.0,1.0);\n";
    }
    source << "  float4 r0=0,r1=0,r2=0,r3=0,r4=0,r5=0,r6=0,r7=0;\n"
           << "  float4 r8=0,r9=0,r10=0,r11=0;\n"
           << "  float4 r12=float4(0,0,0,1);\n"
           << "  float4 oD0=float4(0,0,0,1),oD1=float4(0,0,0,1);\n"
           << "  float4 oT0=float4(0,0,0,1),oT1=float4(0,0,0,1);\n"
           << "  float4 oT2=float4(0,0,0,1),oT3=float4(0,0,0,1);\n"
           << "  float4 oB0=float4(0,0,0,1),oB1=float4(0,0,0,1);\n"
           << "  float4 oFog=float4(0,0,0,1),oPts=float4(0,0,0,1);\n";
    for (unsigned int i = 0; i < instructionCount; ++i)
        AppendInstruction(source, microcode + 1 + i * 4, i);
    if (usesHomogeneousDivide)
        source << "  float4 screenPos=r12;"
               << " screenPos.xy=trunc(screenPos.xy*16.0)/16.0;"
               << " screenPos.w=(screenPos.w>=0.0?clamp(screenPos.w,5.42101086e-20,1.84467441e19):clamp(screenPos.w,-1.84467441e19,-5.42101086e-20));"
               << " float3 ndc=float3((screenPos.x-c[191].x)*c[191].z,(screenPos.y-c[191].y)*c[191].w,screenPos.z*5.96046448e-8);"
               << " output.oPos=float4(ndc*screenPos.w,screenPos.w); output.oD0=nv2aColorClamp(oD0); output.oD1=oD1; output.oT0=oT0;"
               << " output.oT1=oT1; output.oT2=oT2; output.oT3=oT3; output.oB0=nv2aColorClamp(oB0);"
               << " output.oB1=oB1; output.oFog=oFog.x; output.oPts=oPts.x; return output; }\n";
    else
        source << "  output.oPos=r12; output.oD0=nv2aColorClamp(oD0); output.oD1=oD1; output.oT0=oT0;"
               << " output.oT1=oT1; output.oT2=oT2; output.oT3=oT3; output.oB0=nv2aColorClamp(oB0);"
               << " output.oB1=oB1; output.oFog=oFog.x; output.oPts=oPts.x; return output; }\n";
    return source.str();
}

struct BlobRelease { void operator()(ID3DBlob* blob) const { if (blob) blob->Release(); } };

static std::string RemapFogSemantic(const char* source) {
    std::string remapped(source != nullptr ? source : "");
    size_t offset = 0;
    while ((offset = remapped.find(": FOG", offset)) != std::string::npos) {
        remapped.replace(offset, 5, ": TEXCOORD6");
        offset += 11;
    }
    return remapped;
}

typedef HRESULT (WINAPI *D3DCompileProc)(LPCVOID, SIZE_T, LPCSTR, const D3D_SHADER_MACRO*,
                                         ID3DInclude*, LPCSTR, LPCSTR, UINT, UINT,
                                         ID3DBlob**, ID3DBlob**);

static D3DCompileProc GetCompiler() {
    static D3DCompileProc compiler = nullptr;
    static bool attempted = false;
    if (!attempted) {
        attempted = true;
        HMODULE module = LoadLibraryW(L"d3dcompiler_47.dll");
        if (module == nullptr)
            module = LoadLibraryW(L"d3dcompiler_43.dll");
        if (module != nullptr)
            compiler = reinterpret_cast<D3DCompileProc>(GetProcAddress(module, "D3DCompile"));
    }
    return compiler;
}

static std::string PsInputMapping(const std::string& value, unsigned int mapping) {
    switch (mapping & 0xe0u) {
    case 0x00u: return "max(" + value + ", 0.0)";
    case 0x20u: return "(1.0 - clamp(" + value + ", 0.0, 1.0))";
    case 0x40u: return "(2.0 * max(" + value + ", 0.0) - 1.0)";
    case 0x60u: return "(1.0 - 2.0 * max(" + value + ", 0.0))";
    case 0x80u: return "(max(" + value + ", 0.0) - 0.5)";
    case 0xa0u: return "(0.5 - max(" + value + ", 0.0))";
    case 0xc0u: return value;
    case 0xe0u: return "(-(" + value + "))";
    default: return value;
    }
}

static std::string PsOutputMapping(const std::string& value, unsigned int mapping) {
    switch (mapping & 0x38u) {
    case 0x00u: return value;
    case 0x08u: return "(" + value + " - 0.5)";
    case 0x10u: return "(" + value + " * 2.0)";
    case 0x18u: return "((" + value + " - 0.5) * 2.0)";
    case 0x20u: return "(" + value + " * 4.0)";
    case 0x30u: return "(" + value + " / 2.0)";
    default: return value;
    }
}

static std::string PsPackedColorExpression(unsigned int argb) {
    const unsigned int red = (argb >> 16) & 0xffu;
    const unsigned int green = (argb >> 8) & 0xffu;
    const unsigned int blue = argb & 0xffu;
    const unsigned int alpha = (argb >> 24) & 0xffu;
    return "float4(" + std::to_string(red) + ".0/255.0," +
           std::to_string(green) + ".0/255.0," +
           std::to_string(blue) + ".0/255.0," +
           std::to_string(alpha) + ".0/255.0)";
}

static std::string PsRegisterSource(unsigned int registerId, bool rgb,
                                    unsigned int finalSettings,
                                    const std::string& finalE,
                                    const std::string& finalF,
                                    const PixelShaderDefLayout* layout,
                                    unsigned int stage, unsigned int combinerFlags) {
    switch (registerId & 0x0fu) {
    case 0: return "float4(0.0,0.0,0.0,0.0)";
    case 1: {
        const unsigned int constantStage = stage >= 8u ? 8u
            : ((combinerFlags & 0x10u) != 0 ? stage : 0u);
        const unsigned int value = constantStage >= 8u
            ? layout->PSFinalCombinerConstant0 : layout->PSConstant0[constantStage];
        return PsPackedColorExpression(value);
    }
    case 2: {
        const unsigned int constantStage = stage >= 8u ? 8u
            : ((combinerFlags & 0x100u) != 0 ? stage : 0u);
        const unsigned int value = constantStage >= 8u
            ? layout->PSFinalCombinerConstant1 : layout->PSConstant1[constantStage];
        return PsPackedColorExpression(value);
    }
    case 3: return rgb ? "float4(fogColor.rgb, fogFactor)" : "float4(fogColor.rgb, fogFactor)";
    case 4: return "v0";
    case 5: return "v1";
    case 8: return "t0";
    case 9: return "t1";
    case 10: return "t2";
    case 11: return "t3";
    case 12: return "r0";
    case 13: return "r1";
    case 14: {
        const std::string v1 = (finalSettings & 0x40u) != 0 ? "(1.0 - v1)" : "v1";
        const std::string r0 = (finalSettings & 0x20u) != 0 ? "(1.0 - r0)" : "r0";
        const std::string sum = "(" + v1 + " + " + r0 + ")";
        const std::string mapped = (finalSettings & 0x80u) != 0
            ? "clamp(" + sum + ", 0.0, 1.0)" : sum;
        // NV2A exposes V1+R0 as an RGB-only value; its blue/alpha lanes are
        // zero in the reference final-combiner register.
        return "float4((" + mapped + ").rgb, 0.0)";
    }
    case 15:
        return rgb ? "float4((" + finalE + ") * (" + finalF + "),0.0)"
                   : "float4(0.0,0.0,0.0,0.0)";
    default: return "float4(0.0,0.0,0.0,0.0)";
    }
}

static std::string PsInputExpression(unsigned int encoded, bool rgb,
                                     unsigned int finalSettings,
                                     const std::string& finalE,
                                     const std::string& finalF,
                                     const PixelShaderDefLayout* layout,
                                     unsigned int stage, unsigned int combinerFlags) {
    const unsigned int registerId = encoded & 0x0fu;
    const bool alphaChannel = (encoded & 0x10u) != 0;
    const std::string source = PsRegisterSource(registerId, rgb, finalSettings, finalE, finalF,
                                                layout, stage, combinerFlags);
    std::string channel;
    if (rgb)
        channel = alphaChannel ? source + ".aaa" : source + ".rgb";
    else
        channel = alphaChannel ? source + ".a" : source + ".b";
    return PsInputMapping(channel, encoded & 0xe0u);
}

static bool PsCombinerTextureModesSupported(const PixelShaderDefLayout* layout) {
    const unsigned int count = layout->PSCombinerCount & 0xffu;
    if (count == 0 || count > 8)
        return false;
    for (unsigned int i = 0; i < 4; ++i) {
        const unsigned int mode = (layout->PSTextureModes >> (i * 5)) & 0x1fu;
        // These are the modes whose sampler and coordinate semantics are
        // completely described by _D3DPixelShaderDef: none, projective 2D,
        // and passthrough.  Cube/3D/bump modes need texture-object metadata
        // that this Xbox API call does not carry.
        if (mode != 0u && mode != 1u && mode != 4u)
            return false;
    }
    return true;
}

static unsigned int PsOutputDestination(unsigned int value, unsigned int shift) {
    return (value >> shift) & 0x0fu;
}

static void PsAppendDestination(std::ostringstream& source, unsigned int destination,
                                const std::string& value, bool rgb, bool blueToAlpha,
                                const std::string& name) {
    if (destination != 12u && destination != 13u)
        return;
    source << "  " << name << (rgb ? ".rgb = " : ".a = ") << value << ";\n";
    if (rgb && blueToAlpha)
        source << "  " << name << ".a = " << value << ".b;\n";
}

static void PsAppendStage(std::ostringstream& source, unsigned int index,
                          unsigned int inputValue, unsigned int outputValue,
                          bool rgb, unsigned int combinerFlags,
                          const std::string& finalE,
                          const std::string& finalF,
                          const PixelShaderDefLayout* layout) {
    const unsigned int aValue = (inputValue >> 24) & 0xffu;
    const unsigned int bValue = (inputValue >> 16) & 0xffu;
    const unsigned int cValue = (inputValue >> 8) & 0xffu;
    const unsigned int dValue = inputValue & 0xffu;
    const std::string a = PsInputExpression(aValue, rgb, 0, finalE, finalF,
                                            layout, index, combinerFlags);
    const std::string b = PsInputExpression(bValue, rgb, 0, finalE, finalF,
                                            layout, index, combinerFlags);
    const std::string c = PsInputExpression(cValue, rgb, 0, finalE, finalF,
                                            layout, index, combinerFlags);
    const std::string d = PsInputExpression(dValue, rgb, 0, finalE, finalF,
                                            layout, index, combinerFlags);
    const unsigned int flags = outputValue >> 12;
    const unsigned int mapping = flags & 0x38u;
    const std::string ab = (flags & 2u) != 0
        ? (rgb ? "float3(dot(" + a + "," + b + "))" : "(" + a + " * " + b + ")")
        : "(" + a + " * " + b + ")";
    const std::string cd = (flags & 1u) != 0
        ? (rgb ? "float3(dot(" + c + "," + d + "))" : "(" + c + " * " + d + ")")
        : "(" + c + " * " + d + ")";
    const std::string muxCondition = (combinerFlags & 1u) != 0
        ? "(r0.a >= 0.5)"
        : "(fmod(floor(r0.a * 255.0), 2.0) >= 1.0)";
    const std::string mux = (flags & 4u) != 0
        ? "((" + muxCondition + ") ? (" + cd + ") : (" + ab + "))"
        : "(" + ab + " + " + cd + ")";
    const std::string abName = "nv2aAb" + std::to_string(index) + (rgb ? "Rgb" : "Alpha");
    const std::string cdName = "nv2aCd" + std::to_string(index) + (rgb ? "Rgb" : "Alpha");
    const std::string muxName = "nv2aMux" + std::to_string(index) + (rgb ? "Rgb" : "Alpha");
    source << "  " << (rgb ? "float3 " : "float ") << abName << " = clamp("
           << PsOutputMapping(ab, mapping) << ", -1.0, 1.0);\n"
           << "  " << (rgb ? "float3 " : "float ") << cdName << " = clamp("
           << PsOutputMapping(cd, mapping) << ", -1.0, 1.0);\n"
           << "  " << (rgb ? "float3 " : "float ") << muxName << " = clamp("
           << PsOutputMapping(mux, mapping) << ", -1.0, 1.0);\n";
    const unsigned int abDestination = PsOutputDestination(outputValue, 4);
    const unsigned int cdDestination = PsOutputDestination(outputValue, 0);
    const unsigned int muxDestination = PsOutputDestination(outputValue, 8);
    PsAppendDestination(source, abDestination, abName, rgb,
                        rgb && (flags & 0x80u) != 0,
                        abDestination == 13u ? "r1" : "r0");
    PsAppendDestination(source, cdDestination, cdName, rgb,
                        rgb && (flags & 0x40u) != 0,
                        cdDestination == 13u ? "r1" : "r0");
    PsAppendDestination(source, muxDestination, muxName, rgb, false,
                        muxDestination == 13u ? "r1" : "r0");
}

static std::string BuildNV2ACombinerHlsl(const PixelShaderDefLayout* layout) {
    if (!PsCombinerTextureModesSupported(layout))
        return std::string();
    const unsigned int count = layout->PSCombinerCount & 0xffu;
    const unsigned int combinerFlags = layout->PSCombinerCount >> 8;
    std::ostringstream source;
    source << "struct PSIn { float4 d0 : COLOR0; float4 d1 : COLOR1;"
           << " float4 t0 : TEXCOORD0; float4 t1 : TEXCOORD1;"
           << " float4 t2 : TEXCOORD2; float4 t3 : TEXCOORD3; float fog : FOG; };\n"
           << "sampler2D s0 : register(s0); sampler2D s1 : register(s1);"
           << " sampler2D s2 : register(s2); sampler2D s3 : register(s3);\n"
           << "float4 fogColor : register(c0); float4 fogState : register(c31);\n"
           << "float nv2aFogFactor(float value) { return fogState.x < 0.5 ? 1.0 : saturate(value); }\n"
           << "float4 main(PSIn input) : COLOR0 {\n"
           << "  float4 v0=input.d0, v1=input.d1;\n"
           << "  float fogFactor=nv2aFogFactor(input.fog);\n"
           << "  float4 r0=0, r1=0;\n";
    for (unsigned int i = 0; i < 4; ++i) {
        const unsigned int mode = (layout->PSTextureModes >> (i * 5)) & 0x1fu;
        if (mode == 0u)
            source << "  float4 t" << i << "=float4(0,0,0,1);\n";
        else if (mode == 1u)
            source << "  float4 t" << i << "=tex2D(s" << i
                   << ", input.t" << i << ".xy / max(abs(input.t" << i
                   << ".w), 1e-20));\n";
        else
            source << "  float4 t" << i << "=input.t" << i << ";\n";
    }
    const bool hasFinal = layout->PSFinalCombinerInputsABCD != 0u ||
                          layout->PSFinalCombinerInputsEFG != 0u;
    std::string finalE;
    std::string finalF;
    if (hasFinal) {
        const unsigned int e = (layout->PSFinalCombinerInputsEFG >> 24) & 0xffu;
        const unsigned int f = (layout->PSFinalCombinerInputsEFG >> 16) & 0xffu;
        finalE = PsInputExpression(e, true, layout->PSFinalCombinerInputsEFG & 0xffu, "", "",
                                   layout, 8u, combinerFlags);
        finalF = PsInputExpression(f, true, layout->PSFinalCombinerInputsEFG & 0xffu, "", "",
                                   layout, 8u, combinerFlags);
    }
    for (unsigned int i = 0; i < count; ++i) {
        PsAppendStage(source, i, layout->PSRGBInputs[i], layout->PSRGBOutputs[i], true,
                      combinerFlags, finalE, finalF, layout);
        PsAppendStage(source, i, layout->PSAlphaInputs[i], layout->PSAlphaOutputs[i], false,
                      combinerFlags, finalE, finalF, layout);
    }
    if (hasFinal) {
        const unsigned int abcd = layout->PSFinalCombinerInputsABCD;
        const unsigned int efg = layout->PSFinalCombinerInputsEFG;
        const unsigned int settings = efg & 0xffu;
        const std::string a = PsInputExpression((abcd >> 24) & 0xffu, true, settings, finalE, finalF,
                                                layout, 8u, combinerFlags);
        const std::string b = PsInputExpression((abcd >> 16) & 0xffu, true, settings, finalE, finalF,
                                                layout, 8u, combinerFlags);
        const std::string c = PsInputExpression((abcd >> 8) & 0xffu, true, settings, finalE, finalF,
                                                layout, 8u, combinerFlags);
        const std::string d = PsInputExpression(abcd & 0xffu, true, settings, finalE, finalF,
                                                layout, 8u, combinerFlags);
        const std::string g = PsInputExpression((efg >> 8) & 0xffu, false, settings, finalE, finalF,
                                                layout, 8u, combinerFlags);
        source << "  return float4(" << d << " + lerp(" << c << "," << b << "," << a << "), " << g << ");\n";
    } else {
        source << "  return r0;\n";
    }
    source << "}\n";
    return source.str();
}

} // namespace

IDirect3DVertexShader9* nullD3DCompileNV2AVertexShader(
    IDirect3DDevice9* device, const unsigned int* microcode) {
    if (device == nullptr || microcode == nullptr)
        return nullptr;
    const unsigned int* program = ResolveProgram(microcode);
    if (program == nullptr)
        return nullptr;
    static std::map<const unsigned int*, IDirect3DVertexShader9*> cache;
    const auto found = cache.find(program);
    if (found != cache.end())
        return found->second;
    std::string hlsl = BuildHlsl(program);
    hlsl = RemapFogSemantic(hlsl.c_str());
    D3DCompileProc compiler = GetCompiler();
    if (hlsl.empty()) {
        return nullptr;
    }
    if (compiler == nullptr) {
        return nullptr;
    }
    ID3DBlob* bytecode = nullptr;
    ID3DBlob* errors = nullptr;
    const HRESULT result = compiler(hlsl.data(), hlsl.size(), "nv2a", nullptr, nullptr,
                                    "main", "vs_3_0", 0, 0, &bytecode, &errors);
    if (FAILED(result)) {
        if (errors != nullptr)
            OutputDebugStringA((const char*)errors->GetBufferPointer());
        if (errors != nullptr)
            errors->Release();
        return nullptr;
    }
    if (errors != nullptr)
        errors->Release();
    IDirect3DVertexShader9* shader = nullptr;
    if (FAILED(device->CreateVertexShader((const DWORD*)bytecode->GetBufferPointer(), &shader)))
        shader = nullptr;
    bytecode->Release();
    if (shader != nullptr)
        cache[program] = shader;
    return shader;
}

bool nullD3DProgramUsesHomogeneousDivide(const unsigned int* microcode) {
    const unsigned int* program = ResolveProgram(microcode);
    return program != nullptr && ProgramUsesHomogeneousDivide(program);
}

IDirect3DPixelShader9* nullD3DCompileNV2AFallbackPixelShader(
    IDirect3DDevice9* device, unsigned int textureMask) {
    if (device == nullptr)
        return nullptr;
    static std::map<unsigned int, IDirect3DPixelShader9*> cache;
    const auto found = cache.find(textureMask & 3u);
    if (found != cache.end())
        return found->second;

    std::ostringstream source;
    source << "struct PSIn { float4 d0 : COLOR0; float4 t0 : TEXCOORD0;"
           << " float4 t1 : TEXCOORD1; };\n";
    if ((textureMask & 1u) != 0)
        source << "sampler2D s0 : register(s0);\n";
    if ((textureMask & 2u) != 0)
        source << "sampler2D s1 : register(s1);\n";
    source << "float4 main(PSIn input) : COLOR0 {\n"
           << "  float4 color = input.d0;\n";
    if ((textureMask & 1u) != 0)
        source << "  color *= tex2D(s0, input.t0.xy / max(abs(input.t0.w), 1e-20));\n";
    if ((textureMask & 2u) != 0)
        source << "  color *= tex2D(s1, input.t1.xy / max(abs(input.t1.w), 1e-20));\n";
    source << "  return color;\n}\n";

    D3DCompileProc compiler = GetCompiler();
    if (compiler == nullptr)
        return nullptr;
    ID3DBlob* bytecode = nullptr;
    ID3DBlob* errors = nullptr;
    const std::string hlsl = source.str();
    const HRESULT result = compiler(hlsl.data(), hlsl.size(), "nv2a_fallback", nullptr,
                                    nullptr, "main", "ps_3_0", 0, 0,
                                    &bytecode, &errors);
    if (FAILED(result)) {
        if (errors != nullptr)
            OutputDebugStringA((const char*)errors->GetBufferPointer());
        if (errors != nullptr)
            errors->Release();
        return nullptr;
    }
    if (errors != nullptr)
        errors->Release();
    IDirect3DPixelShader9* shader = nullptr;
    if (FAILED(device->CreatePixelShader((const DWORD*)bytecode->GetBufferPointer(), &shader)))
        shader = nullptr;
    bytecode->Release();
    if (shader != nullptr)
        cache[textureMask & 3u] = shader;
    return shader;
}

IDirect3DPixelShader9* nullD3DCompileNV2AWorldPixelShader(
    IDirect3DDevice9* device, const _D3DPixelShaderDef* definition) {
    const PixelShaderDefLayout* layout =
        reinterpret_cast<const PixelShaderDefLayout*>(definition);
    if (device == nullptr || layout == nullptr)
        return nullptr;

    const bool isWorldLightmap =
        layout->PSCombinerCount == 0x00011102u &&
        layout->PSTextureModes == 0x00000021u &&
        layout->PSAlphaInputs[0] == 0xd9d41010u &&
        layout->PSAlphaInputs[1] == 0xd8301010u &&
        layout->PSRGBInputs[1] == 0xc83d0000u;
    const bool isWorldTextured =
        layout->PSCombinerCount == 0x00011101u &&
        layout->PSTextureModes == 0x00000001u &&
        layout->PSAlphaInputs[0] == 0xd8301010u &&
        layout->PSRGBInputs[0] == 0xc8200000u &&
        layout->PSRGBOutputs[0] == 0x000000c0u &&
        layout->PSAlphaOutputs[0] == 0x000000c0u;
    const bool isColor =
        layout->PSCombinerCount == 0x00011101u &&
        layout->PSTextureModes == 0x00000000u &&
        layout->PSAlphaInputs[0] == 0xd4301010u &&
        layout->PSRGBInputs[0] == 0xc4200000u &&
        layout->PSRGBOutputs[0] == 0x000000c0u &&
        layout->PSAlphaOutputs[0] == 0x000000c0u;
    const bool isSky =
        layout->PSCombinerCount == 0x00011101u &&
        layout->PSTextureModes == 0x00000001u &&
        layout->PSAlphaInputs[0] == 0xd8d41010u &&
        layout->PSRGBInputs[0] == 0xc8c40000u &&
        layout->PSRGBOutputs[0] == 0x000000c0u &&
        layout->PSAlphaOutputs[0] == 0x000000c0u;
    const bool isTexturedVertexColored =
        layout->PSCombinerCount == 0x00011101u &&
        layout->PSTextureModes == 0x00000001u &&
        layout->PSAlphaInputs[0] == 0xd8301010u &&
        layout->PSRGBInputs[0] == 0xc8c40000u &&
        layout->PSRGBOutputs[0] == 0x000000c0u &&
        layout->PSAlphaOutputs[0] == 0x000000c0u;
    const bool isPrelitLightmap =
        layout->PSCombinerCount == 0x00011102u &&
        layout->PSTextureModes == 0x00000021u &&
        layout->PSAlphaInputs[0] == 0x00000000u &&
        layout->PSAlphaInputs[1] == 0xd8301010u &&
        layout->PSRGBInputs[0] == 0xc8c40000u &&
        layout->PSRGBInputs[1] == 0xccd90000u &&
        layout->PSRGBOutputs[0] == 0x000000c0u &&
        layout->PSRGBOutputs[1] == 0x000000c0u &&
        layout->PSAlphaOutputs[1] == 0x000000c0u;
    const bool isPointLitProjected =
        layout->PSCombinerCount == 0x00011102u &&
        layout->PSTextureModes == 0x00000021u &&
        layout->PSAlphaInputs[0] == 0xd8d41010u &&
        layout->PSAlphaInputs[1] == 0xd9d530dcu &&
        layout->PSRGBInputs[0] == 0xc8c40000u &&
        layout->PSRGBInputs[1] == 0xc9c520ccu &&
        layout->PSRGBOutputs[0] == 0x000000c0u &&
        layout->PSRGBOutputs[1] == 0x00000c00u &&
        layout->PSAlphaOutputs[0] == 0x000000c0u &&
        layout->PSAlphaOutputs[1] == 0x00000c00u;
    const bool isPointLitLightmap =
        layout->PSCombinerCount == 0x00011103u &&
        layout->PSTextureModes == 0x00000021u &&
        layout->PSAlphaInputs[0] == 0xd9d41010u &&
        layout->PSAlphaInputs[2] == 0xd8301010u &&
        layout->PSRGBInputs[1] == 0xc8c40000u &&
        layout->PSRGBInputs[2] == 0xcc3d0000u &&
        layout->PSRGBOutputs[1] == 0x000000c0u &&
        layout->PSRGBOutputs[2] == 0x000000c0u &&
        layout->PSAlphaOutputs[0] == 0x000000d0u &&
        layout->PSAlphaOutputs[2] == 0x000000c0u;
    const bool isPointLitAdditive =
        layout->PSCombinerCount == 0x00011102u &&
        layout->PSTextureModes == 0x00000021u &&
        layout->PSAlphaInputs[0] == 0x00000000u &&
        layout->PSAlphaInputs[1] == 0xd8301010u &&
        layout->PSRGBInputs[0] == 0xc8c40000u &&
        layout->PSRGBInputs[1] == 0xc8c920ccu &&
        layout->PSRGBOutputs[0] == 0x000000c0u &&
        layout->PSRGBOutputs[1] == 0x00000c00u &&
        layout->PSAlphaOutputs[0] == 0x000000c0u &&
        layout->PSAlphaOutputs[1] == 0x000000c0u;
    const bool isPointLitAdditiveLightmap =
        layout->PSCombinerCount == 0x00011104u &&
        layout->PSTextureModes == 0x00000421u &&
        layout->PSAlphaInputs[2] == 0xdad41010u &&
        layout->PSAlphaInputs[3] == 0xd8301010u &&
        layout->PSRGBInputs[0] == 0xc8c40000u &&
        layout->PSRGBInputs[1] == 0xc8c920ccu &&
        layout->PSRGBInputs[3] == 0xcc3d0000u &&
        layout->PSRGBOutputs[0] == 0x000000c0u &&
        layout->PSRGBOutputs[1] == 0x00000c00u &&
        layout->PSRGBOutputs[3] == 0x000000c0u &&
        layout->PSAlphaOutputs[2] == 0x000000d0u &&
        layout->PSAlphaOutputs[3] == 0x000000c0u;
    const bool isWorldTexturedVertexLit =
        layout->PSCombinerCount == 0x00011102u &&
        layout->PSTextureModes == 0x00000001u &&
        layout->PSAlphaInputs[0] == 0x00000000u &&
        layout->PSAlphaInputs[1] == 0xd8301010u &&
        layout->PSRGBInputs[0] == 0xc8200000u &&
        layout->PSRGBInputs[1] == 0xccc40000u &&
        layout->PSRGBOutputs[0] == 0x000000c0u &&
        layout->PSRGBOutputs[1] == 0x000000c0u &&
        layout->PSAlphaOutputs[1] == 0x000000c0u;
    const bool isWorldLightmapVertexLit =
        layout->PSCombinerCount == 0x00011103u &&
        layout->PSTextureModes == 0x00000021u &&
        layout->PSAlphaInputs[0] == 0xd9d41010u &&
        layout->PSAlphaInputs[1] == 0x00000000u &&
        layout->PSAlphaInputs[2] == 0xd8301010u &&
        layout->PSRGBInputs[1] == 0xc83d0000u &&
        layout->PSRGBInputs[2] == 0xccc40000u &&
        layout->PSRGBOutputs[1] == 0x000000c0u &&
        layout->PSRGBOutputs[2] == 0x000000c0u &&
        layout->PSAlphaOutputs[2] == 0x000000c0u;
    const bool isWorldTwoTexture =
        layout->PSCombinerCount == 0x00011101u &&
        layout->PSTextureModes == 0x00000021u &&
        layout->PSAlphaInputs[0] == 0xd8301010u &&
        layout->PSRGBInputs[0] == 0xc8c90000u &&
        layout->PSRGBOutputs[0] == 0x000000c0u &&
        layout->PSAlphaOutputs[0] == 0x000000c0u;
    const bool isWorldTwoTextureVertexLit =
        layout->PSCombinerCount == 0x00011102u &&
        layout->PSTextureModes == 0x00000021u &&
        layout->PSAlphaInputs[0] == 0x00000000u &&
        layout->PSAlphaInputs[1] == 0xd8301010u &&
        layout->PSRGBInputs[0] == 0xc8c90000u &&
        layout->PSRGBInputs[1] == 0xccc40000u &&
        layout->PSRGBOutputs[0] == 0x000000c0u &&
        layout->PSRGBOutputs[1] == 0x000000c0u &&
        layout->PSAlphaOutputs[1] == 0x000000c0u;
    const bool isWorldBlend =
        layout->PSCombinerCount == 0x00011102u &&
        layout->PSTextureModes == 0x00000021u &&
        layout->PSAlphaInputs[0] == 0x00000000u &&
        layout->PSAlphaInputs[1] == 0xd8301010u &&
        layout->PSRGBInputs[0] == 0x14c9c834u &&
        layout->PSRGBInputs[1] == 0xccc40000u &&
        layout->PSRGBOutputs[0] == 0x00000c00u &&
        layout->PSRGBOutputs[1] == 0x000000c0u &&
        layout->PSAlphaOutputs[1] == 0x000000c0u;
    const bool isWorldBlendLightmap =
        layout->PSCombinerCount == 0x00011103u &&
        layout->PSTextureModes == 0x00000421u &&
        layout->PSAlphaInputs[0] == 0x00000000u &&
        layout->PSAlphaInputs[1] == 0x00000000u &&
        layout->PSAlphaInputs[2] == 0xd8301010u &&
        layout->PSRGBInputs[0] == 0x14c9c834u &&
        layout->PSRGBInputs[1] == 0xccc40000u &&
        layout->PSRGBInputs[2] == 0xccca0000u &&
        layout->PSRGBOutputs[0] == 0x00000c00u &&
        layout->PSRGBOutputs[1] == 0x000000c0u &&
        layout->PSRGBOutputs[2] == 0x000000c0u &&
        layout->PSAlphaOutputs[2] == 0x000000c0u;
    const bool isWorldBlendAlphaLightmap =
        layout->PSCombinerCount == 0x00011104u &&
        layout->PSTextureModes == 0x00008021u &&
        layout->PSAlphaInputs[2] == 0xdbc51010u &&
        layout->PSAlphaInputs[3] == 0xd8301010u &&
        layout->PSRGBInputs[0] == 0x14c9c834u &&
        layout->PSRGBInputs[1] == 0xccc40000u &&
        layout->PSRGBInputs[3] == 0xcc3d0000u &&
        layout->PSRGBOutputs[0] == 0x00000c00u &&
        layout->PSRGBOutputs[1] == 0x000000c0u &&
        layout->PSRGBOutputs[3] == 0x000000c0u &&
        layout->PSAlphaOutputs[2] == 0x000000d0u &&
        layout->PSAlphaOutputs[3] == 0x000000c0u;
    const bool isWorldBlendRgbAndAlphaLightmap =
        layout->PSCombinerCount == 0x00011105u &&
        layout->PSTextureModes == 0x00008421u &&
        layout->PSAlphaInputs[3] == 0xdbc51010u &&
        layout->PSAlphaInputs[4] == 0xd8301010u &&
        layout->PSRGBInputs[0] == 0x14c9c834u &&
        layout->PSRGBInputs[1] == 0xccc40000u &&
        layout->PSRGBInputs[2] == 0xccca0000u &&
        layout->PSRGBInputs[4] == 0xcc3d0000u &&
        layout->PSRGBOutputs[0] == 0x00000c00u &&
        layout->PSRGBOutputs[1] == 0x000000c0u &&
        layout->PSRGBOutputs[2] == 0x000000c0u &&
        layout->PSRGBOutputs[4] == 0x000000c0u &&
        layout->PSAlphaOutputs[3] == 0x000000d0u &&
        layout->PSAlphaOutputs[4] == 0x000000c0u;
    const bool isWorldBlendPointLit =
        layout->PSCombinerCount == 0x00011102u &&
        layout->PSTextureModes == 0x00000021u &&
        layout->PSAlphaInputs[0] == 0x00000000u &&
        layout->PSAlphaInputs[1] == 0xd8301010u &&
        layout->PSRGBInputs[0] == 0x14c9c834u &&
        layout->PSRGBInputs[1] == 0xccc50000u &&
        layout->PSRGBOutputs[0] == 0x00000c00u &&
        layout->PSRGBOutputs[1] == 0x000000c0u &&
        layout->PSAlphaOutputs[1] == 0x000000c0u;
    const bool isWorldBlendPointLitAlphaLightmap =
        layout->PSCombinerCount == 0x00011104u &&
        layout->PSTextureModes == 0x00008021u &&
        layout->PSAlphaInputs[2] == 0xdbc41010u &&
        layout->PSAlphaInputs[3] == 0xd8301010u &&
        layout->PSRGBInputs[0] == 0x14c9c834u &&
        layout->PSRGBInputs[1] == 0xccc50000u &&
        layout->PSRGBInputs[3] == 0xcc3d0000u &&
        layout->PSRGBOutputs[0] == 0x00000c00u &&
        layout->PSRGBOutputs[1] == 0x000000c0u &&
        layout->PSRGBOutputs[3] == 0x000000c0u &&
        layout->PSAlphaOutputs[2] == 0x000000d0u &&
        layout->PSAlphaOutputs[3] == 0x000000c0u;
    const bool isWorldBlendPointLitLightmap =
        layout->PSCombinerCount == 0x00011103u &&
        layout->PSTextureModes == 0x00000421u &&
        layout->PSAlphaInputs[2] == 0xd8301010u &&
        layout->PSRGBInputs[0] == 0x14c9c834u &&
        layout->PSRGBInputs[1] == 0xccc50000u &&
        layout->PSRGBInputs[2] == 0xccca20cdu &&
        layout->PSRGBOutputs[0] == 0x00000c00u &&
        layout->PSRGBOutputs[1] == 0x000000d0u &&
        layout->PSRGBOutputs[2] == 0x00000c00u &&
        layout->PSAlphaOutputs[2] == 0x000000c0u;
    const bool isWorldBlendPointLitRgbAndAlphaLightmap =
        layout->PSCombinerCount == 0x00011105u &&
        layout->PSTextureModes == 0x00008421u &&
        layout->PSAlphaInputs[3] == 0xdbc41010u &&
        layout->PSRGBInputs[0] == 0x14c9c834u &&
        layout->PSRGBInputs[1] == 0xccc50000u &&
        layout->PSRGBInputs[2] == 0xccca20cdu &&
        layout->PSRGBInputs[4] == 0xcc3d0000u &&
        layout->PSRGBOutputs[0] == 0x00000c00u &&
        layout->PSRGBOutputs[1] == 0x000000d0u &&
        layout->PSRGBOutputs[2] == 0x00000c00u &&
        layout->PSRGBOutputs[4] == 0x000000c0u &&
        layout->PSAlphaOutputs[3] == 0x000000d0u;
    if (!isWorldLightmap && !isWorldTextured && !isColor && !isSky && !isTexturedVertexColored &&
        !isPrelitLightmap && !isPointLitProjected &&
        !isPointLitLightmap &&
        !isPointLitAdditive && !isPointLitAdditiveLightmap &&
        !isWorldTexturedVertexLit &&
        !isWorldTwoTexture && !isWorldTwoTextureVertexLit &&
        !isWorldLightmapVertexLit && !isWorldBlend && !isWorldBlendLightmap &&
        !isWorldBlendAlphaLightmap && !isWorldBlendRgbAndAlphaLightmap &&
        !isWorldBlendPointLit && !isWorldBlendPointLitAlphaLightmap &&
        !isWorldBlendPointLitLightmap && !isWorldBlendPointLitRgbAndAlphaLightmap)
        return nullptr;

    static const char WorldSource[] =
        "struct PSIn { float4 d0 : COLOR0; float4 t0 : TEXCOORD0; "
        "float4 t1 : TEXCOORD1; float fog : FOG; };\n"
        "sampler2D s0 : register(s0); sampler2D s1 : register(s1);\n"
        "float4 fogColor : register(c0);\n"
        "float4 main(PSIn input) : COLOR0 {\n"
        "  float4 diffuse = tex2D(s0, input.t0.xy / max(abs(input.t0.w), 1e-20));\n"
        "  float4 lightmap = tex2D(s1, input.t1.xy / max(abs(input.t1.w), 1e-20));\n"
        "  float light = lightmap.a * input.d0.a;\n"
        "  float3 rgb = diffuse.rgb * (1.0 - light);\n"
        "  return float4(lerp(fogColor.rgb, rgb, saturate(input.fog)), diffuse.a);\n"
        "}\n";
    static const char WorldTextureSource[] =
        "struct PSIn { float4 d0 : COLOR0; float4 t0 : TEXCOORD0; float fog : FOG; };\n"
        "sampler2D s0 : register(s0);\n"
        "float4 fogColor : register(c0);\n"
        "float4 main(PSIn input) : COLOR0 {\n"
        "  float4 diffuse = tex2D(s0, input.t0.xy / max(abs(input.t0.w), 1e-20));\n"
        "  return float4(lerp(fogColor.rgb, diffuse.rgb, saturate(input.fog)), diffuse.a);\n"
        "}\n";
    static const char SkySource[] =
        "struct PSIn { float4 d0 : COLOR0; float4 t0 : TEXCOORD0; float fog : FOG; };\n"
        "sampler2D s0 : register(s0);\n"
        "float4 fogColor : register(c0);\n"
        "float4 main(PSIn input) : COLOR0 {\n"
        "  float4 diffuse = tex2D(s0, input.t0.xy / max(abs(input.t0.w), 1e-20));\n"
        "  float3 rgb = diffuse.rgb * input.d0.rgb;\n"
        "  float alpha = diffuse.a * input.d0.a;\n"
        "  return float4(lerp(fogColor.rgb, rgb, saturate(input.fog)), alpha);\n"
        "}\n";
    static const char WorldTextureVertexLitSource[] =
        "struct PSIn { float4 d0 : COLOR0; float4 t0 : TEXCOORD0; float fog : FOG; };\n"
        "sampler2D s0 : register(s0);\n"
        "float4 fogColor : register(c0);\n"
        "float4 main(PSIn input) : COLOR0 {\n"
        "  float4 diffuse = tex2D(s0, input.t0.xy / max(abs(input.t0.w), 1e-20));\n"
        "  float3 rgb = diffuse.rgb * input.d0.rgb;\n"
        "  return float4(lerp(fogColor.rgb, rgb, saturate(input.fog)), diffuse.a);\n"
        "}\n";
    static const char PrelitLightmapSource[] =
        "struct PSIn { float4 d0 : COLOR0; float4 t0 : TEXCOORD0; "
        "float4 t1 : TEXCOORD1; float fog : FOG; };\n"
        "sampler2D s0 : register(s0); sampler2D s1 : register(s1);\n"
        "float4 fogColor : register(c0);\n"
        "float4 main(PSIn input) : COLOR0 {\n"
        "  float4 diffuse = tex2D(s0, input.t0.xy / max(abs(input.t0.w), 1e-20));\n"
        "  float4 lightmap = tex2D(s1, input.t1.xy / max(abs(input.t1.w), 1e-20));\n"
        "  float3 rgb = diffuse.rgb * input.d0.rgb * lightmap.a;\n"
        "  return float4(lerp(fogColor.rgb, rgb, saturate(input.fog)), diffuse.a);\n"
        "}\n";
    static const char PointLitProjectedSource[] =
        "struct PSIn { float4 d0 : COLOR0; float4 d1 : COLOR1; "
        "float4 t0 : TEXCOORD0; float4 t1 : TEXCOORD1; float fog : FOG; };\n"
        "sampler2D s0 : register(s0); sampler2D s1 : register(s1);\n"
        "float4 fogColor : register(c0);\n"
        "float4 main(PSIn input) : COLOR0 {\n"
        "  float4 diffuse = tex2D(s0, input.t0.xy / max(abs(input.t0.w), 1e-20));\n"
        "  float4 projected = tex2D(s1, input.t1.xy / max(abs(input.t1.w), 1e-20));\n"
        "  float3 rgb = diffuse.rgb * input.d0.rgb + projected.rgb * input.d1.rgb;\n"
        "  float alpha = diffuse.a * input.d0.a + projected.a * input.d1.a;\n"
        "  return float4(lerp(fogColor.rgb, rgb, saturate(input.fog)), alpha);\n"
        "}\n";
    static const char WorldLightmapVertexLitSource[] =
        "struct PSIn { float4 d0 : COLOR0; float4 t0 : TEXCOORD0; "
        "float4 t1 : TEXCOORD1; float fog : FOG; };\n"
        "sampler2D s0 : register(s0); sampler2D s1 : register(s1);\n"
        "float4 fogColor : register(c0);\n"
        "float4 main(PSIn input) : COLOR0 {\n"
        "  float4 diffuse = tex2D(s0, input.t0.xy / max(abs(input.t0.w), 1e-20));\n"
        "  float4 lightmap = tex2D(s1, input.t1.xy / max(abs(input.t1.w), 1e-20));\n"
        "  float3 rgb = diffuse.rgb * (1.0 - lightmap.a * input.d0.a);\n"
        "  rgb *= input.d0.rgb;\n"
        "  return float4(lerp(fogColor.rgb, rgb, saturate(input.fog)), diffuse.a);\n"
        "}\n";
    static const char PointLitAdditiveSource[] =
        "struct PSIn { float4 d0 : COLOR0; float4 t0 : TEXCOORD0; "
        "float4 t1 : TEXCOORD1; float fog : FOG; };\n"
        "sampler2D s0 : register(s0); sampler2D s1 : register(s1);\n"
        "float4 fogColor : register(c0);\n"
        "float4 main(PSIn input) : COLOR0 {\n"
        "  float4 diffuse = tex2D(s0, input.t0.xy / max(abs(input.t0.w), 1e-20));\n"
        "  float4 pointLight = tex2D(s1, input.t1.xy / max(abs(input.t1.w), 1e-20));\n"
        "  float3 rgb = diffuse.rgb * (input.d0.rgb + pointLight.rgb);\n"
        "  return float4(lerp(fogColor.rgb, rgb, saturate(input.fog)), diffuse.a);\n"
        "}\n";
    static const char PointLitAdditiveLightmapSource[] =
        "struct PSIn { float4 d0 : COLOR0; float4 t0 : TEXCOORD0; "
        "float4 t1 : TEXCOORD1; float4 t2 : TEXCOORD2; float fog : FOG; };\n"
        "sampler2D s0 : register(s0); sampler2D s1 : register(s1); "
        "sampler2D s2 : register(s2);\n"
        "float4 fogColor : register(c0);\n"
        "float4 main(PSIn input) : COLOR0 {\n"
        "  float4 diffuse = tex2D(s0, input.t0.xy / max(abs(input.t0.w), 1e-20));\n"
        "  float4 lightmap = tex2D(s1, input.t1.xy / max(abs(input.t1.w), 1e-20));\n"
        "  float4 projected = tex2D(s2, input.t2.xy / max(abs(input.t2.w), 1e-20));\n"
        "  float3 rgb = diffuse.rgb * (input.d0.rgb + lightmap.rgb);\n"
        "  rgb *= 1.0 - projected.a * input.d0.a;\n"
        "  return float4(lerp(fogColor.rgb, rgb, saturate(input.fog)), diffuse.a);\n"
        "}\n";
    static const char WorldTwoTextureSource[] =
        "struct PSIn { float4 d0 : COLOR0; float4 t0 : TEXCOORD0; "
        "float4 t1 : TEXCOORD1; float fog : FOG; };\n"
        "sampler2D s0 : register(s0); sampler2D s1 : register(s1);\n"
        "float4 fogColor : register(c0);\n"
        "float4 main(PSIn input) : COLOR0 {\n"
        "  float4 diffuse = tex2D(s0, input.t0.xy / max(abs(input.t0.w), 1e-20));\n"
        "  float4 secondary = tex2D(s1, input.t1.xy / max(abs(input.t1.w), 1e-20));\n"
        "  float3 rgb = diffuse.rgb * secondary.rgb;\n"
        "  return float4(lerp(fogColor.rgb, rgb, saturate(input.fog)), diffuse.a);\n"
        "}\n";
    static const char ColorSource[] =
        "struct PSIn { float4 d0 : COLOR0; float fog : FOG; };\n"
        "float4 fogColor : register(c0);\n"
        "float4 main(PSIn input) : COLOR0 {\n"
        "  return float4(lerp(fogColor.rgb, input.d0.rgb, saturate(input.fog)), input.d0.a);\n"
        "}\n";
    static const char WorldTwoTextureVertexLitSource[] =
        "struct PSIn { float4 d0 : COLOR0; float4 t0 : TEXCOORD0; "
        "float4 t1 : TEXCOORD1; float fog : FOG; };\n"
        "sampler2D s0 : register(s0); sampler2D s1 : register(s1);\n"
        "float4 fogColor : register(c0);\n"
        "float4 main(PSIn input) : COLOR0 {\n"
        "  float4 diffuse = tex2D(s0, input.t0.xy / max(abs(input.t0.w), 1e-20));\n"
        "  float4 secondary = tex2D(s1, input.t1.xy / max(abs(input.t1.w), 1e-20));\n"
        "  float3 rgb = diffuse.rgb * secondary.rgb * input.d0.rgb;\n"
        "  return float4(lerp(fogColor.rgb, rgb, saturate(input.fog)), diffuse.a);\n"
        "}\n";
    static const char WorldBlendSource[] =
        "struct PSIn { float4 d0 : COLOR0; float4 t0 : TEXCOORD0; "
        "float4 t1 : TEXCOORD1; float fog : FOG; };\n"
        "sampler2D s0 : register(s0); sampler2D s1 : register(s1);\n"
        "float4 fogColor : register(c0);\n"
        "float4 main(PSIn input) : COLOR0 {\n"
        "  float4 diffuse = tex2D(s0, input.t0.xy / max(abs(input.t0.w), 1e-20));\n"
        "  float4 blend = tex2D(s1, input.t1.xy / max(abs(input.t1.w), 1e-20));\n"
        "  float3 rgb = lerp(diffuse.rgb, blend.rgb, saturate(input.d0.a));\n"
        "  rgb *= input.d0.rgb;\n"
        "  return float4(lerp(fogColor.rgb, rgb, saturate(input.fog)), diffuse.a);\n"
        "}\n";
    static const char WorldBlendLightmapSource[] =
        "struct PSIn { float4 d0 : COLOR0; float4 t0 : TEXCOORD0; "
        "float4 t1 : TEXCOORD1; float4 t2 : TEXCOORD2; float fog : FOG; };\n"
        "sampler2D s0 : register(s0); sampler2D s1 : register(s1); "
        "sampler2D s2 : register(s2);\n"
        "float4 fogColor : register(c0);\n"
        "float4 main(PSIn input) : COLOR0 {\n"
        "  float4 diffuse = tex2D(s0, input.t0.xy / max(abs(input.t0.w), 1e-20));\n"
        "  float4 blend = tex2D(s1, input.t1.xy / max(abs(input.t1.w), 1e-20));\n"
        "  float4 lightmap = tex2D(s2, input.t2.xy / max(abs(input.t2.w), 1e-20));\n"
        "  float3 rgb = lerp(diffuse.rgb, blend.rgb, saturate(input.d0.a));\n"
        "  rgb *= input.d0.rgb * lightmap.rgb;\n"
        "  return float4(lerp(fogColor.rgb, rgb, saturate(input.fog)), diffuse.a);\n"
        "}\n";
    static const char WorldBlendAlphaLightmapSource[] =
        "struct PSIn { float4 d0 : COLOR0; float4 d1 : COLOR1; "
        "float4 t0 : TEXCOORD0; float4 t1 : TEXCOORD1; float4 t2 : TEXCOORD2; "
        "float4 t3 : TEXCOORD3; float fog : FOG; };\n"
        "sampler2D s0 : register(s0); sampler2D s1 : register(s1); "
        "sampler2D s2 : register(s2);\n"
        "float4 fogColor : register(c0);\n"
        "float4 main(PSIn input) : COLOR0 {\n"
        "  float4 diffuse = tex2D(s0, input.t0.xy / max(abs(input.t0.w), 1e-20));\n"
        "  float4 blend = tex2D(s1, input.t1.xy / max(abs(input.t1.w), 1e-20));\n"
        "  float4 lightmap = tex2D(s2, input.t3.xy / max(abs(input.t3.w), 1e-20));\n"
        "  float3 rgb = lerp(diffuse.rgb, blend.rgb, saturate(input.d0.a));\n"
        "  rgb *= input.d0.rgb;\n"
        "  rgb *= 1.0 - lightmap.a * input.d1.a;\n"
        "  return float4(lerp(fogColor.rgb, rgb, saturate(input.fog)), diffuse.a);\n"
        "}\n";
    static const char WorldBlendRgbAndAlphaLightmapSource[] =
        "struct PSIn { float4 d0 : COLOR0; float4 d1 : COLOR1; "
        "float4 t0 : TEXCOORD0; float4 t1 : TEXCOORD1; float4 t2 : TEXCOORD2; "
        "float4 t3 : TEXCOORD3; float fog : FOG; };\n"
        "sampler2D s0 : register(s0); sampler2D s1 : register(s1); "
        "sampler2D s2 : register(s2); sampler2D s3 : register(s3);\n"
        "float4 fogColor : register(c0);\n"
        "float4 main(PSIn input) : COLOR0 {\n"
        "  float4 diffuse = tex2D(s0, input.t0.xy / max(abs(input.t0.w), 1e-20));\n"
        "  float4 blend = tex2D(s1, input.t1.xy / max(abs(input.t1.w), 1e-20));\n"
        "  float4 lightmap = tex2D(s2, input.t2.xy / max(abs(input.t2.w), 1e-20));\n"
        "  float4 alphaMap = tex2D(s3, input.t3.xy / max(abs(input.t3.w), 1e-20));\n"
        "  float3 rgb = lerp(diffuse.rgb, blend.rgb, saturate(input.d0.a));\n"
        "  rgb *= input.d0.rgb * lightmap.rgb;\n"
        "  rgb *= 1.0 - alphaMap.a * input.d1.a;\n"
        "  return float4(lerp(fogColor.rgb, rgb, saturate(input.fog)), diffuse.a);\n"
        "}\n";
    static const char WorldBlendPointLitSource[] =
        "struct PSIn { float4 d0 : COLOR0; float4 d1 : COLOR1; "
        "float4 t0 : TEXCOORD0; float4 t1 : TEXCOORD1; float fog : FOG; };\n"
        "sampler2D s0 : register(s0); sampler2D s1 : register(s1);\n"
        "float4 fogColor : register(c0);\n"
        "float4 main(PSIn input) : COLOR0 {\n"
        "  float4 diffuse = tex2D(s0, input.t0.xy / max(abs(input.t0.w), 1e-20));\n"
        "  float4 blend = tex2D(s1, input.t1.xy / max(abs(input.t1.w), 1e-20));\n"
        "  float3 rgb = lerp(diffuse.rgb, blend.rgb, saturate(input.d0.a));\n"
        "  rgb *= input.d1.rgb;\n"
        "  return float4(lerp(fogColor.rgb, rgb, saturate(input.fog)), diffuse.a);\n"
        "}\n";
    static const char WorldBlendPointLitAlphaLightmapSource[] =
        "struct PSIn { float4 d0 : COLOR0; float4 d1 : COLOR1; "
        "float4 t0 : TEXCOORD0; float4 t1 : TEXCOORD1; float4 t3 : TEXCOORD3; "
        "float fog : FOG; };\n"
        "sampler2D s0 : register(s0); sampler2D s1 : register(s1); "
        "sampler2D s2 : register(s2);\n"
        "float4 fogColor : register(c0);\n"
        "float4 main(PSIn input) : COLOR0 {\n"
        "  float4 diffuse = tex2D(s0, input.t0.xy / max(abs(input.t0.w), 1e-20));\n"
        "  float4 blend = tex2D(s1, input.t1.xy / max(abs(input.t1.w), 1e-20));\n"
        "  float4 alphaMap = tex2D(s2, input.t3.xy / max(abs(input.t3.w), 1e-20));\n"
        "  float3 rgb = lerp(diffuse.rgb, blend.rgb, saturate(input.d0.a));\n"
        "  rgb *= input.d1.rgb * (1.0 - alphaMap.a * input.d0.a);\n"
        "  return float4(lerp(fogColor.rgb, rgb, saturate(input.fog)), diffuse.a);\n"
        "}\n";
    static const char WorldBlendPointLitLightmapSource[] =
        "struct PSIn { float4 d0 : COLOR0; float4 d1 : COLOR1; "
        "float4 t0 : TEXCOORD0; float4 t1 : TEXCOORD1; float4 t2 : TEXCOORD2; "
        "float fog : FOG; };\n"
        "sampler2D s0 : register(s0); sampler2D s1 : register(s1); "
        "sampler2D s2 : register(s2);\n"
        "float4 fogColor : register(c0);\n"
        "float4 main(PSIn input) : COLOR0 {\n"
        "  float4 diffuse = tex2D(s0, input.t0.xy / max(abs(input.t0.w), 1e-20));\n"
        "  float4 blend = tex2D(s1, input.t1.xy / max(abs(input.t1.w), 1e-20));\n"
        "  float4 lightmap = tex2D(s2, input.t2.xy / max(abs(input.t2.w), 1e-20));\n"
        "  float3 rgb = lerp(diffuse.rgb, blend.rgb, saturate(input.d0.a));\n"
        "  rgb *= lightmap.rgb + input.d1.rgb;\n"
        "  return float4(lerp(fogColor.rgb, rgb, saturate(input.fog)), diffuse.a);\n"
        "}\n";
    static const char WorldBlendPointLitRgbAndAlphaLightmapSource[] =
        "struct PSIn { float4 d0 : COLOR0; float4 d1 : COLOR1; "
        "float4 t0 : TEXCOORD0; float4 t1 : TEXCOORD1; float4 t2 : TEXCOORD2; "
        "float4 t3 : TEXCOORD3; float fog : FOG; };\n"
        "sampler2D s0 : register(s0); sampler2D s1 : register(s1); "
        "sampler2D s2 : register(s2); sampler2D s3 : register(s3);\n"
        "float4 fogColor : register(c0);\n"
        "float4 main(PSIn input) : COLOR0 {\n"
        "  float4 diffuse = tex2D(s0, input.t0.xy / max(abs(input.t0.w), 1e-20));\n"
        "  float4 blend = tex2D(s1, input.t1.xy / max(abs(input.t1.w), 1e-20));\n"
        "  float4 lightmap = tex2D(s2, input.t2.xy / max(abs(input.t2.w), 1e-20));\n"
        "  float4 alphaMap = tex2D(s3, input.t3.xy / max(abs(input.t3.w), 1e-20));\n"
        "  float3 rgb = lerp(diffuse.rgb, blend.rgb, saturate(input.d0.a));\n"
        "  rgb *= (lightmap.rgb + input.d1.rgb) * (1.0 - alphaMap.a * input.d0.a);\n"
        "  return float4(lerp(fogColor.rgb, rgb, saturate(input.fog)), diffuse.a);\n"
        "}\n";
    const char* Source = isWorldBlendPointLitRgbAndAlphaLightmap ? WorldBlendPointLitRgbAndAlphaLightmapSource
        : (isWorldBlendPointLitLightmap ? WorldBlendPointLitLightmapSource
        : (isWorldBlendPointLitAlphaLightmap ? WorldBlendPointLitAlphaLightmapSource
        : (isWorldBlendPointLit ? WorldBlendPointLitSource
        : (isWorldBlendLightmap ? WorldBlendLightmapSource
        : (isWorldBlendRgbAndAlphaLightmap ? WorldBlendRgbAndAlphaLightmapSource
        : (isWorldBlendAlphaLightmap ? WorldBlendAlphaLightmapSource
        : (isPointLitProjected ? PointLitProjectedSource
        : (isPrelitLightmap ? PrelitLightmapSource
        : (isPointLitAdditiveLightmap ? PointLitAdditiveLightmapSource
        : (isPointLitAdditive ? PointLitAdditiveSource
        : (isWorldLightmapVertexLit || isPointLitLightmap ? WorldLightmapVertexLitSource
        : (isWorldTwoTextureVertexLit ? WorldTwoTextureVertexLitSource
        : (isWorldTwoTexture ? WorldTwoTextureSource
        : (isWorldBlend ? WorldBlendSource
        : (isWorldTexturedVertexLit || isTexturedVertexColored ? WorldTextureVertexLitSource
        : (isWorldTextured ? WorldTextureSource
        : (isColor ? ColorSource
        : (isSky ? SkySource : WorldSource))))))))))))))))));
    D3DCompileProc compiler = GetCompiler();
    if (compiler == nullptr)
        return nullptr;
    ID3DBlob* bytecode = nullptr;
    ID3DBlob* errors = nullptr;
    // NV2A sampler stage i uses the matching interpolated pTi coordinate.
    // PSInputTexture only selects combiner T operands; it does not remap the
    // texture-coordinate interpolants used by the sampler.
    std::string remappedSource = RemapFogSemantic(Source);
    const char FogDeclaration[] =
        "float4 fogState : register(c1);\n"
        "float nv2aFogFactor(float value) { return fogState.x < 0.5 ? 1.0 : saturate(value); }\n";
    const std::string FogColorDeclaration = "float4 fogColor : register(c0);\n";
    const size_t FogColorOffset = remappedSource.find(FogColorDeclaration);
    if (FogColorOffset != std::string::npos)
        remappedSource.insert(FogColorOffset + FogColorDeclaration.size(), FogDeclaration);
    size_t FogUseOffset = 0;
    while ((FogUseOffset = remappedSource.find("saturate(input.fog)", FogUseOffset)) !=
           std::string::npos) {
        remappedSource.replace(FogUseOffset, sizeof("saturate(input.fog)") - 1,
                               "nv2aFogFactor(input.fog)");
        FogUseOffset += sizeof("nv2aFogFactor(input.fog)") - 1;
    }
    static std::map<std::string, IDirect3DPixelShader9*> cache;
    const auto found = cache.find(remappedSource);
    if (found != cache.end())
        return found->second;
    const HRESULT result = compiler(remappedSource.data(), remappedSource.size(), "nv2a_psh", nullptr,
                                    nullptr, "main", "ps_3_0", 0, 0,
                                    &bytecode, &errors);
    if (FAILED(result)) {
        if (errors != nullptr)
            OutputDebugStringA((const char*)errors->GetBufferPointer());
        if (errors != nullptr)
            errors->Release();
        return nullptr;
    }
    if (errors != nullptr)
        errors->Release();
    IDirect3DPixelShader9* shader = nullptr;
    if (FAILED(device->CreatePixelShader((const DWORD*)bytecode->GetBufferPointer(), &shader)))
        shader = nullptr;
    bytecode->Release();
    if (shader != nullptr)
        cache[remappedSource] = shader;
    return shader;
}

IDirect3DPixelShader9* nullD3DCompileNV2ACombinerPixelShader(
    IDirect3DDevice9* device, const _D3DPixelShaderDef* definition) {
    const PixelShaderDefLayout* layout =
        reinterpret_cast<const PixelShaderDefLayout*>(definition);
    if (device == nullptr || layout == nullptr)
        return nullptr;
    std::string source = BuildNV2ACombinerHlsl(layout);
    if (source.empty())
        return nullptr;
    source = RemapFogSemantic(source.c_str());
    static std::map<std::string, IDirect3DPixelShader9*> cache;
    const auto found = cache.find(source);
    if (found != cache.end())
        return found->second;
    D3DCompileProc compiler = GetCompiler();
    if (compiler == nullptr)
        return nullptr;
    ID3DBlob* bytecode = nullptr;
    ID3DBlob* errors = nullptr;
    const HRESULT result = compiler(source.data(), source.size(), "nv2a_combiner", nullptr,
                                    nullptr, "main", "ps_3_0", 0, 0,
                                    &bytecode, &errors);
    if (FAILED(result)) {
        if (errors != nullptr)
            OutputDebugStringA((const char*)errors->GetBufferPointer());
        if (errors != nullptr)
            errors->Release();
        return nullptr;
    }
    if (errors != nullptr)
        errors->Release();
    IDirect3DPixelShader9* shader = nullptr;
    if (FAILED(device->CreatePixelShader((const DWORD*)bytecode->GetBufferPointer(), &shader)))
        shader = nullptr;
    bytecode->Release();
    if (shader != nullptr)
        cache[source] = shader;
    return shader;
}
