#pragma once

template <typename T>
struct LRUNode {
    LRUNode *prev = nullptr, *next = nullptr;
    T tag;
    LRUNode(T tag) : tag(tag){};
};

/// <summary>
/// 仅在BlockManager那边使用过T=BlockEntry
/// </summary>
/// <typeparam name="T"></typeparam>
template <typename T>
class LRUEvictor {
public:
    bool try_evict(T tag) { return true; }
};

template <typename T, typename Ev = LRUEvictor<T>>
class LRU;

template <typename T>
struct LRUNodeHandle {
    friend class LRU<T>;

private:
    LRUNode<T> *node;
};

template <typename T, typename Ev>
class LRU {
    int max;
    int count = 0;
    Ev *evictor;
    LRUNode<T> *head, *tail; // dummy

    /// <summary>
    /// 队首添加
    /// </summary>
    /// <param name="node"></param>
    void list_add(LRUNode<T> *node) {
        LRUNode<T> *prev = head;
        LRUNode<T> *next = head->next;
        node->prev = prev;
        prev->next = node;
        node->next = next;
        next->prev = node;
    }

    void list_remove(LRUNode<T> *node) {
        LRUNode<T> *prev = node->prev;
        LRUNode<T> *next = node->next;
        prev->next = next;
        next->prev = prev;
    }

public:
    LRU(int max, LRUEvictor<T> *evictor) : max(max), evictor(evictor) {
        head = new LRUNode<T>(T());     // dummy节点
        tail = new LRUNode<T>(T());
        head->next = tail;
        tail->prev = head;
    }

    ~LRU() {
        LRUNode<T> *cur = head, *next = nullptr;
        while (cur) {
            next = cur->next;
            delete cur;
            cur = next;
        }
    }

    LRUNodeHandle<T> add(T tag) {
        LRUNode<T> *node = new LRUNode<T>(tag);
        list_add(node);
        LRUNodeHandle<T> handle;
        handle.node = node;
        count++;

        // LRU eviction
        // 队尾是LRU
        LRUNode<T> *cur = tail->prev;
        while (count > max && cur != head) {
            LRUNode<T> *prev = cur->prev;
            if (evictor->try_evict(tail->prev->tag)) {
                list_remove(cur);
                count--;
                delete cur;
            }
            cur = prev;
        }

        return handle;
    }

    /// <summary>
    /// 刷新，将node放到队首
    /// </summary>
    /// <param name="handle"></param>
    void use(LRUNodeHandle<T> handle) {
        list_remove(handle.node);
        list_add(handle.node);
    }
};