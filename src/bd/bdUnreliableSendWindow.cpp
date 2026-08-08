// ============================================================================
// bdUnreliableSendWindow.cpp - outgoing unreliable queue (5 funcs).
// Source: bdConnection:bdUnreliableSendWindow.obj
// Reconstructed from Demonware 2.0 source + COD3 release disassembly.
// ============================================================================

#include "bd/bd_types.h"

// ============================================================================
// bdUnreliableSendWindow::bdUnreliableSendWindow - ea: 0x8AA0A0
// ============================================================================
bdUnreliableSendWindow::bdUnreliableSendWindow()
    : m_seqNumber(0), m_sendQueue() {
}

// ============================================================================
// bdUnreliableSendWindow::~bdUnreliableSendWindow - ea: 0x8A9F40
// ============================================================================
bdUnreliableSendWindow::~bdUnreliableSendWindow() {
}

// ============================================================================
// bdUnreliableSendWindow::add - ea: 0x8AA0E0
// ============================================================================
void bdUnreliableSendWindow::add(const bdReference<bdDataChunk>& chunk) {
    m_sendQueue.enqueue(chunk);
}

// ============================================================================
// bdUnreliableSendWindow::getDataToSend - ea: 0x8A9F50
// ============================================================================
void bdUnreliableSendWindow::getDataToSend(bdPacket& packet) {
    bool addedChunk = true;
    bdDataChunk* chunk = NULL;

    while (!m_sendQueue.isEmpty() && addedChunk) {
        bdDataChunk* queued = m_sendQueue.peek().m_ptr;
        if (chunk != NULL && chunk->releaseRef() == 0)
            delete chunk;
        chunk = queued;
        if (chunk != NULL)
            chunk->addRef();

        chunk->setSequenceNumber(m_seqNumber);
        if (packet.addChunk(bdReference<bdChunk>(chunk))) {
            ++m_seqNumber;
            m_sendQueue.dequeue();
        } else {
            addedChunk = false;
        }
    }

    if (chunk != NULL && chunk->releaseRef() == 0)
        delete chunk;
}

// ============================================================================
// bdUnreliableSendWindow::reset - ea: 0x8AA050
// ============================================================================
void bdUnreliableSendWindow::reset() {
    m_seqNumber = 0;
    while (!m_sendQueue.isEmpty())
        m_sendQueue.dequeue();
}
