// ============================================================================
// files.cpp - filesystem core (core.o com_files.cpp)
// Reconstructed from IDA release decompiles (ea comments below).
// ============================================================================

#include "game/core/core_types.h"
#include "game/core/core_systems.h"
#include "game/core/core_globals.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ============================================================================
// Externs (core.o data / libc / helpers)
// ============================================================================
extern int SEH_GetCurrentLanguage();
extern int Q_islower(int c);
extern int Q_stricmp(const char* s1, const char* s2);
extern void Com_Error(int code, const char* fmt, ...);
extern int fs_loadStack;
extern int com_fileAccessed;
extern int fs_numServerPaks;
extern int com_journalDataFile;
extern int fs_checksumFeed;
extern const char defaultFileName[];
extern cvar_t* fs_debug;
extern cvar_t* fs_copyfiles;
extern cvar_t* fs_cdpath;
extern cvar_t* fs_basepath;
extern cvar_t* fs_basegame;
extern cvar_t* fs_homepath;
extern cvar_t* fs_gamedirvar;
extern cvar_t* fs_restrict;
extern cvar_t* fs_ignoreLozalized;
extern cvar_t* com_journal;
extern filelist_s* fs_nonpackfilelist;
extern searchpath_s* fs_memorysearchpaths;
extern filelist_s* fs_memorynonpackfilelist;
extern struct cvar_t* Cvar_Get(const char* var_name, const char* var_value,
                               int flags);
extern cvar_t* Cvar_Set2(const char* var_name, const char* value, int force);
extern void Cbuf_AddText(const char* text);
extern void* mem_heap_malloc(unsigned int size);
extern void mem_heap_free(void* ptr);
extern void* mem_heap_malloc_ctx(int alignment, unsigned int size,
                                 const char* ctx, const char* file, int line);
extern void* _Z_MallocInternal(unsigned int size);
extern void _Z_FreeInternal(void* ptr);
extern char* CopyStringInternal(const char* in);
extern char* va(const char* fmt, ...);
extern void Com_Printf(const char* fmt, ...);
extern void Com_DPrintf(const char* fmt, ...);
extern void Com_Memset(void* dest, int val, unsigned int count);
extern void Com_sprintf(char* dest, int size, const char* fmt, ...);
extern void Q_strncpyz(char* dest, const char* src, int destsize);
extern void Cmd_RemoveCommand(const char* cmd_name);
extern int Cmd_Argc();
extern char* Cmd_Argv(int arg);
extern void Sys_Mkdir(const char* path);
extern void Sys_OutOfMemError();
extern char** Sys_ListFiles(const char* directory, const char* extension,
                            char* filter, int* numfiles, int wantsubs);
extern void Sys_FreeFileList(char** list);
extern char* Sys_DefaultCDPath();
extern char* Sys_DefaultInstallPath();
extern char* Sys_DefaultHomePath();
extern int Sys_DirectoryHasContents(const char* dirname);
extern int Com_SafeMode();
extern void Com_StartupVariable(const char* match);
extern void SEH_InitLanguage();
extern const char* SEH_GetLanguageName(int iLanguage);
extern int SEH_GetCurrentLanguage();

// Forward declarations (mutually recursive)
void FS_CopyFile(char* fromOSPath, char* toOSPath);
void FS_FCloseFile(int f);
unsigned int FS_Write(char* buffer, unsigned int len, int h);
void FS_Flush(int f);
int FS_filelength(int f);

namespace AeAssert {
enum ECoderId { COD3 = 0 };
extern ECoderId gCurrentAuthor;
extern const char* gCurrentFile;
extern int gCurrentLine;
extern const char* gCurrentExpr;
bool IsIgnored();
bool Assert(const char* fmt, ...);
bool Warning(const char* fmt, ...);
}

#define ASSERT(expr, file, line)                                          \
    do {                                                                  \
        AeAssert::gCurrentAuthor = AeAssert::COD3;                        \
        AeAssert::gCurrentFile = (file);                                  \
        AeAssert::gCurrentLine = (line);                                  \
        AeAssert::gCurrentExpr = (expr);                                  \
        if (!AeAssert::IsIgnored()                                        \
            && AeAssert::Assert("old cod assert"))                        \
            __debugbreak();                                               \
    } while (0)

// ============================================================================
// Filesystem
// ============================================================================

// ea: 0x004B5670
int FS_Initialized()
{
    return fs_searchpaths != nullptr;
}

// ea: 0x004B5680
char FS_CheckFileSystemStarted()
{
    char result = (char)fs_searchpaths;
    if (fs_searchpaths == nullptr)
    {
        ASSERT("fs_searchpaths", "c:\\cod\\code\\game\\com_files.cpp", 319);
    }
    return result;
}

// ea: 0x004B56D0
int FS_LoadStack()
{
    return fs_loadStack;
}

// ea: 0x004B56E0
int FS_UseSearchPath(const searchpath_s* pSearch)
{
    return pSearch->bLocalized == 0
        || (fs_ignoreLozalized->integer == 0
            && pSearch->language == SEH_GetCurrentLanguage());
}

// ea: 0x004B5720
int FS_LanguageHasAssets(int iLanguage)
{
    searchpath_s* v1 = fs_searchpaths;
    if (fs_searchpaths == nullptr)
        return 0;
    while (v1->bLocalized == 0 || v1->language != iLanguage)
    {
        v1 = v1->next;
        if (v1 == nullptr)
            return 0;
    }
    return 1;
}

// ea: 0x004B5750
int FS_HashFileName(const char* fname, int hashSize)
{
    const char* v2 = fname;
    int v3 = 0;
    char v6 = 0;
    if (*fname != 0)
    {
        do
        {
            int v4 = tolower((unsigned char)*v2);
            if (v4 == 46)
                break;
            if (v4 == 92)
                v4 = 47;
            int v5 = v4 * (v2 - fname + 119);
            v6 = v2[1];
            v3 += v5;
            ++v2;
        }
        while (v6 != 0);
    }
    return (hashSize - 1) & (v3 ^ ((v3 ^ (v3 >> 10)) >> 10));
}

// ea: 0x004B57C0
FILE* FS_FileForHandle(int f)
{
    ASSERT("f > 0 && f < (1 + 2 + 0)", "c:\\cod\\code\\game\\com_files.cpp",
           450);
    ASSERT("!fsh[f].zipFile", "c:\\cod\\code\\game\\com_files.cpp", 451);
    ASSERT("fsh[f].handleFiles.file.o", "c:\\cod\\code\\game\\com_files.cpp",
           452);
    return (FILE*)fsh[f].handleFiles.file.file;
}

// ea: 0x004B58C0
void FS_ForceFlush(int f)
{
    FILE* v1 = FS_FileForHandle(f);
    setvbuf(v1, nullptr, 4, 0);
}

// ea: 0x004B58E0
int FS_filelength(int f)
{
    ASSERT("f", "c:\\cod\\code\\game\\com_files.cpp", 478);
    FS_CheckFileSystemStarted();
    if (fsh[f].zipFile != 0)
        return fsh[f].fileSize;
    FILE* v2 = FS_FileForHandle(f);
    long v3 = ftell(v2);
    fseek(v2, 0, 2);
    long v4 = ftell(v2);
    fseek(v2, v3, 0);
    return (int)v4;
}

// ea: 0x004B59B0
void FS_Remove()
{
    AeAssert::gCurrentAuthor = AeAssert::COD3;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\com_files.cpp";
    AeAssert::gCurrentLine = 681;
    AeAssert::gCurrentExpr = nullptr;
    if (!AeAssert::IsIgnored() && AeAssert::Warning("not cross-platform"))
        __debugbreak();
}

// ea: 0x004B59F0
int FS_FilenameCompare(const char* s1, const char* s2)
{
    while (1)
    {
        int v3 = *s1;
        int v4 = *s2;
        ++s1;
        ++s2;
        if (Q_islower(v3) != 0)
            v3 -= 32;
        if (Q_islower(v4) != 0)
            v4 -= 32;
        if (v3 == 92 || v3 == 58)
            v3 = 47;
        if (v4 == 92 || v4 == 58)
            v4 = 47;
        if (v3 != v4)
            break;
        if (v3 == 0)
            return 0;
    }
    return -1;
}

// ea: 0x004B5A70
char* FS_ShiftedStrStr(const char* string, const char* substring, char shift)
{
    char buf[256];
    const char* v3 = substring;
    char v4 = *substring;
    int i = 0;
    for (; v4 != 0; ++v3)
    {
        buf[i] = shift + v4;
        v4 = v3[1];
        ++i;
    }
    buf[i] = 0;
    return (char*)strstr(string, buf);
}

// ea: 0x004B5AC0
const char* FS_GetExtensionSubString(const char* filename)
{
    const char* v1 = filename;
    char v2 = *filename;
    const char* result = defaultFileName;
    for (; v2 != 0; ++v1)
    {
        if (v2 == 46)
        {
            result = v1 + 1;
        }
        else if (v2 == 47 || v2 == 92)
        {
            result = defaultFileName;
        }
        v2 = v1[1];
    }
    return result;
}

// ea: 0x004B5B00
int FS_PureIgnoresExtension(const char* extension)
{
    const char* v1 = extension;
    if (*extension == 46)
        v1 = extension + 1;
    return _stricmp(v1, "cfg") == 0
        || Q_stricmp(v1, "menu") == 0
        || Q_stricmp(v1, "dat") == 0;
}

// ea: 0x004B5B50
void FS_ResetFiles()
{
    fs_loadStack = 0;
}

// ea: 0x004B5C00
void FS_AddNonPackFileDirectory()
{
    for (searchpath_s* i = fs_searchpaths; i != nullptr; i = i->next)
    {
        if (i->pack == nullptr)
        {
            ASSERT("0", "c:\\cod\\code\\game\\com_files.cpp", 2370);
        }
    }
}

// ea: 0x004B5C70
fileData_s* FS_GetDataForFile(const char* path, const char* filename,
                              const char* extension)
{
    char name[1024];
    sprintf(name, "%s/%s%s", path, filename, extension);
    if (strlen(name) >= 0x400)
    {
        ASSERT("strlen(name) < 1024", "c:\\cod\\code\\game\\com_files.cpp",
               2405);
    }
    searchpath_s* v3 = fs_memorysearchpaths;
    if (fs_memorysearchpaths != nullptr)
    {
        while (1)
        {
            if (v3->bLocalized == 0
                || (fs_ignoreLozalized->integer == 0
                    && v3->language == SEH_GetCurrentLanguage()))
            {
                pack_t* pack = v3->pack;
                if (pack != nullptr)
                {
                    fileInPack_s* v5 =
                        pack->hashTable[FS_HashFileName(name, pack->hashSize)];
                    if (v5 != nullptr)
                    {
                        while (FS_FilenameCompare(v5->name, name) != 0)
                        {
                            v5 = v5->next;
                            if (v5 == nullptr)
                                goto nextSearch;
                        }
                        return &v5->data;
                    }
                }
            }
        nextSearch:
            v3 = v3->next;
            if (v3 == nullptr)
                goto nonPack;
        }
    }
    else
    {
    nonPack:
        sprintf(name, "%s%s", filename, extension);
        if (strlen(name) >= 0x400)
        {
            ASSERT("strlen(name) < 1024",
                   "c:\\cod\\code\\game\\com_files.cpp", 2431);
        }
        filelist_s* v6 = fs_memorynonpackfilelist;
        if (fs_memorynonpackfilelist != nullptr)
        {
            while (1)
            {
                if (_stricmp(v6->dir, path) == 0)
                {
                    fileInList_s* v7 =
                        v6->hashTable[FS_HashFileName(name, v6->hashSize)];
                    if (v7 != nullptr)
                    {
                        while (FS_FilenameCompare(v7->data.name, name) != 0)
                        {
                            v7 = v7->next;
                            if (v7 == nullptr)
                                goto nextList;
                        }
                        return &v7->data;
                    }
                }
            nextList:
                v6 = v6->next;
                if (v6 == nullptr)
                    return nullptr;
            }
        }
        else
        {
            return nullptr;
        }
    }
}

// ea: 0x004B5E80
void FS_ClearDataForFiles(void* start, void* end)
{
    searchpath_s* search = fs_memorysearchpaths;
    if (fs_memorysearchpaths != nullptr)
    {
        searchpath_s* v2 = search;
        do
        {
            v2 = search;
            pack_t* pack = search->pack;
            if (pack != nullptr)
            {
                pack_t* pack2 = pack;
                for (int i = 0; i < pack->hashSize; ++i)
                {
                    fileInPack_s* v5 = pack2->hashTable[i];
                    if (v5 != nullptr)
                    {
                        do
                        {
                            unsigned int data = (unsigned int)v5->data.data;
                            if (data >= (unsigned int)start
                                && data < (unsigned int)end)
                            {
                                if (v5->data.mem_heap_free != nullptr)
                                {
                                    v5->data.mem_heap_free(&v5->data);
                                    v5->data.mem_heap_free = nullptr;
                                }
                                v5->data.data = nullptr;
                            }
                            v5 = v5->next;
                        }
                        while (v5 != nullptr);
                        v2 = search;
                    }
                    pack2 = v2->pack;
                }
            }
            search = v2->next;
        }
        while (v2->next != nullptr);
    }
    for (filelist_s* j = fs_memorynonpackfilelist; j != nullptr; j = j->next)
    {
        for (int v9 = 0; v9 < j->hashSize; ++v9)
        {
            fileInList_s* v10 = j->hashTable[v9];
            if (v10 != nullptr)
            {
                do
                {
                    unsigned int v11 = (unsigned int)v10->data.data;
                    if (v11 >= (unsigned int)start && v11 < (unsigned int)end)
                    {
                        if (v10->data.mem_heap_free != nullptr)
                        {
                            v10->data.mem_heap_free(&v10->data);
                            v10->data.mem_heap_free = nullptr;
                        }
                        v10->data.data = nullptr;
                    }
                    v10 = v10->next;
                }
                while (v10 != nullptr);
            }
        }
    }
}

// ea: 0x004B5F80
int FS_FTell(int f)
{
    if (fsh[f].zipFile != 0)
        return (int)ftell((FILE*)fsh[f].handleFiles.file.file);
    return (int)ftell((FILE*)fsh[f].handleFiles.file.file);
}

// ea: 0x004B5FC0
void FS_Flush(int f)
{
    fflush((FILE*)fsh[f].handleFiles.file.file);
}

// ============================================================================
// Statics (com_files.cpp internal helpers)
// ============================================================================

static int FS_HandleForFile(int streamThread);
static void FS_BuildOSPath_Internal(const char* base, const char* game,
                                    const char* qpath, char* ospath,
                                    int streamThread);
static int FS_FOpenFileRead_Internal(const char* filename, int* file,
                                     int uniqueFILE, int streamThread);
static void FS_ShutdownSearchPaths(searchpath_s* p);
static void FS_ShutdownFileLists(filelist_s* p);
static void FS_AddGameDirectory(const char* dir, const char* path,
                                int bLanguageDirectory, int iLanguage);
static void FS_AddLocalizedGameDirectory(const char* dir, const char* path);
static int FS_AddFileToList(char* name, char* list[], int nfiles);
static int FS_ReturnPath(const char* zname, char* zpath, int* depth);

// ea: 0x004C66A0
static void FS_BuildOSPath_Internal(const char* base, const char* game,
                                    const char* qpath, char* ospath,
                                    int streamThread)
{
    ASSERT("base", "c:\\cod\\code\\game\\com_files.cpp", 532);
    ASSERT("qpath", "c:\\cod\\code\\game\\com_files.cpp", 533);
    ASSERT("ospath", "c:\\cod\\code\\game\\com_files.cpp", 534);
    const char* v5 = game;
    if (game == nullptr || *game == 0)
    {
        game = fs_gamedir;
        v5 = fs_gamedir;
    }
    unsigned int v6 = (unsigned int)strlen(base);
    unsigned int v7 = (unsigned int)strlen(v5);
    unsigned int lenQpath = (unsigned int)strlen(qpath);
    if (v7 + lenQpath + v6 + 2 >= 128)
    {
        if (streamThread != 0)
        {
            *ospath = 0;
            return;
        }
        Com_Error(0, "FS_BuildOSPath: os path length exceeded MAX_OSPATH");
    }
    memcpy(ospath, base, v6);
    char* v8 = &ospath[v6];
    ospath[v6] = 47;
    memcpy(&ospath[v6 + 1], game, v7);
    if (v7 != 0)
        ospath[v7 + 1 + v6] = 47;
    else
        v7 = -1;
    memcpy(&ospath[v6 + 2 + v7], qpath, lenQpath + 1);
    if (*v8 != 0)
    {
        do
        {
            if (*v8 == 47 || *v8 == 92)
                *v8 = 92;
            char v9 = *++v8;
            if (v9 == 0)
                break;
        }
        while (1);
    }
}

// ea: 0x004C6890
void FS_BuildOSPath(const char* base, const char* game, const char* qpath,
                    char* ospath)
{
    FS_BuildOSPath_Internal(base, game, qpath, ospath, 0);
}

// ea: 0x004C6E70
static int FS_FOpenFileRead_Internal(const char* filename, int* file,
                                     int uniqueFILE, int streamThread)
{
    ASSERT("filename", "c:\\cod\\code\\game\\com_files.cpp", 1060);
    FS_CheckFileSystemStarted();
    if (file != nullptr)
    {
        if (strstr(filename, "..") != nullptr || strstr(filename, "::") != nullptr)
        {
            *file = 0;
            return -1;
        }
        int v15 = FS_HandleForFile(streamThread);
        *file = v15;
        searchpath_s* v16 = fs_searchpaths;
        fsh[v15].handleFiles.unique = uniqueFILE;
        if (fs_searchpaths != nullptr)
        {
            do
            {
                if (v16->bLocalized == 0
                    || (fs_ignoreLozalized->integer == 0
                        && v16->language == SEH_GetCurrentLanguage()))
                {
                    pack_t* pack = v16->pack;
                    int hash = 0;
                    if (pack != nullptr)
                        hash = FS_HashFileName(filename, pack->hashSize);
                    if (strstr(filename, ".bsp") != nullptr)
                    {
                        if (pack != nullptr && pack->hashTable[hash] != nullptr)
                        {
                            Q_strncpyz(fs_bsp_gamedir, pack->pakGamename, 128);
                        }
                        else
                        {
                            directory_t* dir = v16->dir;
                            if (dir != nullptr)
                                Q_strncpyz(fs_bsp_gamedir, dir->gamedir, 128);
                        }
                    }
                    if (pack != nullptr && pack->hashTable[hash] != nullptr)
                    {
                        ASSERT("0", "c:\\cod\\code\\game\\com_files.cpp", 1162);
                    }
                    else if (v16->dir != nullptr)
                    {
                        const char* ExtensionSubString =
                            FS_GetExtensionSubString(filename);
                        if ((fs_restrict->integer == 0 && fs_numServerPaks == 0)
                            || v16->bLocalized != 0
                            || FS_PureIgnoresExtension(ExtensionSubString) != 0)
                        {
                            const char* path = v16->dir->path;
                            char netpath[128];
                            FS_BuildOSPath_Internal(path, path + 128, filename,
                                                    netpath, streamThread);
                            fsh[*file].handleFiles.file.file =
                                fopen(netpath, "rb");
                            if (fsh[*file].handleFiles.file.file != nullptr)
                            {
                                Q_strncpyz(fsh[*file].name, filename, 256);
                                fsh[*file].zipFile = 0;
                                if (fs_debug->integer != 0 && streamThread == 0)
                                {
                                    Com_Printf(
                                        "FS_FOpenFileRead: %s (found in '%s/%s')\n",
                                        filename, path, path + 128);
                                }
                                if (fs_copyfiles->integer != 0
                                    && Q_stricmp(path, fs_cdpath->string) == 0)
                                {
                                    char copypath[128];
                                    FS_BuildOSPath_Internal(
                                        fs_basepath->string, path + 128,
                                        filename, copypath, streamThread);
                                    FS_CopyFile(netpath, copypath);
                                }
                                return FS_filelength(*file);
                            }
                        }
                    }
                }
                v16 = v16->next;
            }
            while (v16 != nullptr);
        }
        if (fs_debug->integer != 0 && streamThread == 0)
            Com_Printf("Can't find %s\n", filename);
        *file = 0;
        return -1;
    }
    searchpath_s* search = fs_searchpaths;
    if (fs_searchpaths == nullptr)
        return 0;
    searchpath_s* v5 = fs_searchpaths;
    while (1)
    {
        if (v5->bLocalized != 0
            && (fs_ignoreLozalized->integer != 0
                || v5->language != SEH_GetCurrentLanguage()))
        {
            goto nextSearch;
        }
        pack_t* v6 = v5->pack;
        int hash = 0;
        if (v6 != nullptr)
            hash = FS_HashFileName(filename, v6->hashSize);
        pack_t* v7 = v5->pack;
        if (v7 != nullptr)
        {
            fileInPack_s* v8 = v7->hashTable[hash];
            if (v8 != nullptr)
            {
                while (FS_FilenameCompare(v8->name, filename) != 0)
                {
                    v8 = v8->next;
                    if (v8 == nullptr)
                    {
                        v5 = search;
                        goto nextSearch;
                    }
                }
                return 1;
            }
        }
        const char* v9 = v5->dir->path;
        if (v9 != nullptr)
        {
            char netpath[128];
            FS_BuildOSPath_Internal(v9, v9 + 128, filename, netpath,
                                    streamThread);
            FILE* v10 = fopen(netpath, "rb");
            if (v10 != nullptr)
            {
                fclose(v10);
                return 1;
            }
            if (strlen(netpath) >= 0x80)
            {
                ASSERT("strlen(netpath) < 128",
                       "c:\\cod\\code\\game\\com_files.cpp", 1095);
            }
            char copypath[128];
            unsigned int v11 = 0;
            char v12 = 0;
            do
            {
                v12 = netpath[v11];
                copypath[v11++] = v12;
            }
            while (v12 != 0);
            unsigned int v13 = 0;
            unsigned int searchLen = (unsigned int)strlen(copypath);
            if (searchLen != 0)
            {
                do
                {
                    if (copypath[v13] == 64)
                        copypath[v13] = 36;
                    ++v13;
                }
                while (v13 < strlen(copypath));
            }
            v10 = fopen(netpath, "rb");
            if (v10 != nullptr)
            {
                fclose(v10);
                return 1;
            }
        }
    nextSearch:
        v5 = v5->next;
        search = v5;
        if (v5 == nullptr)
            return 0;
    }
}

// ea: 0x004C73B0
int FS_FOpenFileReadStream(const char* filename, int* file, int uniqueFILE)
{
    return FS_FOpenFileRead_Internal(filename, file, uniqueFILE, 1);
}

// ea: 0x004C73D0
int FS_FOpenFileRead(const char* filename, int* file, int uniqueFILE)
{
    com_fileAccessed = 1;
    return FS_FOpenFileRead_Internal(filename, file, uniqueFILE, 0);
}

// ea: 0x004C7400
int FS_TouchFile(const char* name)
{
    com_fileAccessed = 1;
    int f;
    FS_FOpenFileRead_Internal(name, &f, 0, 0);
    int result = f;
    if (f != 0)
    {
        FS_FCloseFile(f);
        return 1;
    }
    return result;
}

// ea: 0x004C7440
char* FS_ShortOSFilePath(const char* filename)
{
    searchpath_s* v1 = fs_searchpaths;
    if (fs_searchpaths == nullptr)
        return nullptr;
    while (1)
    {
        if (v1->bLocalized == 0
            || (fs_ignoreLozalized->integer == 0
                && v1->language == SEH_GetCurrentLanguage()))
        {
            const char* path = v1->dir->path;
            if (path != nullptr)
            {
                char netpath[128];
                FS_BuildOSPath_Internal(path, path + 128, filename, netpath, 0);
                FILE* v3 = fopen(netpath, "rb");
                if (v3 != nullptr)
                {
                    fclose(v3);
                    return va("%s/%s", path + 128, filename);
                }
            }
        }
        v1 = v1->next;
        if (v1 == nullptr)
            return nullptr;
    }
}

// ea: 0x004C74F0
unsigned int FS_Read(unsigned char* buffer, unsigned int len, int f)
{
    FS_CheckFileSystemStarted();
    if (f == 0)
        return 0;
    if (fsh[f].zipFile != 0)
        return (unsigned int)fread(buffer, 1, len, (FILE*)fsh[f].handleFiles.file.file);
    unsigned int v6 = len;
    unsigned int tries = 0;
    if (len != 0)
    {
        unsigned char* buf = buffer;
        while (1)
        {
            unsigned int v7 =
                (unsigned int)fread(buf, 1, v6, (FILE*)fsh[f].handleFiles.file.file);
            unsigned int v8 = v7;
            if (v7 != 0)
            {
                if (v7 == (unsigned int)-1)
                    Com_Error(0, "FS_Read: -1 bytes read");
            }
            else
            {
                if (tries != 0)
                    return len - v6;
                tries = 1;
            }
            v6 -= v8;
            buf += v8;
            if (v6 == 0)
                return len;
        }
    }
    return len;
}

// ea: 0x004C75B0
int FS_Seek(int f, long offset, int origin)
{
    FS_CheckFileSystemStarted();
    if (fsh[f].zipFile != 0)
    {
        int v3 = offset;
        if (offset == 0)
        {
            if (origin == 2)
                return fseek((FILE*)fsh[f].handleFiles.file.file, 0, 0) >= 0 ? 0 : -1;
            if (origin == 0)
                return 0;
        }
        long v5 = ftell((FILE*)fsh[f].handleFiles.file.file);
        int v6;
        if (origin != 0)
        {
            if (origin == 1)
            {
                if (offset + FS_filelength(f) >= v5)
                {
                    v6 = offset - v5 + FS_filelength(f);
                }
                else
                {
                    fseek((FILE*)fsh[f].handleFiles.file.file, 0, 0);
                    v6 = offset + FS_filelength(f);
                }
                goto readSkip;
            }
            if (origin != 2)
            {
                AeAssert::gCurrentAuthor = AeAssert::COD3;
                AeAssert::gCurrentFile = "c:\\cod\\code\\game\\com_files.cpp";
                AeAssert::gCurrentLine = 1688;
                AeAssert::gCurrentExpr = nullptr;
                if (!AeAssert::IsIgnored()
                    && AeAssert::Warning(va("Bad origin %i in FS_Seek", origin)))
                    __debugbreak();
                return -1;
            }
            if (offset >= v5)
            {
                v3 = offset - v5;
            }
            else
            {
                fseek((FILE*)fsh[f].handleFiles.file.file, 0, 0);
            }
        }
        else if (offset != 0)
        {
            if (offset < 0)
            {
                fseek((FILE*)fsh[f].handleFiles.file.file, 0, 0);
                v6 = (int)(v5 + offset);
            readSkip:
                if (v6 == 0)
                    return 0;
                char foo[65536];
                while (1)
                {
                    int v7;
                    if (v6 >= 0x10000)
                    {
                        v7 = (int)FS_Read((unsigned char*)foo, 0x10000, f);
                        v6 -= 0x10000;
                    }
                    else
                    {
                        v7 = (int)FS_Read((unsigned char*)foo, v6, f);
                        v6 = 0;
                    }
                    if (v7 == 0)
                        break;
                    if (v6 == 0)
                        return 0;
                }
                return -1;
            }
        }
        else
        {
            ASSERT("offset != 0", "c:\\cod\\code\\game\\com_files.cpp", 1632);
        }
        v6 = v3;
        goto readSkip;
    }
    FILE* v9 = FS_FileForHandle(f);
    switch (origin)
    {
    case 0:
        return fseek(v9, offset, SEEK_CUR);
    case 1:
        return fseek(v9, offset, SEEK_END);
    case 2:
        return fseek(v9, offset, SEEK_SET);
    default:
        break;
    }
    AeAssert::gCurrentAuthor = AeAssert::COD3;
    AeAssert::gCurrentFile = "c:\\cod\\code\\game\\com_files.cpp";
    AeAssert::gCurrentLine = 1729;
    AeAssert::gCurrentExpr = nullptr;
    if (!AeAssert::IsIgnored()
        && AeAssert::Warning(va("Bad origin %i in FS_Seek", origin)))
        __debugbreak();
    return 0;
}

// ea: 0x004C78A0
int FS_ReadFile(char* qpath, void** buffer)
{
    FS_CheckFileSystemStarted();
    char* v2 = qpath;
    if (!qpath || !*qpath)
        Com_Error(0, "FS_ReadFile with empty name");
    int v3;
    char cfgpath[256];
    if (!strstr(qpath, ".cfg"))
    {
        v3 = 0;
        goto open;
    }
    v3 = 1;
    if (!com_journal || com_journal->integer != 2)
    {
        sprintf(cfgpath, "config\\%s", qpath);
        qpath = cfgpath;
        v2 = cfgpath;
    open:
        com_fileAccessed = 1;
        int h;
        int v9 = FS_FOpenFileRead_Internal(v2, &h, 0, 0);
        int v10 = h;
        int len = v9;
        if (h)
        {
            if (buffer)
            {
                ++fs_loadStack;
                void* v11 = mem_heap_malloc_ctx(16, v9 + 1, "fs",
                                                "c:\\cod\\code\\game\\com_files.cpp", 1857);
                *buffer = v11;
                FS_Read((unsigned char*)v11, len, v10);
                ((char*)v11)[len] = 0;
                FS_FCloseFile(v10);
                if (v3 && com_journal && com_journal->integer == 1)
                {
                    Com_DPrintf("Writing %s to journal file.\n", qpath);
                    FS_Write((char*)&len, 4, com_journalDataFile);
                    FS_Write((char*)v11, len, com_journalDataFile);
                    FS_Flush(com_journalDataFile);
                }
                return len;
            }
            else
            {
                if (v3 && com_journal && com_journal->integer == 1)
                {
                    Com_DPrintf("Writing len for %s to journal file.\n", v2);
                    FS_Write((char*)&len, 4, com_journalDataFile);
                    FS_Flush(com_journalDataFile);
                }
                FS_FCloseFile(v10);
                return len;
            }
        }
        else
        {
            if (buffer)
                *buffer = (void*)h;
            if (v3 && com_journal && com_journal->integer == 1)
            {
                Com_DPrintf("Writing zero for %s to journal file.\n", v2);
                len = 0;
                FS_Write((char*)&len, 4, com_journalDataFile);
                FS_Flush(com_journalDataFile);
            }
            return -1;
        }
    }
    Com_DPrintf("Loading %s from journal file.\n", qpath);
    int len;
    if (FS_Read((unsigned char*)&len, 4, com_journalDataFile) != 4)
    {
        if (!buffer)
            return -1;
        *buffer = nullptr;
        return -1;
    }
    int result = len;
    if (len)
    {
        if (buffer)
        {
            void* v6 = mem_heap_malloc_ctx(16, len + 1, "fs",
                                           "c:\\cod\\code\\game\\com_files.cpp", 1796);
            *buffer = v6;
            int v8 = FS_Read((unsigned char*)v6, len, com_journalDataFile);
            if (v8 != len)
                Com_Error(0, "EXE_ERR_JOURNAL_FILE_READ");
            ++fs_loadStack;
            ((char*)v6)[len] = 0;
            return len;
        }
    }
    else
    {
        if (buffer)
        {
            *buffer = nullptr;
            return -1;
        }
        return 1;
    }
    return result;
}

// ea: 0x004C7B60
void FS_FreeFile(void* buffer)
{
    FS_CheckFileSystemStarted();
    if (buffer == nullptr)
        Com_Error(0, "FS_FreeFile( NULL )");
    --fs_loadStack;
    mem_heap_free(buffer);
}

// ea: 0x004BC8C0
void FS_PureServerSetLoadedPaks()
{
}

// ea: 0x004BC8D0
void FS_PureServerSetReferencedPaks()
{
}

// ea: 0x004BDAD0
int FS_CreatePath(char* OSPath)
{
    if (strstr(OSPath, "..") != nullptr || strstr(OSPath, "::") != nullptr)
    {
        Com_Printf("WARNING: refusing to create relative path \"%s\"\n", OSPath);
        return 1;
    }
    char* v1 = OSPath + 1;
    if (OSPath[1] != 0)
    {
        do
        {
            if (*v1 == 92)
            {
                *v1 = 0;
                Sys_Mkdir(OSPath);
                *v1 = 92;
            }
            char v2 = *++v1;
            if (v2 == 0)
                break;
        }
        while (1);
    }
    return 0;
}

// ea: 0x004BDB40
void FS_FCloseFile(int f)
{
    FS_CheckFileSystemStarted();
    if (fsh[f].zipFile != 0)
    {
        if (fsh[f].streamed != 0)
            fclose((FILE*)fsh[f].handleFiles.file.file);
        Com_Memset(&fsh[f], 0, 0x120u);
    }
    else
    {
        FILE* o = (FILE*)fsh[f].handleFiles.file.file;
        if (o != nullptr)
            fclose(o);
        Com_Memset(&fsh[f], 0, 0x120u);
    }
}

// ea: 0x004BDBD0
unsigned int FS_Write(char* buffer, unsigned int len, int h)
{
    FS_CheckFileSystemStarted();
    if (h == 0)
        return h;
    unsigned int v4 = len;
    FILE* v6 = FS_FileForHandle(h);
    unsigned int tries = 0;
    if (len == 0)
    {
        if (fsh[h].streamed != 0)
            fflush(v6);
        return len;
    }
    unsigned int v7;
    while (1)
    {
        v7 = (unsigned int)fwrite(buffer, 1, v4, v6);
        if (v7 != 0)
            break;
        if (tries != 0)
            return 0;
        tries = 1;
    writeMore:
        v4 -= v7;
        buffer += v7;
        if (v4 == 0)
        {
            if (fsh[h].streamed != 0)
                fflush(v6);
            return len;
        }
    }
    if (v7 != (unsigned int)-1)
        goto writeMore;
    Com_Printf("FS_Write: -1 bytes written\n");
    return 0;
}

// ea: 0x004BDC70
void FS_Printf(int h, const char* fmt, ...)
{
    char msg[4096];
    va_list ap;
    va_start(ap, fmt);
    vsprintf(msg, fmt, ap);
    va_end(ap);
    FS_Write(msg, (unsigned int)strlen(msg), h);
}

// ea: 0x004BDCD0
void FS_FreeFileList(char** list)
{
    FS_CheckFileSystemStarted();
    if (list != nullptr)
    {
        char* v1 = *list;
        if (*list != nullptr)
        {
            char** v2 = list;
            do
            {
                mem_heap_free(v1);
                v1 = v2[1];
                ++v2;
            }
            while (v1 != nullptr);
        }
        mem_heap_free(list);
    }
}

// ea: 0x004BDDB0
void FS_ClearMemory()
{
    if (fs_memorysearchpaths != fs_searchpaths)
    {
        FS_ShutdownSearchPaths(fs_memorysearchpaths);
        fs_memorysearchpaths = fs_searchpaths;
    }
    if (fs_memorynonpackfilelist != fs_nonpackfilelist)
    {
        FS_ShutdownFileLists(fs_memorynonpackfilelist);
        fs_memorynonpackfilelist = fs_nonpackfilelist;
    }
}

// ea: 0x004BDDF0
void FS_Shutdown(int closemfp)
{
    int v1 = 1;
    while (v1 <= 3)
    {
        if (fsh[v1].handleFiles.file.file != nullptr)
            FS_FCloseFile(v1);
        ++v1;
    }
    if (closemfp != 0)
    {
        ASSERT("fs_memorysearchpaths == fs_searchpaths",
               "c:\\cod\\code\\game\\com_files.cpp", 2623);
        ASSERT("fs_memorynonpackfilelist == fs_nonpackfilelist",
               "c:\\cod\\code\\game\\com_files.cpp", 2624);
        FS_ShutdownSearchPaths(fs_searchpaths);
        filelist_s* v3 = fs_nonpackfilelist;
        filelist_s* next = nullptr;
        if (fs_nonpackfilelist != nullptr)
        {
            do
            {
                next = v3->next;
                mem_heap_free(v3->buildBuffer);
                mem_heap_free(v3);
                v3 = next;
            }
            while (next != nullptr);
        }
        fs_memorysearchpaths = nullptr;
        fs_memorynonpackfilelist = nullptr;
    }
    else
    {
        if (fs_memorysearchpaths != fs_searchpaths)
            FS_ShutdownSearchPaths(fs_searchpaths);
        if (fs_memorynonpackfilelist != fs_nonpackfilelist)
            FS_ShutdownFileLists(fs_nonpackfilelist);
    }
    fs_searchpaths = nullptr;
    fs_nonpackfilelist = nullptr;
    Cmd_RemoveCommand("path");
    Cmd_RemoveCommand("fullpath");
    Cmd_RemoveCommand("dir");
    Cmd_RemoveCommand("fdir");
    Cmd_RemoveCommand("touchFile");
}

// ea: 0x004C65D0
static int FS_HandleForFile(int streamThread)
{
    int v1, v2;
    if (streamThread != 0)
    {
        v1 = 3;
        v2 = 0;
    }
    else
    {
        v1 = 1;
        v2 = 2;
    }
    int v3 = 0;
    if (v2 <= 0)
        goto noHandles;
    fileHandleData_t* v4 = &fsh[v1];
    while (v4->handleFiles.file.file != nullptr)
    {
        ++v3;
        ++v4;
        if (v3 >= v2)
            goto noHandles;
    }
    return v3 + v1;
noHandles:
    if (streamThread != 0)
    {
        ASSERT("!streamThread", "c:\\cod\\code\\game\\com_files.cpp", 433);
    }
    int v5 = 1;
    fileHandleData_t* v6 = &fsh[1];
    do
    {
        Com_Printf("FILE %2i: '%s'\n", v5, v6->name);
        ++v6;
        ++v5;
    }
    while (v5 <= 3);
    Com_Error(1, "FS_HandleForFile: none free");
    return -1;
}

// ea: 0x004C68B0
void FS_CopyFile(char* fromOSPath, char* toOSPath)
{
    if (strstr(fromOSPath, "journal.dat") == nullptr
        && strstr(fromOSPath, "journaldata.dat") == nullptr)
    {
        FILE* v2 = fopen(fromOSPath, "rb");
        FILE* v3 = v2;
        if (v2 != nullptr)
        {
            fseek(v2, 0, 2);
            unsigned int v4 = (unsigned int)ftell(v3);
            fseek(v3, 0, 0);
            void* v5 = mem_heap_malloc(v4);
            if (fread(v5, 1, v4, v3) != v4)
                Com_Error(0, "FS_CopyFile: read failed");
            fclose(v3);
            if (FS_CreatePath(toOSPath) == 0)
            {
                FILE* v6 = fopen(toOSPath, "wb");
                FILE* v7 = v6;
                if (v6 != nullptr)
                {
                    if (fwrite(v5, 1, v4, v6) != v4)
                        Com_Error(0, "FS_CopyFile: write failed");
                    fclose(v7);
                    mem_heap_free(v5);
                }
                else
                {
                    mem_heap_free(v5);
                }
            }
            else
            {
                mem_heap_free(v5);
            }
        }
    }
}

// ea: 0x004C69B0
int FS_FileExists(const char* file)
{
    char testpath[128];
    FS_BuildOSPath_Internal(fs_homepath->string, fs_gamedir, file, testpath, 0);
    FILE* v1 = fopen(testpath, "rb");
    if (v1 == nullptr)
        return 0;
    fclose(v1);
    return 1;
}

// ea: 0x004C6A10
int FS_FOpenFileWrite(const char* filename)
{
    FS_CheckFileSystemStarted();
    int v1 = FS_HandleForFile(0);
    int v3 = v1;
    fsh[v1].zipFile = 0;
    char ospath[128];
    FS_BuildOSPath_Internal(fs_homepath->string, fs_gamedir, filename, ospath, 0);
    if (fs_debug->integer != 0)
        Com_Printf("FS_FOpenFileWrite: %s\n", ospath);
    if (FS_CreatePath(ospath) != 0)
        return 0;
    fsh[v3].handleFiles.file.file = fopen(ospath, "wb");
    Q_strncpyz(fsh[v3].name, filename, 256);
    fsh[v3].streamed = 0;
    if (fsh[v3].handleFiles.file.file == nullptr)
        return 0;
    return v3;
}

// ea: 0x004C6AE0
int FS_FOpenTextFileWrite(const char* filename)
{
    FS_CheckFileSystemStarted();
    int v1 = FS_HandleForFile(0);
    int v3 = v1;
    fsh[v1].zipFile = 0;
    char ospath[128];
    FS_BuildOSPath_Internal(fs_homepath->string, fs_gamedir, filename, ospath, 0);
    if (fs_debug->integer != 0)
        Com_Printf("FS_FOpenFileWrite: %s\n", ospath);
    if (FS_CreatePath(ospath) != 0)
        return 0;
    sprintf(ospath, "d:\\uo\\console.log");
    Com_Printf("XBOX write hack\n");
    fsh[v3].handleFiles.file.file = fopen(ospath, "wt");
    Q_strncpyz(fsh[v3].name, filename, 256);
    fsh[v3].streamed = 0;
    if (fsh[v3].handleFiles.file.file == nullptr)
        return 0;
    return v3;
}

// ea: 0x004C6BD0
int FS_FOpenFileAppend(const char* filename)
{
    FS_CheckFileSystemStarted();
    int v1 = FS_HandleForFile(0);
    fsh[v1].zipFile = 0;
    Q_strncpyz(fsh[v1].name, filename, 256);
    char ospath[128];
    FS_BuildOSPath_Internal(fs_homepath->string, fs_gamedir, filename, ospath, 0);
    if (fs_debug->integer != 0)
        Com_Printf("FS_FOpenFileAppend: %s\n", ospath);
    if (FS_CreatePath(ospath) != 0)
        return 0;
    FILE* v3 = fopen(ospath, "ab");
    fsh[v1].handleFiles.file.file = v3;
    fsh[v1].streamed = 0;
    if (v3 == nullptr)
        return 0;
    return v1;
}

// ea: 0x004C6CA0
int FS_FileCompare(const char* s1, const char* s2)
{
    FILE* v2 = fopen(s1, "rb");
    if (v2 == nullptr)
        Com_Error(0, "FS_FileCompare: can't open %s", s1);
    FILE* v3 = fopen(s2, "rb");
    if (v3 != nullptr)
    {
        long posa = ftell(v2);
        fseek(v2, 0, 2);
        int v5 = (int)ftell(v2);
        fseek(v2, posa, 0);
        long posb = ftell(v3);
        fseek(v3, 0, 2);
        int len2 = (int)ftell(v3);
        fseek(v3, posb, 0);
        if (v5 == len2)
        {
            void* v6 = mem_heap_malloc(v5);
            void* buffer = v6;
            if (v6 == nullptr && v5 > 0)
                Sys_OutOfMemError();
            Com_Memset(v6, 0, v5);
            if (fread(buffer, 1, v5, v2) != (unsigned int)v5)
                Com_Error(0, "FS_FileCompare: read failed");
            fclose(v2);
            void* v7 = mem_heap_malloc(len2);
            void* pos = v7;
            if (v7 == nullptr && len2 > 0)
                Sys_OutOfMemError();
            Com_Memset(v7, 0, len2);
            if (fread(pos, 1, len2, v3) != (unsigned int)len2)
                Com_Error(0, "FS_FileCompare: read failed");
            fclose(v3);
            int v8 = 0;
            unsigned char* v9 = (unsigned char*)buffer;
            if (v5 <= 0)
                goto equal;
            while (*v9 == ((unsigned char*)pos)[v9 - (unsigned char*)buffer])
            {
                ++v8;
                ++v9;
                if (v8 >= v5)
                    goto equal;
            }
            mem_heap_free(buffer);
            mem_heap_free(pos);
            return 0;
        equal:
            mem_heap_free(buffer);
            mem_heap_free(pos);
            return 1;
        }
        else
        {
            fclose(v2);
            fclose(v3);
            return 0;
        }
    }
    else
    {
        fclose(v2);
        return 0;
    }
}

// ea: 0x004C7BA0
char** FS_ListFilteredFiles(const char* path, const char* extension,
                            char* filter, int* numfiles)
{
    FS_CheckFileSystemStarted();
    char* list[4096];
    const char* v4 = path;
    if (path != nullptr)
    {
        if (extension == nullptr)
            extension = defaultFileName;
        if (*path == 47 || *path == 92)
            v4 = ++path;
        int v6 = Q_stricmp(extension, "/") == 0;
        int bDirSearch = v6;
        int nfiles = 0;
        int depth;
        char zpath[128];
        FS_ReturnPath(v4, zpath, &depth);
        searchpath_s* v7 = fs_searchpaths;
        searchpath_s* search = fs_searchpaths;
        if (fs_searchpaths != nullptr)
        {
            while (1)
            {
                if (v7->bLocalized == 0
                    || (fs_ignoreLozalized->integer == 0
                        && v7->language == SEH_GetCurrentLanguage()))
                {
                    if (v7->pack != nullptr)
                    {
                        ASSERT("0", "c:\\cod\\code\\game\\com_files.cpp", 2040);
                    }
                    else if (v7->dir != nullptr
                             && (fs_restrict->integer == 0 && fs_numServerPaks == 0
                                 || Q_stricmp(extension, "svg") == 0))
                    {
                        char netpath[128];
                        FS_BuildOSPath_Internal(v7->dir->path, v7->dir->gamedir,
                                                v4, netpath, 0);
                        int numSysFiles;
                        char** sysFiles = Sys_ListFiles(netpath, extension,
                                                        filter, &numSysFiles, v6);
                        int v8 = 0;
                        if (numSysFiles > 0)
                        {
                            do
                            {
                                nfiles = FS_AddFileToList(sysFiles[v8++], list,
                                                          nfiles);
                            }
                            while (v8 < numSysFiles);
                            v6 = bDirSearch;
                            v7 = search;
                        }
                        Sys_FreeFileList(sysFiles);
                        v4 = path;
                    }
                }
                search = v7->next;
                if (search == nullptr)
                    break;
                v7 = search;
            }
        }
        int v9 = nfiles;
        *numfiles = nfiles;
        if (nfiles == 0)
            return nullptr;
        char** v11 = (char**)mem_heap_malloc(4 * v9 + 4);
        if (v11 == nullptr && 4 * v9 + 4 > 0)
            Sys_OutOfMemError();
        Com_Memset(v11, 0, 4 * v9 + 4);
        int v12 = 0;
        if (v9 > 0)
        {
            v12 = nfiles;
            memcpy(v11, list, 4 * v9);
        }
        v11[v12] = nullptr;
        return v11;
    }
    *numfiles = 0;
    return nullptr;
}

// ea: 0x004C7DE0
char** FS_ListFiles(const char* path, const char* extension, int* numfiles)
{
    return FS_ListFilteredFiles(path, extension, nullptr, numfiles);
}

// ea: 0x004C8090
void FS_Startup(const char* gameName)
{
    Com_Printf("----- FS_Startup -----\n");
    fs_debug = Cvar_Get("fs_debug", "0", 0);
    fs_copyfiles = Cvar_Get("fs_copyfiles", "0", 16);
    const char* v1 = Sys_DefaultCDPath();
    fs_cdpath = Cvar_Get("fs_cdpath", v1, 16);
    const char* v2 = Sys_DefaultInstallPath();
    fs_basepath = Cvar_Get("fs_basepath", v2, 16);
    fs_basegame = Cvar_Get("fs_basegame", defaultFileName, 16);
    char* string = (char*)Sys_DefaultHomePath();
    if (string == nullptr || *string == 0)
        string = fs_basepath->string;
    fs_homepath = Cvar_Get("fs_homepath", string, 16);
    fs_gamedirvar = Cvar_Get("fs_game", defaultFileName, 24);
    fs_restrict = Cvar_Get("fs_restrict", defaultFileName, 16);
    fs_ignoreLozalized = Cvar_Get("fs_ignoreLozalized", "0", 544);
    if (*fs_cdpath->string != 0)
        FS_AddLocalizedGameDirectory(fs_cdpath->string, gameName);
    if (*fs_basepath->string != 0)
        FS_AddLocalizedGameDirectory(fs_basepath->string, gameName);
    if (*fs_basepath->string != 0
        && Q_stricmp(fs_homepath->string, fs_basepath->string) != 0)
    {
        FS_AddLocalizedGameDirectory(fs_homepath->string, gameName);
    }
    if (*fs_basegame->string != 0
        && Q_stricmp(gameName, defaultFileName) == 0
        && Q_stricmp(fs_basegame->string, gameName) != 0)
    {
        if (*fs_cdpath->string != 0)
            FS_AddLocalizedGameDirectory(fs_cdpath->string, fs_basegame->string);
        if (*fs_basepath->string != 0)
            FS_AddLocalizedGameDirectory(fs_basepath->string, fs_basegame->string);
        char* v4 = fs_homepath->string;
        if (*v4 != 0 && Q_stricmp(v4, fs_basepath->string) != 0)
            FS_AddLocalizedGameDirectory(fs_homepath->string, fs_basegame->string);
    }
    if (*fs_gamedirvar->string != 0
        && Q_stricmp(gameName, defaultFileName) == 0
        && Q_stricmp(fs_gamedirvar->string, gameName) != 0)
    {
        if (*fs_cdpath->string != 0)
            FS_AddLocalizedGameDirectory(fs_cdpath->string, fs_gamedirvar->string);
        if (*fs_basepath->string != 0)
            FS_AddLocalizedGameDirectory(fs_basepath->string, fs_gamedirvar->string);
        char* v5 = fs_homepath->string;
        if (*v5 != 0 && Q_stricmp(v5, fs_basepath->string) != 0)
            FS_AddLocalizedGameDirectory(fs_homepath->string, fs_gamedirvar->string);
    }
    fs_gamedirvar->modified = 0;
    Com_Printf("----------------------\n");
}

// ea: 0x004C8390
void FS_Restart(int checksumFeed)
{
    FS_Shutdown(0);
    fs_checksumFeed = checksumFeed;
    FS_Startup(defaultFileName);
    if (FS_ReadFile("default.cfg", nullptr) <= 0)
    {
        if (lastValidBase[0])
        {
            Cvar_Set2("fs_basepath", lastValidBase, 1);
            Cvar_Set2("fs_gamedirvar", lastValidGame, 1);
            lastValidBase[0] = 0;
            lastValidGame[0] = 0;
            Cvar_Set2("fs_restrict", "0", 1);
            FS_Restart(checksumFeed);
            Com_Error(1, "Invalid game folder\n");
        }
        Com_Error(0, "Couldn't load %s.  Make sure Call of Duty is run from "
                     "the correct folder.", "default.cfg");
    }
    if (Q_stricmp(fs_gamedirvar->string, lastValidGame) && !Com_SafeMode())
    {
        Cbuf_AddText(va("exec %s\n", "bro.cfg"));
    }
    Q_strncpyz(lastValidBase, fs_basepath->string, 128);
    Q_strncpyz(lastValidGame, fs_gamedirvar->string, 128);
}

// ea: 0x004C84B0
int FS_FOpenFileByMode(const char* qpath, int* f, int mode)
{
    int v3 = 0;
    int Internal = 6969;
    switch (mode)
    {
    case 0:  // FS_READ
        com_fileAccessed = 1;
        Internal = FS_FOpenFileRead_Internal(qpath, f, 1, 0);
        break;
    case 1:  // FS_WRITE
    {
        int v5 = FS_FOpenFileWrite(qpath);
        Internal = 0;
        *f = v5;
        if (v5 == 0)
            Internal = -1;
        break;
    }
    case 2:  // FS_APPEND
    case 3:  // FS_APPEND_SYNC
        if (mode == 3)
            v3 = 1;
        {
            int v5 = FS_FOpenFileAppend(qpath);
            Internal = 0;
            *f = v5;
            if (v5 == 0)
                Internal = -1;
        }
        break;
    default:
        Com_Error(0, "FS_FOpenFileByMode: bad mode");
        break;
    }
    if (f != nullptr)
    {
        if (*f != 0)
        {
            fsh[*f].baseOffset =
                (int)ftell((FILE*)fsh[*f].handleFiles.file.file);
            fsh[*f].fileSize = Internal;
            fsh[*f].handleSync = 0;
        }
        fsh[*f].streamed = v3;
    }
    return Internal;
}

// ea: 0x004CA470
void FS_Dir_f()
{
    int ndirs;
    if (Cmd_Argc() < 2 || Cmd_Argc() > 3)
    {
        Com_Printf("usage: dir <directory> [extension]\n");
    }
    else
    {
        const char* v1;
        const char* v2;
        if (Cmd_Argc() == 2)
        {
            v1 = Cmd_Argv(1);
            v2 = defaultFileName;
        }
        else
        {
            v1 = Cmd_Argv(1);
            v2 = Cmd_Argv(2);
        }
        Com_Printf("Directory of %s %s\n", v1, v2);
        Com_Printf("---------------\n");
        char** v3 = FS_ListFilteredFiles(v1, v2, nullptr, &ndirs);
        int v4 = ndirs;
        int v5 = 0;
        for (char** i = v3; v5 < v4; ++v5)
            Com_Printf("%s\n", i[v5]);
        FS_FreeFileList(v3);
    }
}

// ea: 0x004CA530
void FS_TouchFile_f()
{
    if (Cmd_Argc() == 2)
    {
        const char* v1 = Cmd_Argv(1);
        com_fileAccessed = 1;
        int file;
        FS_FOpenFileRead_Internal(v1, &file, 0, 0);
        if (file != 0)
            FS_FCloseFile(file);
    }
    else
    {
        Com_Printf("Usage: touchFile <file>\n");
    }
}

// ea: 0x004CEB90
int FS_InitFilesystem()
{
    Com_StartupVariable("fs_cdpath");
    Com_StartupVariable("fs_basepath");
    Com_StartupVariable("fs_homepath");
    Com_StartupVariable("fs_game");
    Com_StartupVariable("fs_copyfiles");
    Com_StartupVariable("fs_restrict");
    Com_StartupVariable("fs_usewolf");
    Com_StartupVariable("cl_language");
    SEH_InitLanguage();
    FS_Startup(defaultFileName);
    if (FS_ReadFile("default.cfg", nullptr) <= 0)
        Com_Error(0, "Couldn't load %s.  Make sure Call of Duty is run from "
                     "the correct folder.", "default.cfg");
    Q_strncpyz(lastValidBase, fs_basepath->string, 128);
    Q_strncpyz(lastValidGame, fs_gamedirvar->string, 128);
    memset(fs_bsp_gamedir, 0, 128);
    return 0;
}

// ============================================================================
// com_files.cpp statics
// ============================================================================

// ea: 0x004BDD10
static void FS_ShutdownSearchPaths(searchpath_s* p)
{
    searchpath_s* v1 = p;
    searchpath_s* next = nullptr;
    if (p != nullptr)
    {
        do
        {
            pack_t* pack = v1->pack;
            next = v1->next;
            if (pack != nullptr)
            {
                mem_heap_free(pack->buildBuffer);
                mem_heap_free(pack);
            }
            if (v1->dir != nullptr)
                mem_heap_free(v1->dir);
            mem_heap_free(v1);
            v1 = next;
        }
        while (next != nullptr);
    }
}

// ea: 0x004BDD70
static void FS_ShutdownFileLists(filelist_s* p)
{
    filelist_s* v1 = p;
    filelist_s* next = nullptr;
    if (p != nullptr)
    {
        do
        {
            next = v1->next;
            mem_heap_free(v1->buildBuffer);
            mem_heap_free(v1);
            v1 = next;
        }
        while (next != nullptr);
    }
}

// ea: 0x004C7E00
static void FS_AddGameDirectory(const char* dir, const char* path,
                                int bLanguageDirectory, int iLanguage)
{
    char szGameFolder[128];
    if (bLanguageDirectory != 0)
    {
        const char* LanguageName = SEH_GetLanguageName(iLanguage);
        Com_sprintf(szGameFolder, 128, "%s_%s", dir, LanguageName);
    }
    else
    {
        Q_strncpyz(szGameFolder, dir, 128);
    }
    searchpath_s* v6 = fs_searchpaths;
    if (fs_searchpaths != nullptr)
    {
        while (1)
        {
            const char* v7 = v6->dir->path;
            if (v7 != nullptr && Q_stricmp(v7, path) == 0
                && Q_stricmp(v6->dir->gamedir, szGameFolder) == 0)
                break;
            v6 = v6->next;
            if (v6 == nullptr)
                goto addNew;
        }
        int bLocalized = v6->bLocalized;
        if (bLocalized != bLanguageDirectory)
        {
            const char* v10 = bLocalized == 0 ? "localized" : "non-localized";
            Com_Printf("WARNING: game folder %s/%s added as both localized & "
                       "non-localized. Using folder as %s\n",
                       path, szGameFolder, v10);
        }
        if (v6->bLocalized != 0 && v6->language != iLanguage)
        {
            Com_Printf("WARNING: game golder %s/%s re-added as localized "
                       "folder with different language\n",
                       path, szGameFolder);
        }
        return;
    }
addNew:
    if (bLanguageDirectory != 0)
    {
        char ospath[128];
        FS_BuildOSPath_Internal(path, szGameFolder, defaultFileName, ospath, 0);
        ospath[strlen(ospath) - 1] = 0;
        if (Sys_DirectoryHasContents(ospath) == 0)
            return;
    }
    else
    {
        Q_strncpyz(fs_gamedir, szGameFolder, 128);
    }
    searchpath_s* v11 = (searchpath_s*)mem_heap_malloc(0x14u);
    if (v11 == nullptr)
        Sys_OutOfMemError();
    Com_Memset(v11, 0, 0x14u);
    directory_t* v12 = (directory_t*)mem_heap_malloc(0x100u);
    if (v12 == nullptr)
        Sys_OutOfMemError();
    Com_Memset(v12, 0, 0x100u);
    v11->dir = v12;
    Q_strncpyz(v12->path, path, 128);
    Q_strncpyz(v11->dir->gamedir, szGameFolder, 128);
    if (bLanguageDirectory == 0 && iLanguage != 0)
    {
        ASSERT("bLanguageDirectory || (!bLanguageDirectory && !iLanguage)",
               "c:\\cod\\code\\game\\com_files.cpp", 2319);
    }
    v11->bLocalized = bLanguageDirectory;
    v11->language = iLanguage;
    if (bLanguageDirectory != 0 && fs_searchpaths != nullptr)
    {
        searchpath_s* v13 = fs_searchpaths;
        if (fs_searchpaths->next != nullptr)
        {
            searchpath_s* next = nullptr;
            do
            {
                next = v13->next;
                if (v13->next->bLocalized != 0)
                    break;
                v13 = v13->next;
            }
            while (next->next != nullptr);
        }
        v11->next = v13->next;
        v13->next = v11;
    }
    else
    {
        v11->next = fs_searchpaths;
        fs_searchpaths = v11;
    }
}

// ea: 0x004C8050
static void FS_AddLocalizedGameDirectory(const char* dir, const char* path)
{
    for (int i = 13; i >= 0; --i)
        FS_AddGameDirectory(path, dir, 1, i);
    FS_AddGameDirectory(path, dir, 0, 0);
}

// ea: 0x004C1EB0
static int FS_AddFileToList(char* name, char* list[], int nfiles)
{
    if (nfiles == 4095)
        return 4095;
    int v4 = 0;
    if (nfiles <= 0)
    {
        list[nfiles] = CopyStringInternal(name);
        return nfiles + 1;
    }
    while (Q_stricmp(name, list[v4]) != 0)
    {
        if (++v4 >= nfiles)
        {
            list[nfiles] = CopyStringInternal(name);
            return nfiles + 1;
        }
    }
    return nfiles;
}

// ea: 0x004B5B60
static int FS_ReturnPath(const char* zname, char* zpath, int* depth)
{
    int v3 = 0;
    *zpath = 0;
    char v4 = *zname;
    int result = 0;
    int i = 0;
    for (; v4 != 0; ++i)
    {
        if (v4 == 47 || v4 == 92)
        {
            result = i;
            ++v3;
        }
        v4 = zname[i + 1];
    }
    strcpy(zpath, zname);
    zpath[result] = 0;
    if (result + 1 == i)
        --v3;
    *depth = v3;
    return result;
}
