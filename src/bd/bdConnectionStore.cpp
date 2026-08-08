// ============================================================================
// bdConnectionStore.cpp - manages all connections (12 funcs).
// Source: bdConnection:bdConnectionStore.obj
// Verified against IDA (release decompilation) + Demonware 2.0 reference.
// ============================================================================

#include "bd/bdConnectionStore.h"

#include <new>
#include <string.h>

#include "bd/bdSecurityKeyMap.h"
#include "bd/bdAddressMap.h"
#include "bd/bdUnicastConnection.h"

#define BD_UDP_IP_OVERHEAD 28
#define BD_MAX_DATAGRAM_SIZE 1328

// ============================================================================
// bdConnectionStore::bdConnectionStore - ea: 0x8A2DE0
// ============================================================================
bdConnectionStore::bdConnectionStore(bdSocket* socket,
                                     const bdReference<bdCommonAddr>& me,
                                     bdSecurityKeyMap* securityKeyMap,
                                     bdDHKey* dhKey,
                                     const bdArray<bdAddr>& localAddresses)
    : m_socket(socket, securityKeyMap, dhKey, localAddresses),
      m_securityKeyMap(securityKeyMap),
      m_dhKey(dhKey),
      m_me(me.m_ptr),
      m_connectionMap(4, 0.75f),
      m_listeners(),
      m_status(BD_CONNECTION_STORE_UNINITIALIZED) {
    if (m_me.m_ptr != NULL)
        m_me.m_ptr->addRef();
    bdSingleton<bdAddressMapImpl>::getInstance()->setTitleCommonAddr(m_me);
}

// ============================================================================
// bdConnectionStore::~bdConnectionStore - ea: 0x8A3740
// ============================================================================
bdConnectionStore::~bdConnectionStore() {
    closeAll();
    flushAll();
    m_connectionMap.clear();
    bdMemory::deallocate(m_listeners.m_data);
    m_listeners.m_data = NULL;
    m_listeners.m_size = 0;
    m_listeners.m_capacity = 0;
    if (m_me.m_ptr != NULL && m_me.m_ptr->releaseRef() == 0)
        delete m_me.m_ptr;
    m_me.m_ptr = NULL;
}

// ============================================================================
// bdConnectionStore::getSocketRouter - ea: 0x8A1280
// ============================================================================
bdSocketRouter* bdConnectionStore::getSocketRouter() {
    return &m_socket;
}

// ============================================================================
// bdConnectionStore::flush - ea: 0x8A1680
// ============================================================================
unsigned int bdConnectionStore::flush(bdReference<bdConnection>& connection) {
    bdReference<bdAddrHandle> addrHandle = connection.m_ptr->getAddressHandle();
    unsigned char data[1304];
    unsigned int size = connection.m_ptr->getDataToSend(data, sizeof(data));
    if (size != 0) {
        connection.m_ptr->getStats()->addBytesSent(BD_UDP_IP_OVERHEAD);
        connection.m_ptr->getStats()->addPacketsSent(1);
        connection.m_ptr->getStats()->addPacketSizeSent(size);
        int status = m_socket.sendTo(addrHandle, data, size);
        if (status < 0) {
            switch (status) {
            case BD_NET_ADDRESS_INVALID: {
                bdMessageProxy proxy(".\\bdConnectionStore.cpp",
                                     "unsigned int __thiscall bdConnectionStore::flush(class bdReference<class bdConnection>)",
                                     0x137u, "dw/info/");
                proxy.log("bdConnection/connectionstore", "Invalid address. Closing connection.");
                connection.m_ptr->close();
                break;
            }
            case BD_NET_SUBSYTEM_ERROR: {
                bdMessageProxy proxy(".\\bdConnectionStore.cpp",
                                     "unsigned int __thiscall bdConnectionStore::flush(class bdReference<class bdConnection>)",
                                     0x134u, "dw/warn/");
                proxy.log("bdConnection/connectionstore", "net subsystem error!");
                break;
            }
            case BD_NET_WOULD_BLOCK: {
                bdMessageProxy proxy(".\\bdConnectionStore.cpp",
                                     "unsigned int __thiscall bdConnectionStore::flush(class bdReference<class bdConnection>)",
                                     0x140u, "dw/warn/");
                proxy.log("bdConnection/connectionstore", "would block.");
                break;
            }
            case BD_NET_ERROR: {
                bdMessageProxy proxy(".\\bdConnectionStore.cpp",
                                     "unsigned int __thiscall bdConnectionStore::flush(class bdReference<class bdConnection>)",
                                     0x13Cu, "dw/warn/");
                proxy.log("bdConnection/connectionstore", "unknown error.");
                break;
            }
            default: {
                bdMessageProxy proxy(".\\bdConnectionStore.cpp",
                                     "unsigned int __thiscall bdConnectionStore::flush(class bdReference<class bdConnection>)",
                                     0x14Au, "dw/warn/");
                proxy.log("bdConnection/connectionstore", "unknown error.");
                break;
            }
            }
        }
    }
    return size;
}

// ============================================================================
// bdConnectionStore::flushAll - ea: 0x8A3220
// ============================================================================
bool bdConnectionStore::flushAll() {
    m_socket.pump();

    bdLinkedList<bdReference<bdConnection> > toDisconnect;
    bdHashMap<bdAddrHandleHashmapWrapper, bdReference<bdConnection>,
              bdAddrHandleHashmapWrapper>::Node* it = m_connectionMap.getIterator();

    while (it != NULL) {
        bdReference<bdConnection> connection = m_connectionMap.getValue(it);
        bdReference<bdCommonAddr> addr = connection.m_ptr->getAddress();
        if (!addr.m_ptr->isLoopback()) {
            bdReference<bdAddrHandle> addrHandle = connection.m_ptr->getAddressHandle();
            bdSocketAssociationStatus status = m_socket.getStatus(addrHandle);
            switch (status) {
            case BD_SOCKET_CONNECTED: {
                while (flush(connection) != 0) {
                }
                if (connection.m_ptr->getStatus() == bdConnection::BD_DISCONNECTED) {
                    char addrStr[24];
                    bdSingleton<bdAddressMapImpl>::getInstance()->addrToString(addrHandle, addrStr, 22);
                    bdMessageProxy proxy(".\\bdConnectionStore.cpp",
                                         "bool __thiscall bdConnectionStore::flushAll(void)",
                                         0xC0u, "dw/info/");
                    proxy.log("bdConnection/connectionstore", "Connection state disconnected. Disconnecting %s.", addrStr);
                    bdReference<bdConnection> copy = connection;
                    toDisconnect.addTail(copy);
                }
                break;
            }
            case BD_SOCKET_LOST: {
                char addrStr[24];
                bdSingleton<bdAddressMapImpl>::getInstance()->addrToString(addrHandle, addrStr, 22);
                bdMessageProxy proxy(".\\bdConnectionStore.cpp",
                                     "bool __thiscall bdConnectionStore::flushAll(void)",
                                     0xCAu, "dw/info/");
                proxy.log("bdConnection/connectionstore", "Socket router reports socket lost. Disconnecting %s.", addrStr);
                bdReference<bdConnection> copy = connection;
                toDisconnect.addTail(copy);
                break;
            }
            case BD_SOCKET_IDLE:
            default: {
                bdMessageProxy proxy(".\\bdConnectionStore.cpp",
                                     "bool __thiscall bdConnectionStore::flushAll(void)",
                                     0xB0u, "dw/warn/");
                proxy.log("bdConnection/connectionstore", "bad socket status.");
                break;
            }
            case BD_SOCKET_PENDING:
                break;
            }
        } else {
            bdLoopbackConnection* loopback = (bdLoopbackConnection*)connection.m_ptr;
            loopback->updateStatus();
            if (connection.m_ptr->getStatus() == bdConnection::BD_DISCONNECTED) {
                bdReference<bdConnection> copy = connection;
                toDisconnect.addTail(copy);
            }
        }
        m_connectionMap.next(it);
    }

    while (!toDisconnect.isEmpty()) {
        bdReference<bdConnection> connection = toDisconnect.getHead();
        bdReference<bdAddrHandle> addrHandle = connection.m_ptr->getAddressHandle();
        connection.m_ptr->close();
        m_connectionMap.remove(bdAddrHandleHashmapWrapper(addrHandle));
        bdSingleton<bdAddressMapImpl>::getInstance()->unregisterAddr(addrHandle);
        toDisconnect.removeHead();
    }
    return true;
}

// ============================================================================
// bdConnectionStore::receiveFrom - ea: 0x8A21B0
// ============================================================================
bool bdConnectionStore::receiveFrom(bdReference<bdConnection>& connection) {
    bool done = false;
    bool gotConnection = false;
    unsigned char data[1304];
    int iteration = 0;

    while (!done) {
        bdReference<bdAddrHandle> addrHandle;
        int received = m_socket.receiveFrom(addrHandle, data, sizeof(data));
        if (received < 0) {
            switch (received) {
            case BD_NET_WOULD_BLOCK:
            case BD_NET_NOT_BOUND:
            case BD_NET_BLOCKING_CALL_CANCELED:
                done = true;
                break;
            case BD_NET_MSG_SIZE: {
                bdMessageProxy proxy(".\\bdConnectionStore.cpp",
                                     "bool __thiscall bdConnectionStore::receiveFrom(class bdReference<class bdConnection> &)",
                                     0x178u, "dw/warn/");
                proxy.log("bdConnection/connectionstore", "Couldn't receive message. Buffer too small?");
                break;
            }
            case BD_NET_CONNECTION_RESET: {
                bdReference<bdConnection> existing;
                if (m_connectionMap.get(bdAddrHandleHashmapWrapper(addrHandle), existing) && existing.m_ptr != NULL)
                    existing.m_ptr->close();
                break;
            }
            default: {
                bdMessageProxy proxy(".\\bdConnectionStore.cpp",
                                     "bool __thiscall bdConnectionStore::receiveFrom(class bdReference<class bdConnection> &)",
                                     0x183u, "dw/warn/");
                proxy.log("bdConnection/connectionstore", "net error.");
                done = true;
                break;
            }
            }
            continue;
        }
        if (received <= 0)
            continue;

        bdReference<bdConnection> existing;
        bool found = m_connectionMap.get(bdAddrHandleHashmapWrapper(addrHandle), existing);
        if (found && existing.m_ptr != NULL) {
            existing.m_ptr->receive(data, (unsigned int)received);
            if (connection.m_ptr != NULL && connection.m_ptr->releaseRef() == 0)
                delete connection.m_ptr;
            connection.m_ptr = existing.m_ptr;
            if (connection.m_ptr != NULL)
                connection.m_ptr->addRef();
            gotConnection = true;
        } else {
            bdUnicastConnection* uc = new (bdMemory::allocate(sizeof(bdUnicastConnection))) bdUnicastConnection();
            uc->addRef();
            bdReference<bdConnection> newConn(uc);
            for (unsigned int i = 0; i < m_listeners.m_size; i++)
                uc->registerListener(m_listeners.m_data[i]);
            uc->setAddressHandle(addrHandle);
            uc->receive(data, (unsigned int)received);
            if (uc->getStatus() == bdConnection::BD_CONNECTED) {
                bdMessageProxy proxy(".\\bdConnectionStore.cpp",
                                     "bool __thiscall bdConnectionStore::receiveFrom(class bdReference<class bdConnection> &)",
                                     0x1A2u, "dw/info/");
                proxy.log("bdConnection/connectionstore", "New incoming connection created.");
                m_connectionMap.put(bdAddrHandleHashmapWrapper(addrHandle), newConn);
                if (connection.m_ptr != NULL && connection.m_ptr->releaseRef() == 0)
                    delete connection.m_ptr;
                connection.m_ptr = uc;
                if (connection.m_ptr != NULL)
                    connection.m_ptr->addRef();
                gotConnection = true;
            } else {
                flush(newConn);
            }
            if (newConn.m_ptr != NULL && newConn.m_ptr->releaseRef() == 0)
                delete newConn.m_ptr;
        }
        if (gotConnection)
            break;
        iteration++;
        if (iteration > 8)
            break;
    }
    return gotConnection;
}

// ============================================================================
// bdConnectionStore::disconnectAll - ea: 0x8A1D20
// ============================================================================
void bdConnectionStore::disconnectAll() {
    bdMessageProxy proxy(".\\bdConnectionStore.cpp",
                         "void __thiscall bdConnectionStore::disconnectAll(void)",
                         0x8Du, "dw/info/");
    proxy.log("bdConnection/connectionstore", "Disconnecting all connections.");
    bdHashMap<bdAddrHandleHashmapWrapper, bdReference<bdConnection>,
              bdAddrHandleHashmapWrapper>::Node* it = m_connectionMap.getIterator();
    while (it != NULL) {
        bdReference<bdConnection> connection = m_connectionMap.getValue(it);
        connection.m_ptr->disconnect();
        m_connectionMap.next(it);
    }
}

// ============================================================================
// bdConnectionStore::closeAll - ea: 0x8A2B30
// ============================================================================
void bdConnectionStore::closeAll() {
    bdMessageProxy proxy(".\\bdConnectionStore.cpp",
                         "void __thiscall bdConnectionStore::closeAll(void)",
                         0x7Au, "dw/info/");
    proxy.log("bdConnection/connectionstore", "Closing all connections.");
    bdHashMap<bdAddrHandleHashmapWrapper, bdReference<bdConnection>,
              bdAddrHandleHashmapWrapper>::Node* it = m_connectionMap.getIterator();
    while (it != NULL) {
        bdReference<bdConnection> connection = m_connectionMap.getValue(it);
        connection.m_ptr->close();
        m_connectionMap.next(it);
    }
    m_connectionMap.clear();
}

// ============================================================================
// bdConnectionStore::registerListener - ea: 0x8A2180
// ============================================================================
void bdConnectionStore::registerListener(bdConnectionListener* listener) {
    m_listeners.pushBack(listener);
}

// ============================================================================
// bdConnectionStore::unregisterListener - ea: 0x8A2A00
// ============================================================================
void bdConnectionStore::unregisterListener(bdConnectionListener* listener) {
    m_listeners.removeAllKeepOrder(listener);
}

// ============================================================================
// bdConnectionStore::remove - ea: 0x8A2A10
// ============================================================================
void bdConnectionStore::remove(const bdReference<bdConnection>& connection) {
    bdMessageProxy proxy(".\\bdConnectionStore.cpp",
                         "void __thiscall bdConnectionStore::remove(class bdReference<class bdConnection>)",
                         0x6Eu, "dw/info/");
    proxy.log("bdConnection/connectionstore", "Removing connection.");
    bdReference<bdAddrHandle> addrHandle = connection.m_ptr->getAddressHandle();
    bool removed = m_connectionMap.remove(bdAddrHandleHashmapWrapper(addrHandle));
    if (!removed) {
        bdMessageProxy proxy(".\\bdConnectionStore.cpp",
                             "void __thiscall bdConnectionStore::remove(class bdReference<class bdConnection>)",
                             0x74u, "dw/warn/");
        proxy.log("bdConnection/connectionstore", "failed to remove connection from store, connection not present.");
    }
}

// ============================================================================
// bdConnectionStore::create - ea: 0x8A2EB0
// ============================================================================
bdReference<bdConnection> bdConnectionStore::create(const bdReference<bdCommonAddr>& addr,
                                                    const XNKID& secID) {
    bdReference<bdConnection> connection;
    if (addr.m_ptr == NULL) {
        bdMessageProxy proxy(".\\bdConnectionStore.cpp",
                             "class bdReference<class bdConnection> __thiscall bdConnectionStore::create(class bdReference<class bdCommonAddr>,const XNKID &)",
                             0x3Eu, "dw/err");
        proxy.log("defaultFileName", "Common address is null.");
        return connection;
    }

    bdReference<bdAddrHandle> addrHandle;
    if (!bdSingleton<bdAddressMapImpl>::getInstance()->commonAddrToAddr(addr, secID, addrHandle))
        return connection;

    if (m_connectionMap.get(bdAddrHandleHashmapWrapper(addrHandle), connection)) {
        bdMessageProxy proxy(".\\bdConnectionStore.cpp",
                             "class bdReference<class bdConnection> __thiscall bdConnectionStore::create(class bdReference<class bdCommonAddr>,const XNKID &)",
                             0x64u, "dw/warn/");
        proxy.log("bdConnection/connectionstore", "a connection already exists to host.");
        return connection;
    }

    if (addr.m_ptr->isLoopback()) {
        bdMessageProxy proxy(".\\bdConnectionStore.cpp",
                             "class bdReference<class bdConnection> __thiscall bdConnectionStore::create(class bdReference<class bdCommonAddr>,const XNKID &)",
                             0x44u, "dw/info/");
        proxy.log("bdConnection/connectionstore", "Creating new loopback connection.");
        bdLoopbackConnection* lb = new (bdMemory::allocate(sizeof(bdLoopbackConnection)))
            bdLoopbackConnection(addr);
        connection.m_ptr = lb;
        if (connection.m_ptr != NULL)
            connection.m_ptr->addRef();
        connection.m_ptr->setAddressHandle(addrHandle);
        if (!m_connectionMap.put(bdAddrHandleHashmapWrapper(addrHandle), connection))
            connection.m_ptr = NULL;
    } else {
        bdMessageProxy proxy(".\\bdConnectionStore.cpp",
                             "class bdReference<class bdConnection> __thiscall bdConnectionStore::create(class bdReference<class bdCommonAddr>,const XNKID &)",
                             0x49u, "dw/info/");
        proxy.log("bdConnection/connectionstore", "Creating new unicast connection.");
        bdUnicastConnection* uc = new (bdMemory::allocate(sizeof(bdUnicastConnection)))
            bdUnicastConnection(addr);
        connection.m_ptr = uc;
        if (connection.m_ptr != NULL)
            connection.m_ptr->addRef();
        connection.m_ptr->setAddressHandle(addrHandle);
        bool ok = m_socket.connect(addrHandle);
        ok = ok && m_connectionMap.put(bdAddrHandleHashmapWrapper(addrHandle), connection);
        if (!ok) {
            connection.m_ptr->disconnect();
            connection.m_ptr->close();
            connection.m_ptr = NULL;
        }
    }
    return connection;
}