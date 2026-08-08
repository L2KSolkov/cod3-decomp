// ============================================================================
// bdSequenceNumber.cpp - RFC1982 serial-number arithmetic (14 funcs).
// Source: bdCore:bdSequenceNumber.obj
// Reconstructed from Demonware 2.0 source + COD3 release decompilation.
// ============================================================================

#include "bd/bd_types.h"

// ============================================================================
// bdSequenceNumber::bdSequenceNumber (last, seqNumber, bits) - ea: 0x9EC360
// ============================================================================
bdSequenceNumber::bdSequenceNumber(const bdSequenceNumber& last,
                                   unsigned int seqNumber, unsigned int bits)
    : m_seqNum(-1) {
    set(last, seqNumber, bits);
}

// ============================================================================
// bdSequenceNumber::set - ea: 0x9EC070
// ============================================================================
void bdSequenceNumber::set(const bdSequenceNumber& last,
                           unsigned int seqNumber, unsigned int bits) {
    const int range = 2 << (bits - 1);
    const int lastSeq = last.m_seqNum % range;
    const int lastRangeBase = last.m_seqNum - (last.m_seqNum % range);
    const int curSeq = (int)seqNumber % range;

    if ((int)seqNumber >= range) {
        bdMessageProxy proxy(".\\bdContainers\\bdSequenceNumber.cpp",
                             "void __thiscall bdSequenceNumber::set(const class bdSequenceNumber &,const unsigned int,const unsigned int)",
                             0x28u, "dw/err");
        proxy.log("defaultFileName", "Sequence number given outside the range.");
    }

    if (lastSeq < 0) {
        m_seqNum = seqNumber;
    } else if (lastSeq == curSeq) {
        m_seqNum = last.m_seqNum;
    } else {
        const bool lastSmaller =
            ((lastSeq < curSeq) && (curSeq - lastSeq < range / 2)) ||
            ((lastSeq > curSeq) && (lastSeq - curSeq > range / 2));
        const bool lastLarger =
            ((lastSeq < curSeq) && (curSeq - lastSeq > range / 2)) ||
            ((lastSeq > curSeq) && (lastSeq - curSeq < range / 2));

        int rangeChange = 0;
        if (!(lastLarger || lastSmaller)) {
            bdMessageProxy proxy(".\\bdContainers\\bdSequenceNumber.cpp",
                                 "void __thiscall bdSequenceNumber::set(const class bdSequenceNumber &,const unsigned int,const unsigned int)",
                                 0x42u, "dw/warn/");
            proxy.log("bdCore/bdContainers/sequenceNumber",
                      "Sequence numbers are too far away and cannot be compared.");
        }

        if (lastLarger && lastSeq > curSeq) {
            // do nothing
        } else if (lastLarger && lastSeq < curSeq) {
            rangeChange = -1;
        } else if (lastSmaller && lastSeq > curSeq) {
            rangeChange = 1;
        } else if (lastSmaller && lastSeq < curSeq) {
            // do nothing
        }

        m_seqNum = lastRangeBase + range * rangeChange + curSeq;
    }
}

// ============================================================================
// bdSequenceNumber::operator+ - ea: 0x9EC240
// ============================================================================
bdSequenceNumber bdSequenceNumber::operator+(const bdSequenceNumber& other) const {
    bdSequenceNumber seqNum(m_seqNum);
    seqNum.m_seqNum += other.m_seqNum;
    return seqNum;
}

// ============================================================================
// bdSequenceNumber::operator+= - ea: 0x9EC260
// ============================================================================
bdSequenceNumber& bdSequenceNumber::operator+=(const bdSequenceNumber& other) {
    m_seqNum += other.m_seqNum;
    return *this;
}

// ============================================================================
// bdSequenceNumber::operator++ (prefix) - ea: 0x9EC270
// ============================================================================
bdSequenceNumber& bdSequenceNumber::operator++() {
    ++m_seqNum;
    return *this;
}

// ============================================================================
// bdSequenceNumber::operator++ (postfix) - ea: 0x9EC280
// ============================================================================
bdSequenceNumber bdSequenceNumber::operator++(int) {
    bdSequenceNumber other = *this;
    m_seqNum += 1;
    return other;
}

// ============================================================================
// bdSequenceNumber::operator- - ea: 0x9EC2A0
// ============================================================================
bdSequenceNumber bdSequenceNumber::operator-(const bdSequenceNumber& other) const {
    bdSequenceNumber seqNum(m_seqNum);
    seqNum.m_seqNum -= other.m_seqNum;
    return seqNum;
}

// ============================================================================
// Comparison operators - ea: 0x9EC2C0 / 0x9EC2E0 / 0x9EC300 / 0x9EC320 /
// 0x9EC340 (==) / inline (!=)
// ============================================================================
bool bdSequenceNumber::operator>(const bdSequenceNumber& other) const {
    return m_seqNum > other.m_seqNum;
}

bool bdSequenceNumber::operator<(const bdSequenceNumber& other) const {
    return m_seqNum < other.m_seqNum;
}

bool bdSequenceNumber::operator<=(const bdSequenceNumber& other) const {
    return m_seqNum <= other.m_seqNum;
}

bool bdSequenceNumber::operator>=(const bdSequenceNumber& other) const {
    return m_seqNum >= other.m_seqNum;
}

bool bdSequenceNumber::operator==(const bdSequenceNumber& other) const {
    return m_seqNum == other.m_seqNum;
}

bool bdSequenceNumber::operator!=(const bdSequenceNumber& other) const {
    return m_seqNum != other.m_seqNum;
}
