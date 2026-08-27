// Global tag-compatible views of the release Broc API symbols.
//
// IDA names the runtime types as global `BrocAPI`/`BrocExports`, while the
// source-facing definitions live in namespace Broc.  Keep one definition of
// those linker-visible tags so every translation unit uses the same release
// layout instead of inventing offset-only local structs.
#pragma once

#include "engine/broc_types.h"

struct BrocExports : Broc::BrocExports {};
struct BrocAPI : Broc::BrocAPI {};

static_assert(sizeof(BrocExports) == sizeof(Broc::BrocExports),
              "global BrocExports layout mismatch");
static_assert(sizeof(BrocAPI) == sizeof(Broc::BrocAPI),
              "global BrocAPI layout mismatch");

extern BrocAPI* gpBrocAPI;
