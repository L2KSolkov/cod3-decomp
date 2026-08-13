// ============================================================================
// COD3 Core Types - filesystem/searchpath/math/time structs (core.o)
// Reconstructed from IDA local types (PDB symbol data) via ida-pro-mcp.
// All sizes and offsets verified against IDA.
// ============================================================================

#pragma once

#include <stddef.h>
#include <stdint.h>

// ============================================================================
// searchpath_s - filesystem search path entry (20 bytes)
// Size: 0x14 (20 bytes) - verified against IDA
// ============================================================================
struct pack_t;
struct directory_t;
struct searchpath_s {
    searchpath_s* next;       // +0x00
    pack_t*       pack;       // +0x04
    directory_t*  dir;        // +0x08
    int           bLocalized; // +0x0C
    int           language;   // +0x10
};
static_assert(sizeof(searchpath_s) == 0x14, "searchpath_s size mismatch");
static_assert(offsetof(searchpath_s, pack) == 0x04,
              "searchpath_s::pack offset mismatch");
static_assert(offsetof(searchpath_s, language) == 0x10,
              "searchpath_s::language offset mismatch");

// ============================================================================
// fileData_s - file data entry (12 bytes)
// Size: 0x0C (12 bytes) - verified against IDA
// ============================================================================
struct fileData_s {
    char*  name;  // +0x00
    void*  data;  // +0x04
    void (*mem_heap_free)(fileData_s*);  // +0x08
};
static_assert(sizeof(fileData_s) == 0x0C, "fileData_s size mismatch");
static_assert(offsetof(fileData_s, data) == 0x04,
              "fileData_s::data offset mismatch");

// ============================================================================
// fileInPack_s - pak file entry (24 bytes)
// Size: 0x18 (24 bytes) - verified against IDA
// ============================================================================
struct fileInPack_s {
    unsigned int pos;         // +0x00
    fileData_s   data;        // +0x04
    char*        name;        // +0x10
    fileInPack_s* next;       // +0x14
};
static_assert(sizeof(fileInPack_s) == 0x18, "fileInPack_s size mismatch");
static_assert(offsetof(fileInPack_s, data) == 0x04,
              "fileInPack_s::data offset mismatch");
static_assert(offsetof(fileInPack_s, name) == 0x10,
              "fileInPack_s::name offset mismatch");

// ============================================================================
// pack_t - loaded pak archive (416 bytes)
// Size: 0x1A0 (416 bytes) - verified against IDA
// ============================================================================
struct pack_t {
    char          pakFilename[128];   // +0x000
    char          pakBasename[128];   // +0x080
    char          pakGamename[128];   // +0x100
    void*         handle;             // +0x180
    int           checksum;           // +0x184
    int           pure_checksum;      // +0x188
    int           numfiles;           // +0x18C
    unsigned char referenced[4];      // +0x190
    int           hashSize;           // +0x194
    fileInPack_s** hashTable;         // +0x198
    fileInPack_s*  buildBuffer;       // +0x19C
};
static_assert(sizeof(pack_t) == 0x1A0, "pack_t size mismatch");
static_assert(offsetof(pack_t, handle) == 0x180, "pack_t::handle offset mismatch");
static_assert(offsetof(pack_t, hashTable) == 0x198,
              "pack_t::hashTable offset mismatch");

// ============================================================================
// directory_t - search directory (256 bytes)
// Size: 0x100 (256 bytes) - verified against IDA
// ============================================================================
struct directory_t {
    char path[128];     // +0x00
    char gamedir[128];  // +0x80
};
static_assert(sizeof(directory_t) == 0x100, "directory_t size mismatch");
static_assert(offsetof(directory_t, gamedir) == 0x80,
              "directory_t::gamedir offset mismatch");

// ============================================================================
// cplane_s - collision plane (20 bytes)
// Size: 0x14 (20 bytes) - verified against IDA
// ============================================================================
struct cplane_s {
    float          normal[3];   // +0x00
    float          dist;        // +0x0C
    unsigned char  type;        // +0x10
    unsigned char  signbits;    // +0x11
    unsigned char  pad[2];      // +0x12
};
static_assert(sizeof(cplane_s) == 0x14, "cplane_s size mismatch");
static_assert(offsetof(cplane_s, dist) == 0x0C, "cplane_s::dist offset mismatch");

// ============================================================================
// qtime_s - broken-out time (36 bytes)
// Size: 0x24 (36 bytes) - verified against IDA
// ============================================================================
struct qtime_s {
    int tm_sec;    // +0x00
    int tm_min;    // +0x04
    int tm_hour;   // +0x08
    int tm_mday;   // +0x0C
    int tm_mon;    // +0x10
    int tm_year;   // +0x14
    int tm_wday;   // +0x18
    int tm_yday;   // +0x1C
    int tm_isdst;  // +0x20
};
static_assert(sizeof(qtime_s) == 0x24, "qtime_s size mismatch");
static_assert(offsetof(qtime_s, tm_year) == 0x14, "qtime_s::tm_year offset mismatch");

// ============================================================================
// sysEventType_t - system event type enum
// ============================================================================
enum sysEventType_t {
    SE_NONE = 0,
    SE_KEY = 1,
    SE_CHAR = 2,
    SE_MOUSE = 3,
    SE_JOYSTICK = 4,
    SE_CONSOLE = 5,
    SE_PACKET = 6,
};

// ============================================================================
// sysEvent_t - queued system event (24 bytes)
// Size: 0x18 (24 bytes) - verified against IDA
// ============================================================================
struct sysEvent_t {
    int            evTime;        // +0x00
    sysEventType_t evType;        // +0x04
    int            evValue;       // +0x08
    int            evValue2;      // +0x0C
    int            evPtrLength;   // +0x10
    void*          evPtr;         // +0x14
};
static_assert(sizeof(sysEvent_t) == 0x18, "sysEvent_t size mismatch");
static_assert(offsetof(sysEvent_t, evType) == 0x04,
              "sysEvent_t::evType offset mismatch");

// ============================================================================
// angles_t - Euler angles (12 bytes)
// Size: 0x0C (12 bytes) - verified against IDA
// ============================================================================
class angles_t {
public:
    float pitch;  // +0x00
    float yaw;    // +0x04
    float roll;   // +0x08
};
static_assert(sizeof(angles_t) == 0x0C, "angles_t size mismatch");

// ============================================================================
// DObjSkelMat - DObj skeleton matrix (64 bytes)
// Size: 0x40 (64 bytes) - verified against IDA
// ============================================================================
struct DObjSkelMat {
    float axis[3][4];    // +0x00
    float origin[4];     // +0x30
};
static_assert(sizeof(DObjSkelMat) == 0x40, "DObjSkelMat size mismatch");
static_assert(offsetof(DObjSkelMat, origin) == 0x30,
              "DObjSkelMat::origin offset mismatch");

// ============================================================================
// idVec3 - id-style 3-vector (12 bytes)
// Size: 0x0C (12 bytes) - verified against IDA
// ============================================================================
class idVec3 {
public:
    float x;  // +0x00
    float y;  // +0x04
    float z;  // +0x08
};
static_assert(sizeof(idVec3) == 0x0C, "idVec3 size mismatch");
static_assert(offsetof(idVec3, z) == 0x08, "idVec3::z offset mismatch");

// ============================================================================
// mat3_t - id-style 3x3 matrix (36 bytes)
// Size: 0x24 (36 bytes) - verified against IDA
// ============================================================================
class mat3_t {
public:
    idVec3 mat[3];  // +0x00
    void Transpose(mat3_t& matrix);
    void Transpose();
    void ProjectVector(const idVec3& src, idVec3& dst) const;
    void UnprojectVector(const idVec3& src, idVec3& dst) const;
    mat3_t Inverse() const;
    void Clear();
};
static_assert(sizeof(mat3_t) == 0x24, "mat3_t size mismatch");
static_assert(offsetof(mat3_t, mat) == 0x00, "mat3_t::mat offset mismatch");

// ============================================================================
// quat_t - quaternion (16 bytes)
// Size: 0x10 (16 bytes) - verified against IDA
// ============================================================================
class quat_t {
public:
    float x;  // +0x00
    float y;  // +0x04
    float z;  // +0x08
    float w;  // +0x0C
};
static_assert(sizeof(quat_t) == 0x10, "quat_t size mismatch");
static_assert(offsetof(quat_t, w) == 0x0C, "quat_t::w offset mismatch");
