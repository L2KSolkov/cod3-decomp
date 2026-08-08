// ============================================================================
// bdUnreliableReceiveWindow.cpp - incoming unreliable queue (5 funcs).
// Source: bdConnection:bdUnreliableReceiveWindow.obj
// Reconstructed from Demonware 2.0 source + COD3 release disassembly.
// ============================================================================

#include "bd/bd_types.h"

// ============================================================================
// bdUnreliableReceiveWindow::bdUnreliableReceiveWindow - ea: 0x8AAD30
// ============================================================================
bdUnreliableReceiveWindow::bdUnreliableReceiveWindow()
    : m_seqNumber(-1), m_recvQueue() {
}

// ============================================================================
// bdUnreliableReceiveWindow::~bdUnreliableReceiveWindow - ea: 0x8AABA0
// ============================================================================
bdUnreliableReceiveWindow::~bdUnreliableReceiveWindow() {
}

// ============================================================================
// bdUnreliableReceiveWindow::add - ea: 0x8AABB0
// ============================================================================
bool bdUnreliableReceiveWindow::add(const bdReference<bdDataChunk>& chunk) {
    bdSequenceNumber newSeqNum(m_seqNumber, chunk.m_ptr->getSequenceNumber(), 16);
    if (newSeqNum > m_seqNumber) {
        m_recvQueue.enqueue(chunk);
        m_seqNumber = newSeqNum;
    }
    return true;
}

// ============================================================================
// bdUnreliableReceiveWindow::getNextToRead - ea: 0x8AAC50
// ============================================================================
bdReference<bdDataChunk> bdUnreliableReceiveWindow::getNextToRead() {
    bdReference<bdDataChunk> chunk;
    if (!m_recvQueue.isEmpty()) {
        chunk.m_ptr = m_recvQueue.peek().m_ptr;
        if (chunk.m_ptr != NULL)
            chunk.m_ptr->addRef();
        m_recvQueue.dequeue();
    }
    return chunk;
}

// ============================================================================
// bdUnreliableReceiveWindow::reset - ea: 0x8AACF0
// ============================================================================
void bdUnreliableReceiveWindow::reset() {
    m_seqNumber = -1;
    while (!m_recvQueue.isEmpty())
        m_recvQueue.dequeue();
}
