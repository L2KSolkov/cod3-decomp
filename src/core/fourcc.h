// FourCC value wrapper used by the in-place asset/task APIs.
// Layout and member names are from the IDA local type dump.
#pragma once

struct FourCC {
    unsigned int mVal;  // +0x00

    FourCC() = default;
    explicit FourCC(int v);
    int GetVal() const;
};

static_assert(sizeof(FourCC) == 0x4, "FourCC size mismatch");

bool operator==(FourCC lhs, FourCC rhs);
