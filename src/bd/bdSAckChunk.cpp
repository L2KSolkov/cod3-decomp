// ============================================================================
// bdSAckChunk.cpp â€” selective-ack chunk (13 funcs).
// Source: .\bdPacket\bdSAckChunk.cpp (bdConnection)
// Verified against IDA (bdConnection:bdSAckChunk.obj).
// ============================================================================

#include "bd/bd_types.h"

// ============================================================================
// getCumulativeAck â€” ea: 0x8AD670
// ============================================================================
unsigned short bdSAckChunk::getCumulativeAck() const {
    return this->m_cumulativeAck;
}

// ============================================================================
// setCumulativeAck â€” ea: 0x8AD680
// ============================================================================
void bdSAckChunk::setCumulativeAck(unsigned short ack) {
    this->m_cumulativeAck = ack;
}

// ============================================================================
// getGapList â€” ea: 0x8AD690
// ============================================================================
bdLinkedList<bdGapAckBlock>& bdSAckChunk::getGapList() {
    return this->m_gapList;
}

// ============================================================================
// setWindowCredit â€” ea: 0x8AD6A0
// ============================================================================
void bdSAckChunk::setWindowCredit(int credit) {
    this->m_windowCredit = credit;
}

// ============================================================================
// getWindowCredit â€” ea: 0x8AD6B0
// ============================================================================
int bdSAckChunk::getWindowCredit() const {
    return this->m_windowCredit;
}

// ============================================================================
// getFlags â€” ea: 0x8AD6C0
// ============================================================================
bdSAckChunk::bdSAckFlags bdSAckChunk::getFlags() const {
    return this->m_flags;
}

// ============================================================================
// bdSAckChunk::serialize â€” ea: 0x8AD860
// ============================================================================
unsigned int bdSAckChunk::serialize(unsigned char* data, unsigned int size) {
    unsigned int v3 = size;
    unsigned char* v4 = data;
    unsigned int v6 = bdChunk::serialize(data, size);
    unsigned int offset = v6;
    bool ok = false;
    unsigned char flags = (unsigned char)this->m_flags;
    if (bdBytePacker::appendBasicType(v4, v3, v6, &offset, &flags, 1u)) {
        unsigned short zero = 0;
        if (bdBytePacker::appendBasicType(v4, v3, offset, &offset, &zero, 2u))
            ok = true;
    }
    ++offset;
    bool ok2 = false;
    if (ok) {
        unsigned short ack = this->m_cumulativeAck;
        if (bdBytePacker::appendBasicType(v4, v3, offset, &offset, &ack, 2u)) {
            int credit = this->m_windowCredit;
            if (bdBytePacker::appendBasicType(v4, v3, offset, &offset, &credit, 4u)) {
                unsigned short gapCount = (unsigned short)this->m_gapList.m_size;
                if (bdBytePacker::appendBasicType(v4, v3, offset, &offset, &gapCount, 2u)) {
                    unsigned short duplicateTsnCount = 0;
                    if (bdBytePacker::appendBasicType(v4, v3, offset, &offset, &duplicateTsnCount, 2u))
                        ok2 = true;
                }
            }
        }
    }
    for (bdLinkedList<bdGapAckBlock>::Node* node = this->m_gapList.m_head;
         node != NULL; node = node->m_next) {
        unsigned short start = (unsigned short)node->m_value.m_start;
        ok2 = ok2
              && bdBytePacker::appendBasicType(v4, v3, offset, &offset, &start, 2u)
              && bdBytePacker::appendBasicType(v4, v3, offset, &offset,
                                               &node->m_value.m_end, 2u);
    }
    return ok2 ? offset : 0;
}

// ============================================================================
// bdSAckChunk::getSerializedSize â€” ea: 0x8ADA00
// ============================================================================
unsigned int bdSAckChunk::getSerializedSize() {
    return this->serialize(NULL, 0xFFFFFFFFu);
}

// ============================================================================
// bdSAckChunk::bdSAckChunk (default) â€” ea: 0x8ADA40
// ============================================================================
bdSAckChunk::bdSAckChunk()
    : bdChunk((bdChunkTypes)5),
      m_flags(BD_SACK_ACK),
      m_cumulativeAck(0),
      m_windowCredit(0),
      m_gapList() {
}

// ============================================================================
// bdSAckChunk::bdSAckChunk (window, flags) â€” ea: 0x8ADA70
// ============================================================================
bdSAckChunk::bdSAckChunk(int windowCredit, bdSAckFlags flags)
    : bdChunk((bdChunkTypes)5),
      m_flags(flags),
      m_cumulativeAck(0),
      m_windowCredit(windowCredit),
      m_gapList() {
}

// ============================================================================
// bdSAckChunk::~bdSAckChunk â€” ea: 0x8ADAB0
// ============================================================================
bdSAckChunk::~bdSAckChunk() {
    this->m_gapList.clear();
}

// ============================================================================
// bdSAckChunk::addGap â€” ea: 0x8ADB10
// ============================================================================
void bdSAckChunk::addGap(const bdGapAckBlock& block) {
    this->m_gapList.insertAfter(this->m_gapList.m_tail, block);
}

// ============================================================================
// bdSAckChunk::deserialize â€” ea: 0x8ADB30
// ============================================================================
bool bdSAckChunk::deserialize(const unsigned char* data, unsigned int size,
                              unsigned int* offset) {
    unsigned int v4 = size;
    unsigned int v23 = *offset;
    bdSAckChunk* v24 = this;
    unsigned char v6 = 0;
    unsigned int v22;
    bool ok = false;
    if (bdChunk::deserialize(data, size, &v23)
        && bdBytePacker::removeBasicType(data, size, v23, &v23, (unsigned char*)&v22, 1u)) {
        v6 = (unsigned char)v22;
        ok = true;
    }
    this->m_flags = (bdSAckFlags)v6;
    bool ok2 = ok && bdBytePacker::removeBasicType(data, size, v23, &v23,
                                                   (unsigned char*)&v22, 2u);
    unsigned short v9 = 0;
    ++v23;
    if (ok2 && bdBytePacker::removeBasicType(data, size, v23, &v23,
                                             (unsigned char*)&v22, 2u)) {
        v9 = (unsigned short)v22;
        ok2 = true;
    } else {
        ok2 = false;
    }
    this->m_cumulativeAck = v9;
    int v12 = 0;
    if (ok2 && bdBytePacker::removeBasicType(data, size, v23, &v23,
                                             (unsigned char*)&v22, 4u)) {
        v12 = v22;
        ok2 = true;
    } else {
        ok2 = false;
    }
    this->m_windowCredit = v12;
    unsigned int gapCount = 0;
    unsigned int v25 = 0;
    if (ok2 && bdBytePacker::removeBasicType(data, size, v23, &v23,
                                             (unsigned char*)&v25, 2u)) {
        gapCount = v25;
        ok2 = true;
    } else {
        ok2 = false;
    }
    unsigned short v15 = 0;
    v23 += 2;
    if (ok2) {
        while (v15 < gapCount) {
            unsigned short v16 = 0;
            unsigned int tmp;
            if (!bdBytePacker::removeBasicType(data, size, v23, &v23,
                                               (unsigned char*)&tmp, 2u))
                return false;
            v16 = (unsigned short)tmp;
            unsigned short v18 = 0;
            if (!bdBytePacker::removeBasicType(data, size, v23, &v23,
                                               (unsigned char*)&tmp, 2u))
                return false;
            v18 = (unsigned short)tmp;
            bdGapAckBlock block;
            block.m_start = v16;
            block.m_end = v18;
            this->m_gapList.insertAfter(this->m_gapList.m_tail, block);
            bdMessageProxy proxy(".\\bdPacket\\bdSAckChunk.cpp",
                                 "bool __thiscall bdSAckChunk::deserialize(const unsigned char *const ,const unsigned int,unsigned int &)",
                                 0xA4u, "dw/info/");
            proxy.log("bdConnection/chunks", "gap ack: %hu-%hu", v16, v18);
            ++v15;
            v4 = size;
        }
        *offset = v23;
    }
    return ok2;
}
