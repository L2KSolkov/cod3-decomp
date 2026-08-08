// ============================================================================
// bdReliableSendWindow.cpp - outgoing reliable-message window (10 funcs).
// Source: bdConnection:bdReliableSendWindow.obj
// Reconstructed from Demonware 2.0 source + COD3 release disassembly.
// ============================================================================

#include "bd/bd_types.h"

// ============================================================================
// bdMessageFrame helpers
// ============================================================================
bdReliableSendWindow::bdMessageFrame::bdMessageFrame(const bdReference<bdDataChunk>& chunk)
    : m_chunk(chunk.m_ptr), m_timer(), m_sendCount(0), m_missingCount(0),
      m_gapAcked(false) {
    if (m_chunk.m_ptr != NULL)
        m_chunk.m_ptr->addRef();
}

bdReliableSendWindow::bdMessageFrame::~bdMessageFrame() {
    if (m_chunk.m_ptr != NULL && m_chunk.m_ptr->releaseRef() == 0)
        delete m_chunk.m_ptr;
    m_chunk.m_ptr = NULL;
}

bdReliableSendWindow::bdMessageFrame&
bdReliableSendWindow::bdMessageFrame::operator=(const bdMessageFrame& other) {
    if (this != &other) {
        if (m_chunk.m_ptr != NULL && m_chunk.m_ptr->releaseRef() == 0)
            delete m_chunk.m_ptr;
        m_chunk.m_ptr = other.m_chunk.m_ptr;
        if (m_chunk.m_ptr != NULL)
            m_chunk.m_ptr->addRef();
    }
    m_timer.m_start = other.m_timer.m_start;
    m_sendCount = other.m_sendCount;
    m_missingCount = other.m_missingCount;
    m_gapAcked = other.m_gapAcked;
    return *this;
}

// ============================================================================
// bdReliableSendWindow::bdReliableSendWindow - ea: 0x8A8CB0
// ============================================================================
bdReliableSendWindow::bdReliableSendWindow()
    : m_lastAcked(-1),
      m_nextFree(0),
      m_timeoutPeriod(BD_RTT_START_VALUE),
      m_retransmitCountThreshold(BD_FAST_RETRANSMIT_THRESH),
      m_remoteReceiveWindowCredit(BD_DEFAULT_RECEIVE_WINDOW_CREDIT),
      m_flightSize(0),
      m_partialBytesAcked(0),
      m_slowStartThresh(BD_DEFAULT_RECEIVE_WINDOW_CREDIT),
      m_congestionWindow(2 * BD_MAX_DATAGRAM_SIZE) {
    m_lastSent.start();
}

// ============================================================================
// bdReliableSendWindow::~bdReliableSendWindow - ea: 0x8A8D60
// ============================================================================
bdReliableSendWindow::~bdReliableSendWindow() {
}

// ============================================================================
// bdReliableSendWindow::setTimeoutPeriod - ea: 0x8A8A00
// ============================================================================
void bdReliableSendWindow::setTimeoutPeriod(float secs) {
    m_timeoutPeriod = secs;
}

// ============================================================================
// bdReliableSendWindow::getTimeoutPeriod - ea: 0x8A8A10
// ============================================================================
float bdReliableSendWindow::getTimeoutPeriod() const {
    return m_timeoutPeriod;
}

// ============================================================================
// bdReliableSendWindow::add - ea: 0x8A9520
// ============================================================================
bool bdReliableSendWindow::add(const bdReference<bdDataChunk>& chunk) {
    bool messageAdded = false;
    unsigned int index = (unsigned int)m_nextFree.getValue() % BD_MAX_WINDOW_SIZE;
    bdMessageFrame& frameSlot = m_frame[index];

    if (frameSlot.m_chunk.m_ptr == NULL) {
        chunk.m_ptr->setSequenceNumber((unsigned short)m_nextFree.getValue());
        frameSlot = bdMessageFrame(chunk);
        ++m_nextFree;
        messageAdded = true;
    } else {
        bdMessageProxy proxy(".\\bdWindow\\bdReliableSendWindow.cpp",
                             "bool __thiscall bdReliableSendWindow::add(class bdReference<class bdDataChunk>)",
                             0x3Eu, "dw/warn/");
        proxy.log("bdConnection/windows", "reliable message window is full.");
    }

    return messageAdded;
}

// ============================================================================
// bdReliableSendWindow::getDataToSend - ea: 0x8A8DD0
// ============================================================================
void bdReliableSendWindow::getDataToSend(bdPacket& packet) {
    bool retransmitTimerExpired = false;
    bool neededFastRetransmit = false;
    bool haveDataToSend = false;

    for (bdSequenceNumber i = m_lastAcked + 1; i < m_nextFree; i++) {
        unsigned int index = (unsigned int)i.getValue() % BD_MAX_WINDOW_SIZE;
        if (m_frame[index].m_chunk.m_ptr != NULL) {
            bdMessageFrame& frameSlot = m_frame[index];
            bdSequenceNumber frameSeqNum(m_lastAcked,
                                         frameSlot.m_chunk.m_ptr->getSequenceNumber(), 16);
            if (frameSeqNum.getValue() != i.getValue()) {
                bdMessageProxy proxy(".\\bdWindow\\bdReliableSendWindow.cpp",
                                     "void __thiscall bdReliableSendWindow::getDataToSend(class bdPacket &)",
                                     0x5Au, "dw/err/");
                proxy.log("bdConnection/windows", "Window error");
            }
            if (frameSlot.m_chunk.m_ptr != NULL) {
                unsigned int serializedSize = frameSlot.m_chunk.m_ptr->getSerializedSize();
                if (frameSlot.m_sendCount == 0) {
                    break;
                } else if (frameSlot.m_missingCount >= m_retransmitCountThreshold) {
                    neededFastRetransmit = true;
                    m_flightSize -= serializedSize;
                } else if (frameSlot.m_timer.getElapsedTimeInSeconds() > m_timeoutPeriod) {
                    retransmitTimerExpired = true;
                    m_flightSize -= serializedSize;
                }
            }
        }
    }

    if (neededFastRetransmit)
        decreaseCongestionWindow(BD_CWDR_PACKET_LOSS_DETECTED);

    if (retransmitTimerExpired) {
        decreaseCongestionWindow(BD_CWDR_RESEND_TIMER_EXPIRED);
        m_timeoutPeriod = 2.0f * m_timeoutPeriod;
        if (m_timeoutPeriod > BD_UC_RTO_MAX)
            m_timeoutPeriod = BD_UC_RTO_MAX;
    }

    bool full = false;
    for (bdSequenceNumber i = m_lastAcked + 1; i < m_nextFree && !full; i++) {
        unsigned int index = (unsigned int)i.getValue() % BD_MAX_WINDOW_SIZE;
        bdMessageFrame& frameSlot = m_frame[index];

        if (frameSlot.m_chunk.m_ptr != NULL) {
            bdDataChunk* chunk = frameSlot.m_chunk.m_ptr;

            if (frameSlot.m_sendCount == 0) {
                unsigned int serializedSize = chunk->getSerializedSize();
                bool enoughWindowCredit =
                    m_remoteReceiveWindowCredit - m_flightSize > serializedSize;
                bool fitsInPMTUAndCwndNotExceeded =
                    m_flightSize < m_congestionWindow
                        ? (serializedSize < (unsigned int)BD_MAX_DATAGRAM_SIZE)
                        : false;

                if (enoughWindowCredit) {
                    if (packet.addChunk(bdReference<bdChunk>(chunk))) {
                        frameSlot.m_sendCount++;
                        frameSlot.m_timer.start();
                        m_flightSize += serializedSize;
                        haveDataToSend = true;
                    } else {
                        bdMessageProxy proxy(".\\bdWindow\\bdReliableSendWindow.cpp",
                                             "void __thiscall bdReliableSendWindow::getDataToSend(class bdPacket &)",
                                             0xD4u, "dw/info/");
                        proxy.log("bdConnection/windows", "packet full.");
                        full = true;
                    }
                } else if (fitsInPMTUAndCwndNotExceeded) {
                    if (packet.addChunk(bdReference<bdChunk>(chunk))) {
                        bdMessageProxy proxy(".\\bdWindow\\bdReliableSendWindow.cpp",
                                             "void __thiscall bdReliableSendWindow::getDataToSend(class bdPacket &)",
                                             0xDFu, "dw/info/");
                        proxy.log("bdConnection/windows", "sent 1 new packet %u (rule b)",
                                  chunk->getSequenceNumber());
                        frameSlot.m_sendCount++;
                        frameSlot.m_timer.start();
                        m_flightSize += serializedSize;
                        haveDataToSend = true;
                        full = true;
                    } else {
                        bdMessageProxy proxy(".\\bdWindow\\bdReliableSendWindow.cpp",
                                             "void __thiscall bdReliableSendWindow::getDataToSend(class bdPacket &)",
                                             0xEAu, "dw/info/");
                        proxy.log("bdConnection/windows", "packet full.");
                        full = true;
                    }
                } else {
                    full = true;
                }
            } else if (frameSlot.m_missingCount >= m_retransmitCountThreshold) {
                if (packet.addChunk(bdReference<bdChunk>(chunk))) {
                    bdMessageProxy proxy(".\\bdWindow\\bdReliableSendWindow.cpp",
                                         "void __thiscall bdReliableSendWindow::getDataToSend(class bdPacket &)",
                                         0xF6u, "dw/info/");
                    proxy.log("bdConnection/windows", "sent retransmit (fast retransmit) %u",
                              chunk->getSequenceNumber());
                    frameSlot.m_missingCount = 0;
                    frameSlot.m_sendCount++;
                    frameSlot.m_timer.start();
                    m_flightSize += chunk->getSerializedSize();
                    haveDataToSend = true;
                } else {
                    bdMessageProxy proxy(".\\bdWindow\\bdReliableSendWindow.cpp",
                                         "void __thiscall bdReliableSendWindow::getDataToSend(class bdPacket &)",
                                         0x101u, "dw/info/");
                    proxy.log("bdConnection/windows", "packet full.");
                    full = true;
                }
            } else if (frameSlot.m_timer.getElapsedTimeInSeconds() > m_timeoutPeriod) {
                bool congestionWindowAllows = m_flightSize < m_congestionWindow;
                if (congestionWindowAllows) {
                    if (packet.addChunk(bdReference<bdChunk>(chunk))) {
                        bdMessageProxy proxy(".\\bdWindow\\bdReliableSendWindow.cpp",
                                             "void __thiscall bdReliableSendWindow::getDataToSend(class bdPacket &)",
                                             0x10Fu, "dw/info/");
                        proxy.log("bdConnection/windows", "sent retransmit (rto timeout) %u",
                                  chunk->getSequenceNumber());
                        frameSlot.m_missingCount = 0;
                        frameSlot.m_sendCount++;
                        frameSlot.m_timer.start();
                        m_flightSize += chunk->getSerializedSize();
                        haveDataToSend = true;
                    } else {
                        bdMessageProxy proxy(".\\bdWindow\\bdReliableSendWindow.cpp",
                                             "void __thiscall bdReliableSendWindow::getDataToSend(class bdPacket &)",
                                             0x11Au, "dw/info/");
                        proxy.log("bdConnection/windows", "packet full.");
                        full = true;
                    }
                }
            }
        }
    }

    if (!haveDataToSend) {
        if (m_lastSent.getElapsedTimeInSeconds() > 1.0f) {
            decreaseCongestionWindow(BD_CWDR_INACTIVE);
            m_lastSent.start();
        }
    } else {
        m_lastSent.start();
    }
}

// ============================================================================
// bdReliableSendWindow::handleAck - ea: 0x8A9660
// ============================================================================
bool bdReliableSendWindow::handleAck(const bdReference<bdSAckChunk>& chunk, float& rtt) {
    bdSequenceNumber ack(m_lastAcked, chunk.m_ptr->getCumulativeAck(), 16);
    bdSequenceNumber lastSent = m_nextFree - 1;
    if (ack > lastSent) {
        bdMessageProxy proxy(".\\bdWindow\\bdReliableSendWindow.cpp",
                             "bool __thiscall bdReliableSendWindow::handleAck(class bdReference<class bdSAckChunk>,float &)",
                             0x13Fu, "dw/warn/");
        proxy.log("bdConnection/windows", "Acking unsent chunk.");
    }

    if (ack.getValue() >= m_lastAcked.getValue()) {
        unsigned int index = (unsigned int)ack.getValue() % BD_MAX_WINDOW_SIZE;

        if (m_frame[index].m_chunk.m_ptr != NULL && m_frame[index].m_sendCount == 1)
            rtt = m_frame[index].m_timer.getElapsedTimeInSeconds();
        else
            rtt = 0.0f;

        m_remoteReceiveWindowCredit = chunk.m_ptr->getWindowCredit();
        m_flightSize = 0;
        unsigned int count = 0;
        for (bdSequenceNumber i = ack + 1; count < BD_MAX_WINDOW_SIZE; i++) {
            unsigned int idx = (unsigned int)i.getValue() % BD_MAX_WINDOW_SIZE;
            bdMessageFrame& frameSlot = m_frame[idx];
            if (frameSlot.m_chunk.m_ptr != NULL)
                m_flightSize += frameSlot.m_chunk.m_ptr->getSerializedSize() * frameSlot.m_sendCount;
            else
                break;
            count++;
        }

        unsigned int bytesAcked = 0;
        for (bdSequenceNumber i = m_lastAcked + 1; i <= ack; i++) {
            unsigned int idx = (unsigned int)i.getValue() % BD_MAX_WINDOW_SIZE;
            if (m_frame[idx].m_chunk.m_ptr != NULL) {
                bdMessageFrame& frameSlot = m_frame[idx];
                bytesAcked += frameSlot.m_chunk.m_ptr->getSerializedSize();
                if (frameSlot.m_chunk.m_ptr->releaseRef() == 0)
                    delete frameSlot.m_chunk.m_ptr;
                frameSlot.m_chunk.m_ptr = NULL;
                frameSlot.m_timer.reset();
            } else {
                bdMessageProxy proxy(".\\bdWindow\\bdReliableSendWindow.cpp",
                                     "bool __thiscall bdReliableSendWindow::handleAck(class bdReference<class bdSAckChunk>,float &)",
                                     0x183u, "dw/warn/");
                proxy.log("bdConnection/windows", "Invalid ack.");
            }
        }

        bdLinkedList<bdGapAckBlock>& blocks = chunk.m_ptr->getGapList();
        bdSequenceNumber lastSeq = ack + 1;
        while (!blocks.isEmpty()) {
            bdGapAckBlock& block = blocks.getHead();
            bdSequenceNumber startSeq = ack + block.m_start;
            bdSequenceNumber endSeq = ack + block.m_end;

            for (bdSequenceNumber i = lastSeq; i <= endSeq; i++) {
                unsigned int idx = (unsigned int)i.getValue() % BD_MAX_WINDOW_SIZE;
                bdMessageFrame& frameSlot = m_frame[idx];

                if (frameSlot.m_chunk.m_ptr == NULL) {
                    bdMessageProxy proxy(".\\bdWindow\\bdReliableSendWindow.cpp",
                                         "bool __thiscall bdReliableSendWindow::handleAck(class bdReference<class bdSAckChunk>,float &)",
                                         0x195u, "dw/err");
                    proxy.log("defaultFileName", "Shouldn't be null!");
                }
                if (frameSlot.m_sendCount == 0) {
                    bdMessageProxy proxy(".\\bdWindow\\bdReliableSendWindow.cpp",
                                         "bool __thiscall bdReliableSendWindow::handleAck(class bdReference<class bdSAckChunk>,float &)",
                                         0x199u, "dw/warn/");
                    proxy.log("bdConnection/windows", "%x Send count should be > 0",
                              (unsigned int)this);
                }

                if (i < startSeq) {
                    frameSlot.m_missingCount++;
                    if (frameSlot.m_gapAcked) {
                        frameSlot.m_gapAcked = false;
                        if (frameSlot.m_timer.getElapsedTimeInSeconds() == 0.0f)
                            frameSlot.m_timer.start();
                    }
                } else {
                    frameSlot.m_gapAcked = true;
                    frameSlot.m_timer.start();
                    bytesAcked += frameSlot.m_chunk.m_ptr->getSerializedSize();
                }
            }

            lastSeq = endSeq + 1;
            blocks.removeHead();
        }

        increaseCongestionWindow(chunk, bytesAcked);
        m_lastAcked = ack;
    }

    return true;
}

// ============================================================================
// bdReliableSendWindow::isEmpty - ea: 0x8A93B0
// ============================================================================
bool bdReliableSendWindow::isEmpty() const {
    bool empty = true;
    for (bdSequenceNumber i = m_lastAcked; i < m_nextFree && empty; i++) {
        unsigned int index = (unsigned int)i.getValue() % BD_MAX_WINDOW_SIZE;
        if (m_frame[index].m_chunk.m_ptr != NULL)
            empty = false;
    }
    return empty;
}

// ============================================================================
// bdReliableSendWindow::increaseCongestionWindow - ea: 0x8A9420
// ============================================================================
void bdReliableSendWindow::increaseCongestionWindow(const bdReference<bdSAckChunk>& chunk,
                                                    unsigned int bytesAcked) {
    (void)chunk;
    unsigned int cwnd = m_congestionWindow;
    if (m_flightSize >= m_congestionWindow) {
        if (m_congestionWindow > m_slowStartThresh) {
            m_partialBytesAcked += bytesAcked;
            if (m_partialBytesAcked >= m_congestionWindow) {
                cwnd += BD_MAX_DATAGRAM_SIZE;
                m_congestionWindow = cwnd;
                if (cwnd >= m_partialBytesAcked)
                    m_partialBytesAcked = 0;
                else
                    m_partialBytesAcked = m_partialBytesAcked - cwnd;
            }
        } else {
            if (bytesAcked <= (unsigned int)BD_MAX_DATAGRAM_SIZE)
                cwnd += bytesAcked;
            else
                cwnd += BD_MAX_DATAGRAM_SIZE;
            m_congestionWindow = cwnd;
        }
    }
}

// ============================================================================
// bdReliableSendWindow::decreaseCongestionWindow - ea: 0x8A8A20
// ============================================================================
void bdReliableSendWindow::decreaseCongestionWindow(bdCongestionWindowDecreaseReason reason) {
    switch (reason) {
    case BD_CWDR_PACKET_LOSS_DETECTED: {
        unsigned int result = m_congestionWindow / 2;
        if (result <= 2 * (unsigned int)BD_MAX_DATAGRAM_SIZE)
            result = 2 * BD_MAX_DATAGRAM_SIZE;
        m_slowStartThresh = result;
        m_congestionWindow = result;
        m_partialBytesAcked = 0;
        break;
    }
    case BD_CWDR_RESEND_TIMER_EXPIRED: {
        unsigned int result = m_congestionWindow / 2;
        if (result <= 2 * (unsigned int)BD_MAX_DATAGRAM_SIZE)
            result = 2 * BD_MAX_DATAGRAM_SIZE;
        m_slowStartThresh = result;
        m_congestionWindow = BD_MAX_DATAGRAM_SIZE;
        m_partialBytesAcked = 0;
        break;
    }
    case BD_CWDR_INACTIVE: {
        unsigned int result = m_congestionWindow / 2;
        if (result <= 4 * (unsigned int)BD_MAX_DATAGRAM_SIZE) {
            m_congestionWindow = 4 * BD_MAX_DATAGRAM_SIZE;
            m_partialBytesAcked = 0;
        } else {
            m_congestionWindow = result;
            m_partialBytesAcked = 0;
        }
        break;
    }
    }
}
