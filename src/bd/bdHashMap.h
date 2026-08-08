// ============================================================================
// bdHashMap - open-addressing chained hash map (24 bytes).
// Source: bdConnection:bdConnectionStore.obj COMDATs.
// Layout verified against IDA: m_size +0, m_capacity +4, m_loadFactor +8,
// m_threshold +0xC, m_map +0x10, m_numIterators +0x14.
// Node = { TValue, TKey, Node* } (12 bytes for pointer-valued key/value).
// ============================================================================

#ifndef COD3_BD_BDHASHMAP_H
#define COD3_BD_BDHASHMAP_H

#include "bd/bd_types.h"

inline unsigned int bdBitOperations_nextPowerOf2(unsigned int v) {
    unsigned int p = 1;
    while (p < v)
        p <<= 1;
    return p;
}

// ============================================================================
// bdAddrHandleHashmapWrapper - map key wrapper (4 bytes). The COD3 map hashes
// keys by reading bdAddrHandle::m_addr at +8 (the IPv4 dword).
// ============================================================================
struct bdAddrHandleHashmapWrapper {
    bdReference<bdAddrHandle> m_handle;   // +0x00

    bdAddrHandleHashmapWrapper() : m_handle() {}
    bdAddrHandleHashmapWrapper(const bdReference<bdAddrHandle>& handle) : m_handle(handle) {}

    static unsigned int hash(const bdAddrHandleHashmapWrapper& key) {
        return key.m_handle.m_ptr != NULL ? key.m_handle.m_ptr->m_addr.inUn.m_iaddr : 0;
    }
};
static_assert(sizeof(bdAddrHandleHashmapWrapper) == 0x04, "bdAddrHandleHashmapWrapper size mismatch");

// ============================================================================
// bdHashMap<TKey, TValue, THashingClass>
// ============================================================================
template <typename TKey, typename TValue, typename THashingClass>
class bdHashMap {
public:
    struct Node {
        TValue m_value;   // +0x00
        TKey   m_key;     // +sizeof(TValue)
        Node*  m_next;    // +2*sizeof(TValue)
    };

    unsigned int m_size;          // +0x00
    unsigned int m_capacity;      // +0x04
    float        m_loadFactor;    // +0x08
    float        m_threshold;     // +0x0C
    Node**       m_map;           // +0x10
    unsigned int m_numIterators;  // +0x14

    bdHashMap() : m_size(0), m_capacity(0), m_loadFactor(0.75f),
                  m_threshold(0.0f), m_map(NULL), m_numIterators(0) {}
    bdHashMap(unsigned int initialCapacity, float loadFactor) : m_numIterators(0) {
        createMap(initialCapacity, loadFactor);
    }
    ~bdHashMap() {
        clear();
        bdMemory::deallocate(m_map);
        m_map = NULL;
    }

    void createMap(unsigned int capacity, float loadFactor) {
        if (loadFactor <= 0.0f || loadFactor > 1.0f) {
            bdMessageProxy proxy("..\\bdCore/bdContainers/bdHashMap.inl",
                                 "void __thiscall bdHashMap::createMap(const unsigned int,const float)",
                                 0x1DEu, "dw/warn/");
            proxy.log("hashmap", "Illegal loadFactor. Using default value.");
            m_loadFactor = 0.75f;
        }
        m_size = 0;
        m_capacity = bdBitOperations_nextPowerOf2(capacity);
        m_loadFactor = loadFactor;
        m_threshold = (float)m_capacity * loadFactor;
        m_map = (Node**)bdMemory::allocate(4 * m_capacity);
        memset(m_map, 0, 4 * m_capacity);
    }

    unsigned int getHashIndex(unsigned int hash) const {
        return hash & (m_capacity - 1);
    }

    bool get(const TKey& key, TValue& value) const {
        if (m_size == 0)
            return false;
        unsigned int hash = THashingClass::hash(key);
        Node* node = m_map[hash & (m_capacity - 1)];
        while (node != NULL && THashingClass::hash(node->m_key) != hash)
            node = node->m_next;
        if (node == NULL)
            return false;
        if (value.m_ptr != node->m_value.m_ptr) {
            if (value.m_ptr != NULL && value.m_ptr->releaseRef() == 0)
                delete value.m_ptr;
            value.m_ptr = node->m_value.m_ptr;
            if (value.m_ptr != NULL)
                value.m_ptr->addRef();
        }
        return true;
    }

    bool put(const TKey& key, const TValue& value) {
        unsigned int hash = THashingClass::hash(key);
        unsigned int index = hash & (m_capacity - 1);
        Node* node = m_map[index];
        while (node != NULL && THashingClass::hash(node->m_key) != hash)
            node = node->m_next;
        if (node != NULL)
            return false;
        if (m_size + 1 > (unsigned int)m_threshold) {
            resize(2 * m_capacity);
            index = hash & (m_capacity - 1);
        }
        ++m_size;
        Node* newNode = (Node*)bdMemory::allocate(sizeof(Node));
        if (newNode != NULL) {
            newNode->m_value.m_ptr = value.m_ptr;
            if (value.m_ptr != NULL)
                value.m_ptr->addRef();
            newNode->m_key.m_handle.m_ptr = key.m_handle.m_ptr;
            if (key.m_handle.m_ptr != NULL)
                key.m_handle.m_ptr->addRef();
            newNode->m_next = m_map[index];
            m_map[index] = newNode;
        }
        return true;
    }

    bool remove(const TKey& key) {
        unsigned int hash = THashingClass::hash(key);
        unsigned int index = hash & (m_capacity - 1);
        Node* node = m_map[index];
        Node** link = &m_map[index];
        while (node != NULL && THashingClass::hash(node->m_key) != hash) {
            link = &node->m_next;
            node = node->m_next;
        }
        if (node == NULL)
            return false;
        *link = node->m_next;
        if (node->m_value.m_ptr != NULL && node->m_value.m_ptr->releaseRef() == 0)
            delete node->m_value.m_ptr;
        if (node->m_key.m_handle.m_ptr != NULL && node->m_key.m_handle.m_ptr->releaseRef() == 0)
            delete node->m_key.m_handle.m_ptr;
        bdMemory::deallocate(node);
        --m_size;
        return true;
    }

    void clear() {
        for (unsigned int i = 0; i < m_capacity; i++) {
            Node* node = m_map[i];
            while (node != NULL) {
                Node* next = node->m_next;
                if (node->m_value.m_ptr != NULL && node->m_value.m_ptr->releaseRef() == 0)
                    delete node->m_value.m_ptr;
                if (node->m_key.m_handle.m_ptr != NULL && node->m_key.m_handle.m_ptr->releaseRef() == 0)
                    delete node->m_key.m_handle.m_ptr;
                bdMemory::deallocate(node);
                node = next;
            }
            m_map[i] = NULL;
        }
        m_size = 0;
    }

    void resize(unsigned int newCapacity) {
        unsigned int oldCapacity = m_capacity;
        Node** oldMap = m_map;
        unsigned int powerOf2 = bdBitOperations_nextPowerOf2(newCapacity);
        if (powerOf2 <= m_capacity)
            return;
        m_capacity = powerOf2;
        m_threshold = (float)m_capacity * m_loadFactor;
        m_map = (Node**)bdMemory::allocate(4 * m_capacity);
        m_size = 0;
        memset(m_map, 0, 4 * m_capacity);
        for (unsigned int i = 0; i < oldCapacity; i++) {
            Node* node = oldMap[i];
            while (node != NULL) {
                Node* next = node->m_next;
                unsigned int index = THashingClass::hash(node->m_key) & (m_capacity - 1);
                node->m_next = m_map[index];
                m_map[index] = node;
                ++m_size;
                node = next;
            }
        }
        bdMemory::deallocate(oldMap);
    }

    unsigned int getSize() const { return m_size; }
    Node* getIterator() const {
        for (unsigned int i = 0; i < m_capacity; i++) {
            if (m_map[i] != NULL)
                return m_map[i];
        }
        return NULL;
    }
    void next(Node*& it) const {
        if (it == NULL)
            return;
        if (it->m_next != NULL) {
            it = it->m_next;
            return;
        }
        unsigned int index = getHashIndex(THashingClass::hash(it->m_key)) + 1;
        while (index < m_capacity && m_map[index] == NULL)
            index++;
        it = (index < m_capacity) ? m_map[index] : NULL;
    }
    TValue& getValue(Node* it) { return it->m_value; }
    const TValue& getValue(Node* it) const { return it->m_value; }
};
static_assert(sizeof(bdHashMap<bdAddrHandleHashmapWrapper, bdReference<bdConnection>, bdAddrHandleHashmapWrapper>) == 0x18,
              "bdHashMap size mismatch");


#endif // COD3_BD_BDHASHMAP_H
