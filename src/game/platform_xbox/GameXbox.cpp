// ============================================================================
// GameXbox.cpp - Xbox game platform layer (game_xbox.o, first 31 functions)
// Verified against IDA (release map offsets + 0x40C000 = VA).
// Owns the idTech Sys_* platform surface: paths, file listing, event queue,
// stream-thread stubs and console stubs.
// The map's `time` override (decompile: returns 0) is intentionally not ported:
// the Win32 CRT provides time(); /FORCE:UNRESOLVED would mask a duplicate.
// ============================================================================

#include <string.h>
#include <windows.h>

// rdata 0xCD67AE - shared default file name ("or"; defined in g_globals.cpp,
// also used by MemoryUnit.o and sv.o; declared extern in game/sv/sv_stubs.h).
extern const char* const defaultFileName;

// ============================================================================
// Assertion system externs (core_xboxr:AeAssert.o)
// ============================================================================
namespace AeAssert {
enum ECoderId { COD3 = 0 };
extern ECoderId gCurrentAuthor;
extern const char* gCurrentFile;
extern int gCurrentLine;
extern const char* gCurrentExpr;
bool IsIgnored();
bool Assert(const char* fmt, ...);
}

// ============================================================================
// Cross-object externs (core.o / game2.o, resolved when those objects port)
// ============================================================================
extern void Com_sprintf(char* dest, int size, const char* fmt, ...);
extern int Q_stricmp(const char* s1, const char* s2);
extern int Com_FilterPath(char* filter, char* name, int casesensitive);
extern char* CopyStringInternal(const char* in);
extern void* _Z_MallocInternal(int size);
extern void _Z_FreeInternal(void* ptr);
extern int Sys_Milliseconds();
extern void Cvar_Set(const char* var_name, const char* value);
extern void Cvar_SetValue(const char* var_name, float value);
extern void IN_Init();
extern int FS_Read(void* buffer, int len, int f);
extern int FS_Seek(int f, long offset, int origin);
extern void tlFatal(const char* fmt, ...);

// ============================================================================
// sysEvent_t - input/event queue entry (0x18, verified against IDA)
// ============================================================================
enum sysEventType_t
{
    SE_NONE = 0,
    SE_KEY = 1,
    SE_CHAR = 2,
    SE_MOUSE = 3,
    SE_JOYSTICK_AXIS = 4,
    SE_CONSOLE = 5,
    SE_PACKET = 6,
};

struct sysEvent_t
{
    int evTime;          // +0x00
    sysEventType_t evType;   // +0x04
    int evValue;         // +0x08
    int evValue2;        // +0x0C
    int evPtrLength;     // +0x10
    void* evPtr;         // +0x14
};
static_assert(sizeof(sysEvent_t) == 0x18, "sysEvent_t size mismatch");

// ============================================================================
// game_xbox.o globals
// ============================================================================
bool gSkipFrontEnd;
bool gSkipMovies;
int eventHead;
int eventTail;
sysEvent_t eventQue[64];

// ea: 0x71EF50
void CheckCommands(const char* text)
{
    gSkipFrontEnd = false;
    gSkipMovies = false;
    int v1 = 0;
    if (*text != 0)
    {
        char v2;
        do
        {
            if (v1 >= 4
                && text[v1 - 4] == '+'
                && text[v1 - 3] == 's'
                && text[v1 - 2] == 't'
                && text[v1 - 1] == 'u'
                && text[v1] == 'b')
            {
                gSkipMovies = true;
            }
            if (v1 >= 6
                && text[v1 - 6] == '+'
                && text[v1 - 5] == 'd'
                && text[v1 - 4] == 'e'
                && text[v1 - 3] == 'v'
                && text[v1 - 2] == 'm'
                && text[v1 - 1] == 'a'
                && text[v1] == 'p')
            {
                gSkipFrontEnd = true;
            }
            v2 = text[++v1];
        }
        while (v2 != 0);
    }
}

// ea: 0x71EFF0
void Sys_Mkdir(const char* path)
{
    CreateDirectoryA(path, 0);
}

// ea: 0x71F000
char* Sys_Cwd()
{
    return (char*)defaultFileName;
}

// ea: 0x71F010
char* Sys_DefaultCDPath()
{
    return "d:";
}

// ea: 0x71F020
char* Sys_DefaultBasePath()
{
    return (char*)defaultFileName;
}

// ea: 0x71F030
char* Sys_DefaultHomePath()
{
    return nullptr;
}

// ea: 0x71F040
char* Sys_DefaultInstallPath()
{
    return (char*)defaultFileName;
}

// ea: 0x71F050
void Sys_ListFilteredFiles(const char* basedir, char* subdirs, char* filter,
                           char** list, int* numfiles)
{
    char search[128];
    WIN32_FIND_DATAA wfd;
    char filename[128];
    if (*numfiles < 4095)
    {
        if (strlen(subdirs) != 0)
            Com_sprintf(search, 128, "%s\\%s\\*", basedir, subdirs);
        else
            Com_sprintf(search, 128, "%s\\*", basedir);
        HANDLE hFind = FindFirstFileA(search, &wfd);
        if (hFind != (HANDLE)-1)
        {
            do
            {
                if ((wfd.dwFileAttributes & 0x10) == 0
                    || Q_stricmp(wfd.cFileName, ".") != 0
                        && Q_stricmp(wfd.cFileName, "..") != 0
                        && Q_stricmp(wfd.cFileName, "CVS") != 0)
                {
                    if (*numfiles >= 4095)
                        break;
                    if (subdirs != nullptr)
                        Com_sprintf(filename, 128, "%s\\%s", subdirs, wfd.cFileName);
                    else
                        Com_sprintf(filename, 128, "%s", wfd.cFileName);
                    if (Com_FilterPath(filter, filename, 0) != 0)
                        list[(*numfiles)++] = CopyStringInternal(filename);
                }
            }
            while (FindNextFileA(hFind, &wfd) != 0);
            CloseHandle(hFind);
        }
    }
}

// ea: 0x71F1E0
int linelen(char* str)
{
    char v1 = *str;
    const char* i;
    for (i = str; v1 != 0; v1 = *++i)
    {
        if (v1 == 10)
            break;
    }
    return (int)(i - str);
}

// ea: 0x71F210
int ReadLine(const char* in, char* out)
{
    char v2 = *in;
    const char* i;
    for (i = in; v2 != 0; v2 = *++i)
    {
        if (v2 == 10)
            break;
    }
    int result = (int)(i - in);
    if (result > 0)
    {
        memcpy(out, in, result);
        out[result] = 0;
    }
    return result;
}

// ea: 0x71F260
char** Sys_ListFiles(const char* directory, const char* extension,
                     char* filter, int* numfiles, int wantsubs)
{
    int v5;
    int v9;
    int v12;
    char* list[4096];
    char search[128];
    WIN32_FIND_DATAA wfd;

    if (filter != 0)
    {
        char* v14 = filter;
        int nf = 0;
        Sys_ListFilteredFiles(directory, (char*)defaultFileName, v14, list, &nf);
        v5 = nf;
        list[nf] = nullptr;
        *numfiles = nf;
        if (nf == 0)
            return nullptr;
        goto LABEL_24;
    }
    {
        const char* v8 = extension;
        if (extension == nullptr)
        {
            v8 = defaultFileName;
            v9 = 16;
        }
        else if (*extension == '/' && extension[1] == 0)
        {
            v8 = defaultFileName;
            v9 = 0;
        }
        else
        {
            v9 = 16;
        }
        Com_sprintf(search, 128, "%s\\*%s", directory, v8);
    }
    v5 = 0;
    HANDLE hFind = FindFirstFileA(search, &wfd);
    if (hFind == (HANDLE)-1)
    {
        *numfiles = 0;
        return nullptr;
    }
    while (wantsubs == 0)
    {
        v12 = wfd.dwFileAttributes & 0x10;
        if (v9 != v12)
            goto LABEL_17;
LABEL_22:
        if (FindNextFileA(hFind, &wfd) == 0)
            goto LABEL_23;
    }
    v12 = wfd.dwFileAttributes & 0x10;
    if ((wfd.dwFileAttributes & 0x10) == 0)
        goto LABEL_22;
LABEL_17:
    if (v12 != 0
        && (Q_stricmp(wfd.cFileName, ".") == 0
            || Q_stricmp(wfd.cFileName, "..") == 0
            || Q_stricmp(wfd.cFileName, "CVS") == 0))
    {
        goto LABEL_22;
    }
    list[v5++] = CopyStringInternal(wfd.cFileName);
    if (v5 != 4095)
        goto LABEL_22;
LABEL_23:
    list[v5] = nullptr;
    CloseHandle(hFind);
    *numfiles = v5;
    if (v5 == 0)
        return nullptr;
LABEL_24:
    {
        char** result = (char**)_Z_MallocInternal(4 * v5 + 4);
        if (v5 > 0)
            memcpy(result, list, 4 * v5);
        result[v5] = nullptr;
        return result;
    }
}

// ea: 0x71F420
void Sys_FreeFileList(char** list)
{
    if (list != nullptr)
    {
        char* v1 = *list;
        if (*list != nullptr)
        {
            char** v2 = list;
            do
            {
                _Z_FreeInternal(v1);
                v1 = v2[1];
                ++v2;
            }
            while (v1 != nullptr);
        }
        _Z_FreeInternal(list);
    }
}

// ea: 0x71F460
int Sys_DirectoryHasContents(const char* directory)
{
    char search[128];
    WIN32_FIND_DATAA wfd;
    Com_sprintf(search, 128, "%s\\*", directory);
    HANDLE hFind = FindFirstFileA(search, &wfd);
    if (hFind == (HANDLE)-1)
        return 0;
    while ((wfd.dwFileAttributes & 0x10) != 0
        && (Q_stricmp(wfd.cFileName, ".") == 0
            || Q_stricmp(wfd.cFileName, "..") == 0
            || Q_stricmp(wfd.cFileName, "CVS") == 0))
    {
        if (FindNextFileA(hFind, &wfd) == 0)
        {
            CloseHandle(hFind);
            return 0;
        }
    }
    return 1;
}

// ea: 0x71F520
void Sys_InitStreamThread()
{
}

// ea: 0x71F530
void Sys_ShutdownStreamThread()
{
}

// ea: 0x71F540
void Sys_BeginStreamedFile(int f, int size)
{
    (void)f;
    (void)size;
}

// ea: 0x71F550
void Sys_EndStreamedFile(int f)
{
    (void)f;
}

// ea: 0x71F560
int Sys_StreamedRead(void* buffer, int size, int count, int f)
{
    return FS_Read((unsigned char*)buffer, (unsigned int)(count * size), f);
}

// ea: 0x71F580
void Sys_StreamSeek(int f, int offset, int origin)
{
    FS_Seek(f, offset, origin);
}

// ea: 0x71F590
void* Sys_InitializeCriticalSection()
{
    return (void*)-1;
}

// ea: 0x71F5A0
void Sys_EnterCriticalSection(void* cs)
{
    (void)cs;
}

// ea: 0x71F5B0
void Sys_LeaveCriticalSection(void* cs)
{
    (void)cs;
}

// ea: 0x71F5C0
void Sys_OutOfMemError()
{
    tlFatal("out of memory");
}

// ea: 0x71F5D0
int Sys_ConfigureChecksumChanged(int checksumFeed)
{
    (void)checksumFeed;
    return 0;
}

// ea: 0x71F5E0
void GLimp_SetGamma(unsigned char* red, unsigned char* green, unsigned char* blue)
{
    (void)red;
    (void)green;
    (void)blue;
}

// ea: 0x71F5F0 - not ported: the Xbox binary overrode the CRT `time` (return 0);
// Win32 keeps the CRT's real time(). See file header.

// ea: 0x71F600
void Sys_QueEvent(int time, sysEventType_t type, int value, int value2,
                  int ptrLength, void* ptr)
{
    sysEvent_t* v6 = &eventQue[eventHead & 0x3F];
    if (eventHead - eventTail >= 64)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\ps2_shared.cpp";
        AeAssert::gCurrentLine = 54;
        AeAssert::gCurrentExpr = "0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("sys_QueEvent: Overflow - Tell MikeA"))
        {
            __debugbreak();
        }
        if (v6->evPtr != nullptr)
            _Z_FreeInternal(v6->evPtr);
        ++eventTail;
    }
    int v7 = time;
    ++eventHead;
    if (time == 0)
        v7 = Sys_Milliseconds();
    v6->evTime = v7;
    v6->evType = type;
    v6->evValue = value;
    v6->evValue2 = value2;
    v6->evPtrLength = ptrLength;
    v6->evPtr = ptr;
}

// ea: 0x71F6C0
void Sys_LoadingKeepAlive()
{
}

// ea: 0x71F6D0
sysEvent_t Sys_GetEvent()
{
    sysEvent_t result;
    if (eventHead <= eventTail)
    {
        result.evTime = Sys_Milliseconds();
        result.evType = SE_NONE;
        result.evValue = 0;
        result.evValue2 = 0;
        result.evPtrLength = 0;
        result.evPtr = nullptr;
    }
    else
    {
        int v2 = eventTail & 0x3F;
        ++eventTail;
        result.evTime = eventQue[v2].evTime;
        result.evType = eventQue[v2].evType;
        result.evValue = eventQue[v2].evValue;
        result.evValue2 = eventQue[v2].evValue2;
        result.evPtrLength = eventQue[v2].evPtrLength;
        result.evPtr = eventQue[v2].evPtr;
    }
    return result;
}

// ea: 0x71F760
void Sys_Init()
{
    Cvar_Set("arch", "ps2");
    Cvar_SetValue("sys_cpuid", 1.0);
    IN_Init();
}

// ea: 0x71F790
void Sys_ShowConsole(int show, int force)
{
    (void)show;
    (void)force;
}
