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
        result << "c[" << ConstantRegister(FieldValue(token, F_CONST)) << "]";
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
    case 6: return BroadcastScalar("dot(" + a + ".xyz, " + b + ".xyz) + " + c + ".w");
    case 7: return BroadcastScalar("dot(" + a + ", " + b + ")");
    case 8: return "float4(1.0, " + a + ".y * " + b + ".y, " + a + ".z, " + b + ".w)";
    case 9: return "min(" + a + ", " + b + ")";
    case 10: return "max(" + a + ", " + b + ")";
    case 11: return "(" + a + " < " + b + " ? 1.0 : 0.0)";
    case 12: return "(" + a + " >= " + b + " ? 1.0 : 0.0)";
    case 13: return a;
    default: return std::string();
    }
}

static std::string IluExpression(unsigned int opcode, const std::string& c) {
    switch (opcode) {
    case 1: return c;
    case 2: return BroadcastScalar("1.0 / " + c);
    case 3: return BroadcastScalar("1.0 / max(abs(" + c + "), 1.17549435e-38)");
    case 4: return BroadcastScalar("rsqrt(abs(" + c + "))");
    case 5: return BroadcastScalar("exp2(" + c + ")");
    case 6: return BroadcastScalar("log2(abs(" + c + "))");
    case 7: return "float4(1.0, max(" + c + ".x, 0.0), " + c + ".x > 0.0 ? pow(max(" + c + ".y, 0.0), " + c + ".w) : 0.0, 1.0)";
    default: return std::string();
    }
}

static const char* OutputName(unsigned int address) {
    static const char* names[] = {
        "r12", "oD1", "oD1", "oD0", "oD1", "oFog", "oPts",
        "oB0", "oB1", "oT0", "oT1", "oT2", "oT3"
    };
    const unsigned int index = address & 15u;
    return index < sizeof(names) / sizeof(names[0]) ? names[index] : "oD1";
}

static void AppendWrite(std::ostringstream& source, const std::string& expression,
                        const char* destination, const std::string& mask) {
    if (mask.empty())
        return;
    if (mask == "xyzw")
        source << "  " << destination << " = " << expression << ";\n";
    else
        source << "  " << destination << "." << mask << " = (" << expression << ")." << mask << ";\n";
}

static void AppendInstruction(std::ostringstream& source, const unsigned int* token) {
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
    const std::string macExpr = mac == 0 ? std::string() : MacExpression(mac, a, b, cMac);
    const std::string iluExpr = ilu == 0 ? std::string() : IluExpression(ilu, cIlu);

    if (mac != 0 && FieldValue(token, F_OUT_MUX) == 0)
        AppendWrite(source, macExpr, OutputName(outAddress), Mask(FieldValue(token, F_OUT_O_MASK)));
    if (ilu != 0 && FieldValue(token, F_OUT_MUX) != 0)
        AppendWrite(source, iluExpr, OutputName(outAddress), Mask(FieldValue(token, F_OUT_O_MASK)));

    if (paired) {
        if (mac != 0 && FieldValue(token, F_OUT_MAC_MASK) != 0 && outRegister != 1) {
            source << "  r" << outRegister << "." << Mask(FieldValue(token, F_OUT_MAC_MASK))
                   << " = (" << macExpr << ")." << Mask(FieldValue(token, F_OUT_MAC_MASK)) << ";\n";
        }
        if (ilu != 0 && FieldValue(token, F_OUT_ILU_MASK) != 0) {
            source << "  r1." << Mask(FieldValue(token, F_OUT_ILU_MASK))
                   << " = (" << iluExpr << ")." << Mask(FieldValue(token, F_OUT_ILU_MASK)) << ";\n";
        }
    } else if (mac != 0 && FieldValue(token, F_OUT_MUX) == 1) {
        AppendWrite(source, macExpr, (std::string("r") + std::to_string(outRegister)).c_str(),
                    Mask(FieldValue(token, F_OUT_MAC_MASK)));
    } else if (ilu != 0 && FieldValue(token, F_OUT_MUX) != 1) {
        AppendWrite(source, iluExpr, "r1", Mask(FieldValue(token, F_OUT_ILU_MASK)));
    }
}

static std::string BuildHlsl(const unsigned int* microcode) {
    const unsigned int header = microcode[0];
    const unsigned int instructionCount = header >> 16;
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
           << "VSOut main(VSIn input) { VSOut output;\n"
           << "  float4 v0=input.v0,v1=input.v1,v2=input.v2,v3=input.v3;\n"
           << "  float4 v4=input.v4,v5=input.v5,v6=input.v6,v7=input.v7;\n"
           << "  float4 r0=0,r1=0,r2=0,r3=0,r4=0,r5=0,r6=0,r7=0;\n"
           << "  float4 r8=0,r9=0,r10=0,r11=0;\n"
           << "  float4 r12=0; float4 oD0=0,oD1=0,oT0=0,oT1=0,oT2=0,oT3=0;\n"
           << "  float4 oB0=0,oB1=0,oFog=0,oPts=1;\n";
    for (unsigned int i = 0; i < instructionCount; ++i)
        AppendInstruction(source, microcode + 1 + i * 4);
    source << "  output.oPos=r12; output.oD0=oD0; output.oD1=oD1; output.oT0=oT0;"
           << " output.oT1=oT1; output.oT2=oT2; output.oT3=oT3; output.oB0=oB0;"
           << " output.oB1=oB1; output.oFog=oFog.x; output.oPts=oPts.x; return output; }\n";
    return source.str();
}

struct BlobRelease { void operator()(ID3DBlob* blob) const { if (blob) blob->Release(); } };

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

} // namespace

IDirect3DVertexShader9* nullD3DCompileNV2AVertexShader(
    IDirect3DDevice9* device, const unsigned int* microcode) {
    if (device == nullptr || microcode == nullptr)
        return nullptr;
    const unsigned int* program = microcode;
    const unsigned int header = program[0];
    if ((header & 0xffffu) != 0x2078u) {
        // A few existing Xbox call sites pass &Shader, where Shader is the
        // registered token pointer. Resolve that ABI form before decoding.
        const unsigned int* indirect = reinterpret_cast<const unsigned int*>(
            (uintptr_t)(*(const unsigned long*)microcode));
        if (indirect == nullptr || (indirect[0] & 0xffffu) != 0x2078u)
            return nullptr;
        program = indirect;
    }
    static std::map<const unsigned int*, IDirect3DVertexShader9*> cache;
    const auto found = cache.find(program);
    if (found != cache.end())
        return found->second;
    const std::string hlsl = BuildHlsl(program);
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
