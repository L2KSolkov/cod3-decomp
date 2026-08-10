// ============================================================================
// g_comparse.cpp - game.o parse session + tokenizer (q_parse.cpp)
// Verified against IDA (release map offsets + 0x40C000 = VA).
// ============================================================================

#include "game/logic/g_local.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

// ============================================================================
// Parse state (game.o data)
// ============================================================================
struct parseInfo_t {
    char token[128];         // +0x00
    int lines;               // +0x80
    int ungetToken;          // +0x84
    int spaceDelimited;      // +0x88
    int csv;                 // +0x8C
    int negativeNumbers;     // +0x90
    int backup_lines;        // +0x94
    const char* backup_text; // +0x98
    char parseFile[128];     // +0x9C
};
static parseInfo_t parseInfo[8];
static int parseInfoNum;
static parseInfo_t* pi;
static const char* tokenPos;
static const char* prevTokenPos;

struct com_parse_mark_t {
    int lines;               // +0x00
    const char* text;        // +0x04
    int ungetToken;          // +0x08
    int backup_lines;        // +0x0C
    const char* backup_text; // +0x10
};

static const char* const punctuation[] = {
    "+=", "-=", "*=", "/=", "&=", "|=", "^=", "++", "--", "&&", "||",
    "->", "<<", ">>", "<=", ">=", "==", "!=", "...", nullptr,
};

// ============================================================================
// Parse session helpers - ea: 0x60FB20..0x60FC90
// ============================================================================
// ea: 0x0060FB20
parseInfo_t* Com_EndParseSession()
{
    int v0 = parseInfoNum;
    if (parseInfoNum == ERR_FATAL)
    {
        Com_Error(ERR_FATAL, "Com_EndParseSession: called without begin");
        v0 = parseInfoNum;
    }
    parseInfoNum = v0 - 1;
    parseInfo_t* result = &parseInfo[v0 - 1];
    pi = result;
    return result;
}

// ea: 0x0060FB60
void Com_ResetParseSessions()
{
    parseInfoNum = ERR_FATAL;
    pi = parseInfo;
}

// ea: 0x0060FB80
void Com_SetSpaceDelimited(int spaceDelimited)
{
    pi->spaceDelimited = spaceDelimited;
}

// ea: 0x0060FBA0
void Com_SetCSV(int csv)
{
    pi->csv = csv;
}

void Com_SetParseNegativeNumbers(int negativeNumbers)
{
    pi->negativeNumbers = negativeNumbers;
}

// ea: 0x0060FBE0
int Com_GetCurrentParseLine()
{
    return pi->lines;
}

void Com_ScriptError(const char* msg, ...)
{
    char string[32000];
    va_list ap;
    va_start(ap, msg);
    vsprintf(string, msg, ap);
    va_end(ap);
    Com_Error(ERR_DROP, "File %s, line %i: %s", pi->parseFile, pi->lines,
              string);
}

// ea: 0x0060FC40
void Com_ScriptWarning(const char* msg, ...)
{
    char string[32000];
    va_list ap;
    va_start(ap, msg);
    vsprintf(string, msg, ap);
    va_end(ap);
    Com_Printf("File %s, line %i: %s", pi->parseFile, pi->lines, string);
}

// ============================================================================
// Com_UngetToken / GetLastTokenPos - ea: 0x60FC90 / 0x60FEC0
// ============================================================================
// ea: 0x0060FC90
parseInfo_t* Com_UngetToken()
{
    parseInfo_t* result = pi;
    if (pi->ungetToken != 0)
    {
        Com_ScriptError("UngetToken called twice");
        result = pi;
    }
    tokenPos = prevTokenPos;
    pi->ungetToken = 1;
    return result;
}

// ea: 0x0060FEC0
const char* Com_GetLastTokenPos()
{
    return tokenPos;
}

// ============================================================================
// Com_ParseSetMark / Com_ParseReturnToMark - ea: 0x60FCE0 / 0x60FDB0
// ============================================================================
// ea: 0x0060FCE0
void Com_ParseSetMark(const char** text, com_parse_mark_t* mark)
{
    if (text == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\q_parse.cpp";
        AeAssert::gCurrentLine = 189;
        AeAssert::gCurrentExpr = "text";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (mark == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\q_parse.cpp";
        AeAssert::gCurrentLine = 190;
        AeAssert::gCurrentExpr = "mark";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    mark->lines = pi->lines;
    mark->text = *text;
    mark->ungetToken = pi->ungetToken;
    mark->backup_lines = pi->backup_lines;
    mark->backup_text = pi->backup_text;
}

// ea: 0x0060FDB0
void Com_ParseReturnToMark(const char** text, com_parse_mark_t* mark)
{
    if (text == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\q_parse.cpp";
        AeAssert::gCurrentLine = 206;
        AeAssert::gCurrentExpr = "text";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    if (mark == nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\q_parse.cpp";
        AeAssert::gCurrentLine = 207;
        AeAssert::gCurrentExpr = "mark";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    pi->lines = mark->lines;
    *text = mark->text;
    pi->ungetToken = mark->ungetToken;
    pi->backup_lines = mark->backup_lines;
    pi->backup_text = mark->backup_text;
}

// ============================================================================
// SkipWhitespace + Com_ParseExt - ea: 0x610200
// ============================================================================
static const char* SkipWhitespace(const char* data, int* hasNewLines)
{
    const char* s = data;
    while (*s != 0)
    {
        if (*s <= 32)
        {
            if (*s == 10)
            {
                ++pi->lines;
                *hasNewLines = 1;
            }
            ++s;
        }
        else
        {
            return s;
        }
    }
    return nullptr;
}

static parseInfo_t* Com_ParseCSV(const char** data_p, int allowLineBreaks)
{
    // CSV tokenizer: quoted fields with \"" escape, commas as separators.
    const char* v7 = *data_p;
    int v2 = 0;
    pi->token[0] = 0;
    while (*v7 != 0)
    {
        if (*v7 == ',')
        {
            ++v7;
            break;
        }
        if (*v7 == 10 && allowLineBreaks == 0)
            break;
        if (*v7 == 34)
        {
            ++v7;
            while (*v7 != 34 && *v7 != 0)
            {
                if (*v7 == 10)
                    ++pi->lines;
                if (v2 < 127)
                    pi->token[v2++] = *v7;
                ++v7;
            }
            if (*v7 == 34)
                ++v7;
        }
        else
        {
            if (v2 < 127)
                pi->token[v2++] = *v7;
            ++v7;
        }
    }
    pi->token[v2] = 0;
    *data_p = v7;
    return pi;
}

parseInfo_t* Com_ParseExt(const char** data_p, int allowLineBreaks)
{
    int v2 = 0;
    int hasNewLines = 0;
    char v8;
    char v9;
    char v12;
    char v13;
    char v14;
    char v15;
    if (data_p == nullptr)
        Com_Error(ERR_FATAL, "Com_ParseExt: NULL data_p");
    const char* v3 = *data_p;
    parseInfo_t* v5 = pi;
    pi->token[0] = 0;
    if (*data_p == nullptr)
    {
        *data_p = nullptr;
        return v5;
    }
    v5->backup_lines = v5->lines;
    v5->backup_text = *data_p;
    if (v5->csv != 0)
        return Com_ParseCSV(data_p, allowLineBreaks);
    const char* v7 = SkipWhitespace(v3, &hasNewLines);
    if (v7 == nullptr)
    {
        *data_p = nullptr;
        return pi;
    }
    for (;;)
    {
        if (hasNewLines != 0 && allowLineBreaks == 0)
        {
            *data_p = v7;
            return pi;
        }
        v8 = *v7;
        if (*v7 != 47)
            goto token_start;
        v9 = v7[1];
        if (v9 != 47)
            break;
        // // line comment
        do
        {
            if (v8 == 10)
                break;
            v8 = *++v7;
        } while (v8 != 0);
    skip_ws:
        v7 = SkipWhitespace(v7, &hasNewLines);
        if (v7 == nullptr)
        {
            *data_p = nullptr;
            return pi;
        }
    }
    if (v9 == 42)
    {
        // /* */ block comment
        parseInfo_t* v10 = pi;
        while (v8 != 42 || v7[1] != 47)
        {
            if (v8 == 10)
                ++v10->lines;
            v8 = *++v7;
            if (v8 == 0)
                goto skip_ws;
        }
        if (*v7 != 0)
            v7 += 2;
        goto skip_ws;
    }
token_start:
    parseInfo_t* v11 = pi;
    prevTokenPos = tokenPos;
    tokenPos = v7;
    if (v8 == 34)
    {
        // quoted string
        ++v7;
        for (;;)
        {
            v12 = *v7++;
            if (v12 == 92)
            {
                if (*v7 == 34)
                {
                    v12 = 34;
                    ++v7;
                    goto copy_char;
                }
            }
            else if (v12 == 34 || v12 == 0)
            {
                goto token_done;
            }
            if (*v7 == 10)
                ++v11->lines;
        copy_char:
            if (v2 < 127)
                v11->token[v2++] = v12;
        }
    }
    if (pi->spaceDelimited != 0)
    {
        do
        {
            if (v2 < 127)
                v11->token[v2++] = v8;
            v8 = *++v7;
        } while (v8 > 32);
        goto token_done;
    }
    if (v8 >= 48 && v8 <= 57)
        goto number_start;
    if (pi->negativeNumbers != 0 && v8 == 45)
    {
        v13 = v7[1];
        if (v13 < 48 || v13 > 57)
            goto punctuation_or_single;
        goto number_start;
    }
    if (v8 == 46)
    {
        v14 = v7[1];
        if (v14 >= 48 && v14 <= 57)
        {
        number_loop:
        number_start:
            do
            {
                if (v2 < 127)
                    v11->token[v2++] = v8;
                v8 = *++v7;
            } while ((v8 >= 48 && v8 <= 57) || v8 == 46);
            if (v8 == 101 || v8 == 69)
            {
                if (v2 < 127)
                    v11->token[v2++] = v8;
                v15 = *++v7;
                if (v15 == 45 || v15 == 43)
                {
                    if (v2 < 127)
                        v11->token[v2++] = v15;
                    v15 = *++v7;
                }
                do
                {
                    if (v2 < 127)
                        v11->token[v2++] = v15;
                    v15 = *++v7;
                } while (v15 >= 48 && v15 <= 57);
            }
            goto token_done;
        }
    }
    else if ((v8 >= 97 && v8 <= 122) || (v8 >= 65 && v8 <= 90)
             || v8 == 95 || v8 == 47 || v8 == 92 || v8 == 58 || v8 == 46)
    {
        do
        {
            if (v2 < 127)
                v11->token[v2++] = v8;
            v8 = *++v7;
        } while ((v8 >= 97 && v8 <= 122) || (v8 >= 65 && v8 <= 90)
                 || v8 == 95 || v8 == 47 || v8 == 92 || v8 == 58
                 || v8 == 46);
        goto token_done;
    }
punctuation_or_single:
    {
        const char* v16 = punctuation[0];
        const char** v17 = (const char**)punctuation;
        int v18;
        if (punctuation[0] != nullptr)
        {
            for (;;)
            {
                v18 = (int)strlen(v16);
                int i;
                for (i = 0; i < v18; ++i)
                {
                    if (v7[i] != v16[i])
                        break;
                }
                if (i == v18)
                    break;
                v16 = v17[1];
                ++v17;
                if (v16 == nullptr)
                {
                    v11 = pi;
                    goto single_char;
                }
            }
            memcpy(pi, *v17, v18);
            parseInfo_t* v20 = pi;
            pi->token[v18] = 0;
            *data_p = &v7[v18];
            return v20;
        }
        else
        {
        single_char:
            v11->token[0] = *v7;
            v11->token[1] = 0;
            *data_p = v7 + 1;
            return v11;
        }
    }
token_done:
    if (v2 == 128)
        v2 = 0;
    v11->token[v2] = 0;
    *data_p = v7;
    return v11;
}

// ============================================================================
// Com_Parse / Com_ParseOnLine / Com_SkipRestOfLine
// ============================================================================
// ea: 0x00610300
const char* Com_Parse(const char** data_p)
{
    parseInfo_t* v1 = pi;
    if (pi->ungetToken != 0)
    {
        const char* backup_text = pi->backup_text;
        pi->ungetToken = 0;
        *data_p = backup_text;
        v1->lines = v1->backup_lines;
    }
    return Com_ParseExt(data_p, 1)->token;
}

// ea: 0x00610340
parseInfo_t* Com_ParseOnLine(const char** data_p)
{
    parseInfo_t* result = pi;
    if (pi->ungetToken != 0)
    {
        int spaceDelimited = pi->spaceDelimited;
        pi->ungetToken = 0;
        if (spaceDelimited == 0)
            return result;
        *data_p = result->backup_text;
        result->lines = result->backup_lines;
    }
    return Com_ParseExt(data_p, 0);
}

// ea: 0x00610390
void Com_SkipRestOfLine(const char** data)
{
    const char* v1 = *data;
    if (*data == nullptr)
        return;
    int v2 = *v1;
    if (*v1 == 0)
        return;
    for (;;)
    {
        ++v1;
        if (v2 == 10)
            break;
        v2 = *v1;
        if (*v1 == 0)
        {
            *data = v1;
            return;
        }
    }
    ++pi->lines;
    *data = v1;
}
