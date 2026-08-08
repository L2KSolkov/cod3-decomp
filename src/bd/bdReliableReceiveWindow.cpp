// ============================================================================
// bdReliableReceiveWindow.cpp - incoming reliable-message window (6 funcs).
// Source: bdConnection:bdReliableReceiveWindow.obj
// Reconstructed from Demonware 2.0 source + COD3 release disassembly.
// ============================================================================

#include "bd/bd_types.h"

#include <new>

// ============================================================================
// bdReliableReceiveWindow::bdReliableReceiveWindow - ea: 0x8A8230
// ============================================================================
bdReliableReceiveWindow::bdReliableReceiveWindow()
    : m_newest(-1),
      m_lastCumulative(-1),
      m_lastDispatched(-1),
      m_shouldAck(false),
      m_recvWindowCredit(BD_DEFAULT_RECEIVE_WINDOW_CREDIT),
      m_recvWindowUsedCredit(0),
      m_sack() {
}

// ============================================================================
// bdReliableReceiveWindow::~bdReliableReceiveWindow - ea: 0x8A8290
// ============================================================================
bdReliableReceiveWindow::~bdReliableReceiveWindow() {
    if (m_sack.m_ptr != NULL && m_sack.m_ptr->releaseRef() == 0)
        delete m_sack.m_ptr;
    m_sack.m_ptr = NULL;
    for (int i = 0; i < BD_MAX_WINDOW_SIZE; i++) {
        if (m_frame[i].m_ptr != NULL && m_frame[i].m_ptr->releaseRef() == 0)
            delete m_frame[i].m_ptr;
        m_frame[i].m_ptr = NULL;
    }
}

// ============================================================================
// bdReliableReceiveWindow::add - ea: 0x8A8810
// ============================================================================
bool bdReliableReceiveWindow::add(const bdReference<bdDataChunk>& chunk) {
    bool result = true;

    do {
        if (chunk.m_ptr == NULL) {
            bdMessageProxy proxy(".\\bdWindow\\bdReliableReceiveWindow.cpp",
                                 "bool __thiscall bdReliableReceiveWindow::add(class bdReference<class bdDataChunk>)",
                                 0x1Fu, "dw/err");
            proxy.log("defaultFileName", "bdReliableReceiveWindow::add, null chunk.");
        }
    } while (g_assertFalse);

    bdSequenceNumber seqNumber(m_newest, chunk.m_ptr->getSequenceNumber(), 16);
    unsigned int serializedSize = chunk.m_ptr->getSerializedSize();

    if (seqNumber < m_newest ||
        serializedSize + m_recvWindowUsedCredit <= m_recvWindowCredit) {
        if (seqNumber > m_lastDispatched + BD_MAX_WINDOW_SIZE) {
            bdMessageProxy proxy(".\\bdWindow\\bdReliableReceiveWindow.cpp",
                                 "bool __thiscall bdReliableReceiveWindow::add(class bdReference<class bdDataChunk>)",
                                 0x2Eu, "dw/info/");
            proxy.log("bdConnection/windows", "Window overflow (a) - info only");
            result = false;
        } else if (seqNumber > m_lastDispatched) {
            unsigned int index = (unsigned int)seqNumber.getValue() % BD_MAX_WINDOW_SIZE;
            if (m_frame[index].m_ptr == NULL) {
                m_recvWindowUsedCredit += chunk.m_ptr->getSerializedSize();
                m_frame[index].m_ptr = chunk.m_ptr;
                if (chunk.m_ptr != NULL)
                    chunk.m_ptr->addRef();
                if (seqNumber > m_newest)
                    m_newest = seqNumber;
            } else if (m_frame[index].m_ptr->getSequenceNumber() !=
                       chunk.m_ptr->getSequenceNumber()) {
                bdMessageProxy proxy(".\\bdWindow\\bdReliableReceiveWindow.cpp",
                                     "bool __thiscall bdReliableReceiveWindow::add(class bdReference<class bdDataChunk>)",
                                     0x42u, "dw/err/");
                proxy.log("bdConnection/windows", "Window overflow (b) - error");
                result = false;
            }
        }
    } else {
        bdMessageProxy proxy(".\\bdWindow\\bdReliableReceiveWindow.cpp",
                             "bool __thiscall bdReliableReceiveWindow::add(class bdReference<class bdDataChunk>)",
                             0x49u, "dw/info/");
        proxy.log("bdConnection/windows", "Not enough recv window credit.");
        result = false;
    }

    calculateAck();
    return result;
}

// ============================================================================
// bdReliableReceiveWindow::getDataToSend - ea: 0x8A8320
// ============================================================================
void bdReliableReceiveWindow::getDataToSend(bdPacket& packet) {
    if (m_sack.m_ptr != NULL) {
        if (packet.addChunk(bdReference<bdChunk>(m_sack.m_ptr))) {
            if (m_sack.m_ptr->releaseRef() == 0)
                delete m_sack.m_ptr;
            m_sack.m_ptr = NULL;
        } else {
            bdMessageProxy proxy(".\\bdWindow\\bdReliableReceiveWindow.cpp",
                                 "void __thiscall bdReliableReceiveWindow::getDataToSend(class bdPacket &)",
                                 0x60u, "dw/info/");
            proxy.log("bdConnection/windows", "SACK chunk didn't fit in packet");
        }
    }
}

// ============================================================================
// bdReliableReceiveWindow::getNextToRead - ea: 0x8A83B0
// ============================================================================
bdReference<bdDataChunk> bdReliableReceiveWindow::getNextToRead() {
    bdReference<bdDataChunk> chunk;
    unsigned int index = ((unsigned int)m_lastDispatched.getValue() + 1) % BD_MAX_WINDOW_SIZE;
    bdSequenceNumber toDispatch = m_lastDispatched + 1;

    if (m_frame[index].m_ptr != NULL) {
        bdSequenceNumber frameSeqNumber(m_lastDispatched,
                                        m_frame[index].m_ptr->getSequenceNumber(), 16);
        if (toDispatch == frameSeqNumber) {
            chunk.m_ptr = m_frame[index].m_ptr;
            if (chunk.m_ptr != NULL)
                chunk.m_ptr->addRef();
            if (m_frame[index].m_ptr->releaseRef() == 0)
                delete m_frame[index].m_ptr;
            m_frame[index].m_ptr = NULL;
            m_lastDispatched = frameSeqNumber;

            m_recvWindowUsedCredit -= chunk.m_ptr->getSerializedSize();

            if (m_sack.m_ptr != NULL)
                m_sack.m_ptr->setWindowCredit(m_recvWindowCredit - m_recvWindowUsedCredit);
            else {
                bdMessageProxy proxy(".\\bdWindow\\bdReliableReceiveWindow.cpp",
                                     "class bdReference<class bdDataChunk> __thiscall bdReliableReceiveWindow::getNextToRead(void)",
                                     0x7Fu, "dw/err/");
                proxy.log("bdConnection/windows", "No SACK available");
            }
        } else {
            bdMessageProxy proxy(".\\bdWindow\\bdReliableReceiveWindow.cpp",
                                 "class bdReference<class bdDataChunk> __thiscall bdReliableReceiveWindow::getNextToRead(void)",
                                 0x84u, "dw/info/");
            proxy.log("bdConnection/windows", "unexpected seq number. (%d != %d)",
                      toDispatch.getValue(), frameSeqNumber.getValue());
        }
    }

    return chunk;
}

// ============================================================================
// bdReliableReceiveWindow::calculateAck - ea: 0x8A8580
// ============================================================================
void bdReliableReceiveWindow::calculateAck() {
    bdSAckChunk* sack = (bdSAckChunk*)bdMemory::allocate(sizeof(bdSAckChunk));
    if (sack != NULL)
        sack = new (sack) bdSAckChunk(m_recvWindowCredit - m_recvWindowUsedCredit);

    if (m_sack.m_ptr != NULL && m_sack.m_ptr->releaseRef() == 0)
        delete m_sack.m_ptr;
    m_sack.m_ptr = sack;
    if (sack != NULL)
        sack->addRef();

    bdSequenceNumber i = m_lastCumulative > m_lastDispatched
                             ? m_lastCumulative + 1
                             : m_lastDispatched + 1;
    for (; i <= m_newest; i++) {
        unsigned int index = (unsigned int)i.getValue() % BD_MAX_WINDOW_SIZE;
        if (m_frame[index].m_ptr != NULL)
            m_lastCumulative = i;
        else
            break;
    }

    if (m_lastCumulative.getValue() != -1) {
        bdGapAckBlock block;
        for (i = m_lastCumulative + 1; i <= m_newest; i++) {
            unsigned int index = (unsigned int)i.getValue() % BD_MAX_WINDOW_SIZE;
            if (m_frame[index].m_ptr != NULL) {
                if (!block.m_start)
                    block.m_start = (unsigned int)((i - m_lastCumulative).getValue());
            } else {
                if (block.m_start) {
                    block.m_end = (unsigned int)(((i - 1) - m_lastCumulative).getValue());
                    m_sack.m_ptr->addGap(block);
                    block.m_start = 0;
                    block.m_end = 0;
                }
            }
        }
        if (block.m_start) {
            block.m_end = (unsigned int)(((i - 1) - m_lastCumulative).getValue());
            m_sack.m_ptr->addGap(block);
        }
        m_sack.m_ptr->setCumulativeAck((unsigned short)m_lastCumulative.getValue());
    }
}
