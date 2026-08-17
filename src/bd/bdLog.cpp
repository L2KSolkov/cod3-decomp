#include "bd/bd_types.h"

#include <new>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

struct bdLogChannelMapNode;

class bdLogChannelMap {
public:
    unsigned int m_size;
    unsigned int m_capacity;
    float m_loadFactor;
    float m_threshold;
    bdLogChannelMapNode** m_map;
    unsigned int m_numIterators;

    bdLogChannelMap()
        : m_size(0), m_capacity(0), m_loadFactor(0.75f),
          m_threshold(0.0f), m_map(NULL), m_numIterators(0) {}

    void createMap(unsigned int capacity, float loadFactor);
    void destroy();
    bdLogChannel* find(const bdString& key) const;
    bool put(const bdString& key, bdLogChannel* value);
    bool remove(const bdString& key);

private:
    unsigned int hash(const bdString& key) const;
    void resize(unsigned int capacity);
};

struct bdLogChannelMapNode {
    bdLogChannel* m_value;
    bdString m_key;
    bdLogChannelMapNode* m_next;
};

static_assert(sizeof(bdLogChannelMap) == 0x18, "bdLogChannel map size mismatch");
static_assert(sizeof(bdLogChannelMapNode) == 0x0C, "bdLogChannel map node size mismatch");

class bdLogChannel {
public:
    bdString m_name;
    bdLogChannelMap m_children;
    bdLogSubscriber** m_subscribers;
    unsigned int m_subscriberCapacity;
    unsigned int m_subscriberSize;

    explicit bdLogChannel(const char* name)
        : m_name(name), m_subscribers(NULL), m_subscriberCapacity(0),
          m_subscriberSize(0) {
        m_children.createMap(4, 0.75f);
    }

    ~bdLogChannel();

    void log(const char* file, const char* function, unsigned int line,
             const char* channel, const char* message);
    void addSubscriber(const char* root, const char* channel,
                       bdLogSubscriber* subscriber);
    void removeSubscriber(const char* root, const char* channel,
                          bdLogSubscriber* subscriber);
    void removeSubscriber(bdLogSubscriber* subscriber);

private:
    void increaseCapacity(unsigned int requested);
    void removeAll(bdLogSubscriber* subscriber);
};

static_assert(sizeof(bdLogChannel) == 0x28, "bdLogChannel size mismatch");

unsigned int bdLogChannelMap::hash(const bdString& key) const {
    const char* buffer = key.getBuffer();
    const unsigned int length = key.getLength();
    unsigned int value = 0;
    for (unsigned int i = 0; i < length; ++i)
        value = (unsigned int)((int)(signed char)buffer[i]) + 31 * value;
    return value;
}

void bdLogChannelMap::createMap(unsigned int capacity, float loadFactor) {
    m_size = 0;
    m_capacity = capacity;
    m_loadFactor = loadFactor;
    m_threshold = (float)capacity * loadFactor;
    m_map = (bdLogChannelMapNode**)bdMemory::allocate(4 * capacity);
    if (m_map != NULL)
        memset(m_map, 0, 4 * capacity);
}

void bdLogChannelMap::destroy() {
    if (m_map != NULL) {
        for (unsigned int i = 0; i < m_capacity; ++i) {
            bdLogChannelMapNode* node = m_map[i];
            while (node != NULL) {
                bdLogChannelMapNode* next = node->m_next;
                node->m_key.~bdString();
                bdMemory::deallocate(node);
                node = next;
            }
            m_map[i] = NULL;
        }
    }
    m_size = 0;
    bdMemory::deallocate(m_map);
    m_map = NULL;
    m_capacity = 0;
    m_threshold = 0.0f;
}

bdLogChannel* bdLogChannelMap::find(const bdString& key) const {
    if (m_map == NULL || m_capacity == 0)
        return NULL;
    const unsigned int keyHash = hash(key);
    bdLogChannelMapNode* node = m_map[keyHash & (m_capacity - 1)];
    while (node != NULL && hash(node->m_key) != keyHash)
        node = node->m_next;
    return node != NULL ? node->m_value : NULL;
}

void bdLogChannelMap::resize(unsigned int capacity) {
    if (capacity <= m_capacity)
        return;
    bdLogChannelMapNode** oldMap = m_map;
    const unsigned int oldCapacity = m_capacity;
    m_map = (bdLogChannelMapNode**)bdMemory::allocate(4 * capacity);
    if (m_map == NULL)
        return;
    memset(m_map, 0, 4 * capacity);
    m_capacity = capacity;
    m_threshold = capacity * m_loadFactor;
    for (unsigned int i = 0; i < oldCapacity; ++i) {
        bdLogChannelMapNode* node = oldMap[i];
        while (node != NULL) {
            bdLogChannelMapNode* next = node->m_next;
            const unsigned int index = hash(node->m_key) & (m_capacity - 1);
            node->m_next = m_map[index];
            m_map[index] = node;
            node = next;
        }
    }
    bdMemory::deallocate(oldMap);
}

bool bdLogChannelMap::put(const bdString& key, bdLogChannel* value) {
    if (m_size + 1 > (unsigned int)m_threshold)
        resize(2 * m_capacity);
    const unsigned int keyHash = hash(key);
    const unsigned int index = keyHash & (m_capacity - 1);
    bdLogChannelMapNode* node = m_map[index];
    while (node != NULL && hash(node->m_key) != keyHash)
        node = node->m_next;
    if (node != NULL)
        return false;
    node = (bdLogChannelMapNode*)bdMemory::allocate(sizeof(bdLogChannelMapNode));
    if (node == NULL)
        return false;
    new (&node->m_key) bdString(key);
    node->m_value = value;
    node->m_next = m_map[index];
    m_map[index] = node;
    ++m_size;
    return true;
}

bool bdLogChannelMap::remove(const bdString& key) {
    if (m_map == NULL || m_capacity == 0)
        return false;
    const unsigned int keyHash = hash(key);
    const unsigned int index = keyHash & (m_capacity - 1);
    bdLogChannelMapNode** link = &m_map[index];
    bdLogChannelMapNode* node = *link;
    while (node != NULL && hash(node->m_key) != keyHash) {
        link = &node->m_next;
        node = node->m_next;
    }
    if (node == NULL)
        return false;
    *link = node->m_next;
    node->m_key.~bdString();
    bdMemory::deallocate(node);
    --m_size;
    return true;
}

void bdLogChannel::increaseCapacity(unsigned int requested) {
    unsigned int value = requested;
    const unsigned int capacity = m_subscriberCapacity;
    if (requested <= capacity)
        value = capacity;
    const unsigned int newCapacity = value + capacity;
    bdLogSubscriber** data = (bdLogSubscriber**)bdMemory::allocate(4 * newCapacity);
    if (m_subscriberSize != 0)
        memcpy(data, m_subscribers, 4 * m_subscriberSize);
    bdMemory::deallocate(m_subscribers);
    m_subscriberCapacity = newCapacity;
    m_subscribers = data;
}

void bdLogChannel::removeAll(bdLogSubscriber* subscriber) {
    for (unsigned int i = 0; i < m_subscriberSize; ++i) {
        if (m_subscribers[i] == subscriber) {
            m_subscribers[i] = m_subscribers[m_subscriberSize - 1];
            --m_subscriberSize;
            if (m_subscriberCapacity > 4 * m_subscriberSize) {
                m_subscriberCapacity -= m_subscriberCapacity >> 1;
                m_subscribers = (bdLogSubscriber**)bdMemory::reallocate(
                    m_subscribers, 4 * m_subscriberCapacity);
            }
            --i;
        }
    }
}

bdLogChannel::~bdLogChannel() {
    for (unsigned int i = 0; i < m_children.m_capacity; ++i) {
        bdLogChannelMapNode* node = m_children.m_map != NULL ? m_children.m_map[i] : NULL;
        while (node != NULL) {
            bdLogChannelMapNode* next = node->m_next;
            if (node->m_value != NULL) {
                node->m_value->~bdLogChannel();
                bdMemory::deallocate(node->m_value);
            }
            node = next;
        }
    }
    bdMemory::deallocate(m_subscribers);
    m_subscribers = NULL;
    m_subscriberCapacity = 0;
    m_subscriberSize = 0;
    m_children.destroy();
}

void bdLogChannel::log(const char* file, const char* function,
                       unsigned int line, const char* channel,
                       const char* message) {
    for (unsigned int i = 0; i < m_subscriberSize; ++i)
        m_subscribers[i]->publish(m_name.getBuffer(), file, function, line, message);

    const char* slash = strchr(channel, '/');
    const unsigned int length = slash != NULL ? (unsigned int)(slash - channel)
                                               : (unsigned int)strlen(channel);
    if (length != 0) {
        char segment[100];
        memcpy(segment, channel, length);
        segment[length] = 0;
        bdString key(segment);
        bdLogChannel* child = m_children.find(key);
        if (child != NULL)
            child->log(file, function, line, channel + length + (slash != NULL), message);
    }
}

void bdLogChannel::addSubscriber(const char* root, const char* channel,
                                 bdLogSubscriber* subscriber) {
    const char* slash = strchr(channel, '/');
    const unsigned int length = slash != NULL ? (unsigned int)(slash - channel)
                                               : (unsigned int)strlen(channel);
    if (length != 0) {
        char segment[100];
        memcpy(segment, channel, length);
        segment[length] = 0;
        bdString key(segment);
        bdLogChannel* child = m_children.find(key);
        if (child == NULL) {
            void* memory = bdMemory::allocate(0x28);
            child = memory != NULL ? new (memory) bdLogChannel(root) : NULL;
            m_children.put(key, child);
        }
        child->addSubscriber(root, channel + length + (slash != NULL), subscriber);
    } else {
        if (m_subscriberSize == m_subscriberCapacity)
            increaseCapacity(1);
        m_subscribers[m_subscriberSize++] = subscriber;
    }
}

void bdLogChannel::removeSubscriber(const char* root, const char* channel,
                                    bdLogSubscriber* subscriber) {
    const char* slash = strchr(channel, '/');
    const unsigned int length = slash != NULL ? (unsigned int)(slash - channel)
                                               : (unsigned int)strlen(channel);
    if (length != 0) {
        char segment[100];
        memcpy(segment, channel, length);
        segment[length] = 0;
        bdString key(segment);
        bdLogChannel* child = m_children.find(key);
        if (child != NULL) {
            child->removeSubscriber(root, channel + length + (slash != NULL), subscriber);
            if (child->m_subscriberSize == 0) {
                m_children.remove(key);
                child->~bdLogChannel();
                bdMemory::deallocate(child);
            }
        }
    } else {
        removeAll(subscriber);
    }
}

void bdLogChannel::removeSubscriber(bdLogSubscriber* subscriber) {
    removeAll(subscriber);
    for (unsigned int i = 0; i < m_children.m_capacity; ++i) {
        bdLogChannelMapNode* node = m_children.m_map != NULL ? m_children.m_map[i] : NULL;
        while (node != NULL) {
            bdLogChannelMapNode* next = node->m_next;
            bdLogChannel* child = node->m_value;
            child->removeSubscriber(subscriber);
            if (child->m_subscriberSize == 0) {
                bdString key(node->m_key);
                m_children.remove(key);
                child->~bdLogChannel();
                bdMemory::deallocate(child);
            }
            node = next;
        }
    }
}

class bdSingletonRegistryImpl {
public:
    typedef void (__cdecl *DestroyFunction)();

    bdSingletonRegistryImpl()
        : m_destroyFunctions(NULL), m_destroyCapacity(0), m_destroySize(0),
          m_cleaningUp(false) {}

    virtual ~bdSingletonRegistryImpl() {
        bdMemory::deallocate(m_destroyFunctions);
        m_destroyFunctions = NULL;
        m_destroyCapacity = 0;
        m_destroySize = 0;
    }

    bool add(DestroyFunction function) {
        if (m_cleaningUp)
            return false;
        if (m_destroySize == m_destroyCapacity)
            increaseCapacity(1);
        m_destroyFunctions[m_destroySize++] = function;
        return true;
    }

    void cleanUp() {
        m_cleaningUp = true;
        while (m_destroySize != 0)
            m_destroyFunctions[--m_destroySize]();
    }

private:
    void increaseCapacity(unsigned int requested) {
        unsigned int value = requested;
        if (requested <= m_destroyCapacity)
            value = m_destroyCapacity;
        const unsigned int newCapacity = value + m_destroyCapacity;
        DestroyFunction* data = (DestroyFunction*)bdMemory::allocate(4 * newCapacity);
        if (m_destroySize != 0)
            memcpy(data, m_destroyFunctions, 4 * m_destroySize);
        bdMemory::deallocate(m_destroyFunctions);
        m_destroyFunctions = data;
        m_destroyCapacity = newCapacity;
    }

    DestroyFunction* m_destroyFunctions;
    unsigned int m_destroyCapacity;
    unsigned int m_destroySize;
    bool m_cleaningUp;
};

static_assert(sizeof(bdSingletonRegistryImpl) == 0x14,
              "bdSingletonRegistryImpl size mismatch");

template <>
bdSingletonRegistryImpl* bdSingleton<bdSingletonRegistryImpl>::m_instance = NULL;

template <>
bdLogImpl* bdSingleton<bdLogImpl>::m_instance = NULL;

void destroySingletonRegistry() {
    bdSingletonRegistryImpl* instance =
        bdSingleton<bdSingletonRegistryImpl>::m_instance;
    if (instance != NULL) {
        instance->~bdSingletonRegistryImpl();
        bdMemory::deallocate(instance);
        bdSingleton<bdSingletonRegistryImpl>::m_instance = NULL;
    }
}

bdSingletonRegistryImpl* getSingletonRegistry() {
    bdSingletonRegistryImpl* instance =
        bdSingleton<bdSingletonRegistryImpl>::m_instance;
    if (instance == NULL) {
        void* memory = bdMemory::allocate(0x14);
        instance = memory != NULL ? new (memory) bdSingletonRegistryImpl() : NULL;
        bdSingleton<bdSingletonRegistryImpl>::m_instance = instance;
        if (instance != NULL && !instance->add(&destroySingletonRegistry)) {
            instance->~bdSingletonRegistryImpl();
            bdMemory::deallocate(instance);
            bdSingleton<bdSingletonRegistryImpl>::m_instance = NULL;
            return NULL;
        }
    }
    return instance;
}

void destroyLogSingleton() {
    bdLogImpl* instance = bdSingleton<bdLogImpl>::m_instance;
    if (instance != NULL) {
        instance->~bdLogImpl();
        bdMemory::deallocate(instance);
        bdSingleton<bdLogImpl>::m_instance = NULL;
    }
}

static unsigned int bdStrlcpy(char* destination, const char* source,
                              unsigned int count) {
    const unsigned int result = (unsigned int)strlen(source);
    if (count != 0) {
        unsigned int length = count - 1;
        if (result < length)
            length = result;
        memcpy(destination, source, length);
        destination[length] = 0;
    }
    return result;
}

bdLogImpl::bdLogImpl() : m_root(NULL) {
}

bdLogImpl::~bdLogImpl() {
    if (m_root != NULL) {
        m_root->~bdLogChannel();
        bdMemory::deallocate(m_root);
        m_root = NULL;
    }
}

bdLogChannel* bdLogImpl::setRoot(const char* channel) {
    if (m_root != NULL) {
        m_root->~bdLogChannel();
        bdMemory::deallocate(m_root);
    }
    void* memory = bdMemory::allocate(0x28);
    m_root = memory != NULL ? new (memory) bdLogChannel(channel) : NULL;
    return m_root;
}

void bdLogImpl::log(const char* file, const char* function, unsigned int line,
                    const char* channel, const char* message) {
    if (m_root == NULL)
        setRoot("dw");
    m_root->log(file, function, line, channel + 3, message);
}

void bdLogImpl::subscribe(const char* channel, bdLogSubscriber* subscriber) {
    if (m_root == NULL)
        setRoot("dw");
    if (strstr(channel, "dw") != channel)
        m_root->addSubscriber(channel, channel, subscriber);
}

void bdLogImpl::unsubscribe(const char* channel, bdLogSubscriber* subscriber) {
    if (strstr(channel, "dw") != channel)
        m_root->removeSubscriber(channel, channel, subscriber);
}

void bdLogImpl::unsubscribeAll(bdLogSubscriber* subscriber) {
    m_root->removeSubscriber(subscriber);
}

template <>
bdLogImpl* bdSingleton<bdLogImpl>::getInstance() {
    bdLogImpl* instance = bdSingleton<bdLogImpl>::m_instance;
    if (instance == NULL) {
        void* memory = bdMemory::allocate(8);
        instance = memory != NULL ? new (memory) bdLogImpl() : NULL;
        bdSingleton<bdLogImpl>::m_instance = instance;
        if (instance != NULL) {
            bdSingletonRegistryImpl* registry = getSingletonRegistry();
            if (registry != NULL && registry->add(&destroyLogSingleton))
                return instance;
            instance->~bdLogImpl();
            bdMemory::deallocate(instance);
            bdSingleton<bdLogImpl>::m_instance = NULL;
        }
    }
    return bdSingleton<bdLogImpl>::m_instance;
}

void bdMessageProxy::log(const char* channel, const char* format, ...) const {
    char baseChannel[152];
    unsigned int length = (unsigned int)strlen(m_baseChannel);
    if (length >= 0x95)
        length = 149;
    memcpy(baseChannel, m_baseChannel, length);
    baseChannel[length] = 0;
    const unsigned int used = (unsigned int)strlen(baseChannel);
    if (used < 0x95)
        bdStrlcpy(&baseChannel[used], channel, 150 - used);

    char message[1028];
    va_list ap;
    va_start(ap, format);
    _vscprintf(format, ap);
    vsnprintf(message, 0x400, format, ap);
    va_end(ap);
    message[1023] = 0;

    bdLogImpl* instance = bdSingleton<bdLogImpl>::getInstance();
    if (instance->m_root == NULL)
        instance->setRoot("dw");
    instance->m_root->log(m_file, m_function, m_line, &baseChannel[3], message);
}
