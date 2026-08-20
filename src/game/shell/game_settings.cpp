// ============================================================================
// game_settings.cpp - GameSettings (shell.o game_data.cpp family)
// Save/load/format/delete of the MP profile memory-unit bundle.
// ============================================================================

#include "game/shell/shell_types.h"
#include "game/platform_xbox/MemoryUnitManager.h"
#include "core/ae_fixed_string.h"

#include <new>
#include <string.h>
#include <stdio.h>

// StubData / SaveGameData come from sv_stubs.h via shell_types.h.
extern SaveGameData gSaveGameData[4];  // ?gSaveGameData@@3PAUSaveGameData@@A
extern FEManager g_femanager;        // ?g_femanager@@3UFEManager@@A
extern int currCl;                   // ?currCl@@3HA @ 0xF1579C
extern bool g_enableControllerTest;  // ?g_enableControllerTest@@3_NA
extern bool gCE;                     // ?gCE@@3_NA @ 0xF91714
extern void* mem_heap_malloc(int alignment, unsigned int size);  // core.o
extern void* mem_heap_malloc_ctx(unsigned int size, int alignment,
                                 const char* ctx, const char* file, int line);
extern void mem_heap_free(void* ptr);                            // core.o
extern const char defaultFileName[];  // ?defaultFileName

namespace LocalClient {
extern int ClientToPort(int client);  // ?ClientToPort@LocalClient@@YAHH@Z
}

// Minimal STBManager view (same pattern as loading_menu.cpp / igo_widgets.cpp)
class STBManager {
public:
    static STBManager* sInst;  // ?sInst@STBManager@@2PAV1@A @ 0xF00EA0
    const char* GetSTBString(const char* pszReference);  // core.o
    const char* GetSTBString(unsigned int hash);  // core.o
};

// ============================================================================
// GameSettings (672 bytes) - verified against IDA
// vtable: Callback (slot 0), ~GameSettings (slot 1)
// ============================================================================
class GameSettings : public MemoryUnitManager::Observer {
public:
    static GameSettings* sInst;                         // ?sInst@GameSettings@@2PAV1@A
    static GameSettings* CreateInst();                   // ?CreateInst@GameSettings@@SAXXZ
    MemoryUnitManager::Container container;  // +0x04 (648 bytes)
    SaveGameData* m_temp_buffer;             // +0x28C
    bool m_mc_has_save;                      // +0x290
    bool m_damaged_save;                     // +0x291
    char m_cur_name[12];                     // +0x292
    bool m_continued_without_saving;         // +0x29E
    // +0x29F pad -> 0x2A0

    GameSettings();                                          // 0x580F40
    virtual ~GameSettings();                                 // 0x5759F0
    virtual void Callback(MemoryUnitManager::eOperation operation);  // 0x575A10

    SaveGameData* get_temp_buffer();               // 0x575B70
    unsigned int get_temp_buffer_size() const;     // 0x575B80
    void delete_save();                            // 0x575C30
    void format();                                 // 0x575C60
    bool debug_unformat();                         // 0x575C70
    bool debug_format();                           // 0x575C80
    bool debug_delete_all();                       // 0x575C90
    void write_crc(SaveGameData* sd);              // 0x575CA0
    void start_new_game(bool from_fe);             // 0x575CB0
    int get_total_game_size();                     // 0x575CD0
    bool enough_space();                           // 0x575D00
    int get_blocks_needed();                       // 0x575D70
    int get_blocks_total();                        // 0x575DD0
    void get_insufficient_space_error(char* str,
                                      bool from_fe);  // 0x575E10
    bool is_memory_unit_damaged();                 // 0x575E80
    bool does_file_exist();                        // 0x575EB0
    bool is_valid(int slot_id, SaveGameData* gd);  // 0x575F30
    bool is_corrupt(SaveGameData* gd);             // 0x575F50
    void set_cur_name(const char* name);           // 0x576000
    void collect_GameSettings(SaveGameData* sd);   // 0x576030
    void dashboard_reboot_to_free_blocks();        // 0x581010
    MemoryUnitManager::eStatus save();             // 0x587380
    MemoryUnitManager::eStatus del(int slot_num);  // 0x587520
    MemoryUnitManager::eStatus load(int slot_num); // 0x587670
    MemoryUnitManager::eStatus load_all(
        SaveGameData** const saves);               // 0x5877B0

private:
    void init_save(SaveGameData* sv);              // 0x575B90
};
static_assert(sizeof(GameSettings) == 0x2A0,
              "GameSettings size mismatch");
static_assert(offsetof(GameSettings, container) == 0x04,
              "GameSettings::container offset mismatch");
static_assert(offsetof(GameSettings, m_temp_buffer) == 0x28C,
              "GameSettings::m_temp_buffer offset mismatch");
static_assert(offsetof(GameSettings, m_cur_name) == 0x292,
              "GameSettings::m_cur_name offset mismatch");

GameSettings* GameSettings::sInst = nullptr;

// ea: 0x004DD930
GameSettings* GameSettings::CreateInst()
{
    if (sInst != nullptr)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\game_data.h";
        AeAssert::gCurrentLine = 71;
        AeAssert::gCurrentExpr = "sInst==0";
        if (!AeAssert::IsIgnored()
            && AeAssert::Assert("singleton already created!"))
            __debugbreak();
    }
    void* memory = mem_heap_malloc_ctx(
        0x2A0u, 4, "shell", "c:\\cod\\code\\game\\game_data.h", 71);
    if (memory != nullptr)
        sInst = new (memory) GameSettings();
    else
        sInst = nullptr;
    return sInst;
}
static_assert(offsetof(GameSettings, m_continued_without_saving) == 0x29E,
              "GameSettings::m_continued_without_saving offset mismatch");

// ea: 0x00580F40
GameSettings::GameSettings()
    : container(defaultFileName)
{
    m_mc_has_save = false;
    m_damaged_save = false;
    MemoryUnitManager::Initialize(0);
    Broc::string sku(defaultFileName);
    const char* v2 = sku.mBlock != nullptr
                         ? (const char*)&sku.mBlock[1]
                         : defaultFileName;
    MemoryUnitManager::SetTitlePrefix(v2);
    MemoryUnitManager::RegisterObserver(this);
    unsigned int GameSaveSize = MemoryUnitManager::GetGameSaveSize(0x1BF4u);
    m_temp_buffer = (SaveGameData*)mem_heap_malloc(32, GameSaveSize);
    strncpy(m_cur_name, "Alex", 0xCu);
    m_cur_name[11] = 0;
    m_continued_without_saving = false;
}

// ea: 0x005759F0
GameSettings::~GameSettings()
{
    mem_heap_free(m_temp_buffer);
}

// ea: 0x00575A10
void GameSettings::Callback(MemoryUnitManager::eOperation operation)
{
    g_enableControllerTest = true;
    MemoryUnitManager::eStatus LastError = MemoryUnitManager::GetLastError();
    if (LastError == MemoryUnitManager::eSuccess
        || (LastError == MemoryUnitManager::eFileDoesNotExist
            && operation == MemoryUnitManager::eSave))
    {
        switch (operation)
        {
        case MemoryUnitManager::eLoad:
            m_mc_has_save = true;
            g_femanager.mProfileManager->mOperationState =
                ProfileManager::kOperationSuccess;
            break;
        case MemoryUnitManager::eSave:
            m_mc_has_save = true;
            g_femanager.mProfileManager->mOperationState =
                ProfileManager::kOperationSuccess;
            break;
        case MemoryUnitManager::eDelete:
        case MemoryUnitManager::eFormat:
            g_femanager.mProfileManager->mOperationState =
                ProfileManager::kOperationSuccess;
            break;
        default:
            AeAssert::gCurrentAuthor = AeAssert::COD3;
            AeAssert::gCurrentFile = "c:\\cod\\code\\game\\game_data.cpp";
            AeAssert::gCurrentLine = 254;
            AeAssert::gCurrentExpr = "0";
            if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
                __debugbreak();
            break;
        }
    }
    else if (operation == MemoryUnitManager::eLoad)
    {
        m_mc_has_save = false;
        m_damaged_save = false;
        if (LastError == MemoryUnitManager::eCRCFailure
            || LastError == MemoryUnitManager::eFileDoesNotExist)
            m_damaged_save = true;
        g_femanager.mProfileManager->mOperationState = ProfileManager::kOperationFailure;
    }
    else if (operation == MemoryUnitManager::eSave
             || operation == MemoryUnitManager::eFormat
             || operation == MemoryUnitManager::eDelete)
    {
        g_femanager.mProfileManager->mOperationState = ProfileManager::kOperationFailure;
    }
    else
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\game_data.cpp";
        AeAssert::gCurrentLine = 287;
        AeAssert::gCurrentExpr = nullptr;
        if (!AeAssert::IsIgnored()
            && AeAssert::Warning("something bad happened"))
            __debugbreak();
    }
}

// ea: 0x00575B70
SaveGameData* GameSettings::get_temp_buffer()
{
    return m_temp_buffer;
}

// ea: 0x00575B80
unsigned int GameSettings::get_temp_buffer_size() const
{
    return MemoryUnitManager::GetGameSaveSize(0x1BF4u);
}

// ea: 0x00575B90
void GameSettings::init_save(SaveGameData* sv)
{
    g_enableControllerTest = false;
    if (sv->mStubData.mSaveGameSlot > 6u)
    {
        AeAssert::gCurrentAuthor = AeAssert::COD3;
        AeAssert::gCurrentFile = "c:\\cod\\code\\game\\game_data.cpp";
        AeAssert::gCurrentLine = 353;
        AeAssert::gCurrentExpr =
            "sv->mStubData.mSaveGameSlot >= 0 && "
            "sv->mStubData.mSaveGameSlot <= 6";
        if (!AeAssert::IsIgnored() && AeAssert::Assert("old cod assert"))
            __debugbreak();
    }
    const char* v2 = gCE ? "8-01-06CE" : "8-01-06";
    strncpy(sv->m_version_number, v2, 0x19u);
    strncpy(sv->m_title_prefix, MemoryUnitManager::GetTitlePrefix(), 0xCu);
}

// ea: 0x00575C30
void GameSettings::delete_save()
{
    if (MemoryUnitManager::DeleteGame("Profiles")
        != MemoryUnitManager::eSuccess)
        g_femanager.mProfileManager->mOperationState = ProfileManager::kOperationFailure;
    m_damaged_save = false;
}

// ea: 0x00575C60
void GameSettings::format()
{
}

// ea: 0x00575C70
bool GameSettings::debug_unformat()
{
    return false;
}

// ea: 0x00575C80
bool GameSettings::debug_format()
{
    return false;
}

// ea: 0x00575C90
bool GameSettings::debug_delete_all()
{
    return false;
}

// ea: 0x00575CA0
void GameSettings::write_crc(SaveGameData* sd)
{
    (void)sd;
}

// ea: 0x00575CB0
void GameSettings::start_new_game(bool from_fe)
{
    if (!from_fe)
        m_continued_without_saving = false;
}

// ea: 0x00575CD0
int GameSettings::get_total_game_size()
{
    unsigned int v1 = 6 * MemoryUnitManager::GetGameSaveSize(0x1BF4u);
    unsigned int v2 = v1 + 2 * MemoryUnitManager::GetClusterSize();
    unsigned int ClusterSize = MemoryUnitManager::GetClusterSize();
    return (int)(ClusterSize + v2 + 4 * ClusterSize);
}

// ea: 0x00575D00
bool GameSettings::enough_space()
{
    MemoryUnitManager::MemoryUnitInfo mui;
    MemoryUnitManager::eStatus MemoryUnitInfo =
        MemoryUnitManager::GetMemoryUnitInfo(&mui);
    unsigned int v1 = 6 * MemoryUnitManager::GetGameSaveSize(0x1BF4u);
    unsigned int v2 = v1 + 2 * MemoryUnitManager::GetClusterSize();
    unsigned int ClusterSize = MemoryUnitManager::GetClusterSize();
    int v5 = (int)MemoryUnitManager::BytesToBlocks(
        ClusterSize + v2 + 4 * ClusterSize);
    unsigned int v6 = (unsigned int)MemoryUnitManager::BytesToBlocks(
        mui.bytesFree);
    if (MemoryUnitInfo != MemoryUnitManager::eSuccess)
    {
        if (MemoryUnitInfo == MemoryUnitManager::eNoMedium)
            return true;
    }
    else if (v5 <= v6)
    {
        return true;
    }
    return false;
}

// ea: 0x00575D70
int GameSettings::get_blocks_needed()
{
    MemoryUnitManager::MemoryUnitInfo mui;
    unsigned int v1 = 6 * MemoryUnitManager::GetGameSaveSize(0x1BF4u);
    unsigned int v2 = v1 + 2 * MemoryUnitManager::GetClusterSize();
    unsigned int ClusterSize = MemoryUnitManager::GetClusterSize();
    unsigned int v4 = MemoryUnitManager::BytesToBlocks(
        ClusterSize + v2 + 4 * ClusterSize);
    MemoryUnitManager::GetMemoryUnitInfo(&mui);
    unsigned int result = v4 - mui.blocksFree;
    if ((v4 - mui.blocksFree) <= 0)
        return 1;
    return (int)result;
}

// ea: 0x00575DD0
int GameSettings::get_blocks_total()
{
    unsigned int v1 = 6 * MemoryUnitManager::GetGameSaveSize(0x1BF4u);
    unsigned int v2 = v1 + 2 * MemoryUnitManager::GetClusterSize();
    unsigned int ClusterSize = MemoryUnitManager::GetClusterSize();
    return (int)MemoryUnitManager::BytesToBlocks(
        ClusterSize + v2 + 4 * ClusterSize);
}

// ea: 0x00575E10
void GameSettings::get_insufficient_space_error(char* str, bool from_fe)
{
    (void)from_fe;
    (void)MemoryUnitManager::GetGameSaveSize(0x1BF4u);
    (void)MemoryUnitManager::GetClusterSize();
    (void)MemoryUnitManager::GetClusterSize();
    const char* STBString = STBManager::sInst->GetSTBString(
        "MEM_ERROR_NOT_ENOUGH_BLOCKS1");
    const char* v6 = STBManager::sInst->GetSTBString(
        "MEM_ERROR_NOT_ENOUGH_BLOCKS2");
    int blocks_needed = get_blocks_needed();
    sprintf(str, "%s %d %s", STBString, blocks_needed, v6);
}

// ea: 0x00575E80
bool GameSettings::is_memory_unit_damaged()
{
    MemoryUnitManager::MemoryUnitInfo mui;
    return m_damaged_save
           || MemoryUnitManager::GetMemoryUnitInfo(&mui)
                  == MemoryUnitManager::eDamagedMedium;
}

// ea: 0x00575EB0
bool GameSettings::does_file_exist()
{
    m_mc_has_save = false;
    MemoryUnitManager::SavedGame files[100];
    int num = MemoryUnitManager::GetSavedGames(files);
    if (num < 0)
        return false;
    if (num <= 0)
        return false;
    int v3 = 0;
    for (MemoryUnitManager::SavedGame* i = files;
         strcmp("Profiles", i->name) != 0; ++i)
    {
        if (++v3 >= num)
            return false;
    }
    m_mc_has_save = true;
    return true;
}

// ea: 0x00575F30
bool GameSettings::is_valid(int slot_id, SaveGameData* gd)
{
    return slot_id == gd->mStubData.mSaveGameSlot;
}

// ea: 0x00575F50
bool GameSettings::is_corrupt(SaveGameData* gd)
{
    bool m_damaged_save = this->m_damaged_save;
    const char* v3 = gCE ? "8-01-06CE" : "8-01-06";
    if (strncmp(gd->m_version_number, v3, 0x19u) == 0
        && strcmp(gd->m_title_prefix,
                  MemoryUnitManager::GetTitlePrefix()) == 0)
    {
        if (m_damaged_save)
            this->m_damaged_save = true;
        return m_damaged_save;
    }
    this->m_damaged_save = true;
    return true;
}

// ea: 0x00576000
void GameSettings::set_cur_name(const char* name)
{
    strncpy(m_cur_name, name, 0xCu);
    m_cur_name[11] = 0;
}

// ea: 0x00576030
void GameSettings::collect_GameSettings(SaveGameData* sd)
{
    int port = LocalClient::ClientToPort(currCl);
    SaveGameData* src = &gSaveGameData[port];
    src->mStubData.mSaved = true;
    int v4 = src->mStubData.mHour + src->mStubData.mDay;
    int v5 = src->mStubData.mMin + v4;
    int v6 = (src->mStubData.mSec + v5) % 50;
    src->mStubData.mSaveId = v6;
    sd->mStubData = src->mStubData;
}

// ea: 0x00581010 (Xbox-specific dashboard reboot; structural stub)
void GameSettings::dashboard_reboot_to_free_blocks()
{
}

// ea: 0x00587380
MemoryUnitManager::eStatus GameSettings::save()
{
    unsigned int GameSaveSize = MemoryUnitManager::GetGameSaveSize(0x1BF4u);
    unsigned char* buf = (unsigned char*)m_temp_buffer;
    int temp_buffer_size = (int)GameSaveSize;
    memset(buf, 0, GameSaveSize);
    memcpy(buf, &gSaveGameData[LocalClient::ClientToPort(currCl)],
           0x1BF4u);
    SaveGameData* sv = m_temp_buffer;
    collect_GameSettings(sv);
    init_save(sv);
    container.Reset("Profiles");
    Broc::string v15(MemoryUnitManager::GetTitlePrefix());
    Broc::string filename = v15 + "Profiles";
    for (int v6 = 0; v6 < 6; ++v6)
    {
        if (!m_mc_has_save || m_damaged_save
            || sv->mStubData.mSaveGameSlot == v6)
        {
            if (v6 <= 0)
            {
                const char* v8 = filename.mBlock != nullptr
                                     ? (const char*)&filename.mBlock[1]
                                     : defaultFileName;
                ae_formatted_string<128, unsigned char> v12("%s", v8);
                container.AddFile((const char*)v12.mBuff, buf,
                                  temp_buffer_size);
            }
            else
            {
                const char* v11 = filename.mBlock != nullptr
                                      ? (const char*)&filename.mBlock[1]
                                      : defaultFileName;
                ae_formatted_string<128, unsigned char> v13("%s-%d", v11,
                                                            v6);
                container.AddFile((const char*)v13.mBuff, buf,
                                  temp_buffer_size);
            }
        }
    }
    MemoryUnitManager::eStatus v9 = MemoryUnitManager::SaveGame(container);
    m_damaged_save = false;
    return v9;
}

// ea: 0x00587520
MemoryUnitManager::eStatus GameSettings::del(int slot_num)
{
    int temp_buffer_size =
        (int)MemoryUnitManager::GetGameSaveSize(0x1BF4u);
    unsigned int GameSaveSize = MemoryUnitManager::GetGameSaveSize(0x1BF4u);
    memset(m_temp_buffer, 0, GameSaveSize);
    m_temp_buffer->mStubData.mSaveGameSlot = slot_num;
    init_save(m_temp_buffer);
    container.Reset("Profiles");
    Broc::string v6(MemoryUnitManager::GetTitlePrefix());
    Broc::string filename = v6 + "Profiles";
    if (slot_num <= 0)
    {
        const char* v9 = filename.mBlock != nullptr
                             ? (const char*)&filename.mBlock[1]
                             : defaultFileName;
        ae_formatted_string<128, unsigned char> v14("%s", v9);
        container.AddFile((const char*)v14.mBuff,
                          (unsigned char*)m_temp_buffer, temp_buffer_size);
    }
    else
    {
        const char* v7 = filename.mBlock != nullptr
                             ? (const char*)&filename.mBlock[1]
                             : defaultFileName;
        ae_formatted_string<128, unsigned char> v8("%s-%d", v7, slot_num);
        container.AddFile((const char*)v8.mBuff,
                          (unsigned char*)m_temp_buffer, temp_buffer_size);
    }
    return MemoryUnitManager::SaveGame(container);
}

// ea: 0x00587670
MemoryUnitManager::eStatus GameSettings::load(int slot_num)
{
    unsigned int GameSaveSize = MemoryUnitManager::GetGameSaveSize(0x1BF4u);
    memset(m_temp_buffer, 0, MemoryUnitManager::GetGameSaveSize(0x1BF4u));
    g_enableControllerTest = false;
    container.Reset("Profiles");
    Broc::string v13(MemoryUnitManager::GetTitlePrefix());
    Broc::string filename = v13 + "Profiles";
    if (slot_num <= 0)
    {
        const char* v8 = filename.mBlock != nullptr
                             ? (const char*)&filename.mBlock[1]
                             : defaultFileName;
        ae_formatted_string<128, unsigned char> v12("%s", v8);
        container.AddFile((const char*)v12.mBuff,
                          (unsigned char*)m_temp_buffer, GameSaveSize);
    }
    else
    {
        const char* v6 = filename.mBlock != nullptr
                             ? (const char*)&filename.mBlock[1]
                             : defaultFileName;
        ae_formatted_string<128, unsigned char> v7("%s-%d", v6, slot_num);
        container.AddFile((const char*)v7.mBuff,
                          (unsigned char*)m_temp_buffer, GameSaveSize);
    }
    return MemoryUnitManager::LoadGame(container);
}

// ea: 0x005877B0
MemoryUnitManager::eStatus GameSettings::load_all(
    SaveGameData** const saves)
{
    unsigned int GameSaveSize = MemoryUnitManager::GetGameSaveSize(0x1BF4u);
    container.Reset("Profiles");
    Broc::string v15(MemoryUnitManager::GetTitlePrefix());
    Broc::string filename = v15 + "Profiles";
    for (int v6 = 0; v6 < 6; ++v6)
    {
        if (v6 <= 0)
        {
            const char* v9 = filename.mBlock != nullptr
                                 ? (const char*)&filename.mBlock[1]
                                 : defaultFileName;
            ae_formatted_string<128, unsigned char> v13("%s", v9);
            container.AddFile((const char*)v13.mBuff,
                              (unsigned char*)saves[v6], GameSaveSize);
        }
        else
        {
            const char* v7 = filename.mBlock != nullptr
                                 ? (const char*)&filename.mBlock[1]
                                 : defaultFileName;
            ae_formatted_string<128, unsigned char> v8("%s-%d", v7, v6);
            container.AddFile((const char*)v8.mBuff,
                              (unsigned char*)saves[v6], GameSaveSize);
        }
    }
    return MemoryUnitManager::LoadGame(container);
}
