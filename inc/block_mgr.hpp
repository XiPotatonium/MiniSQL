#pragma once

#include "lru.hpp"
#include <stdexcept>
#include <stdint.h>
#include <stdio.h>
#include <string>
#include <unordered_map>
#include <utility>

using namespace std;

// 块大小
const int BLOCK_SIZE = 4096;

/// <summary>
/// 用于唯一确定一个块，由文件路径、文件内块号构成
/// </summary>
struct BlockEntry {
    string file_path;
    int block_index = -1;
    BlockEntry() = default;
    BlockEntry(string file_path, int block_index);
};

// TODO 另想办法，不要修改namespace std
template <>
struct hash<BlockEntry> {
    size_t operator()(const BlockEntry &entry) const {
        size_t h1 = hash<string>()(entry.file_path);
        size_t h2 = hash<int>()(entry.block_index);
        return h1 ^ (h2 << 1);
    }
};

// TODO 重载==
template <>
struct equal_to<BlockEntry> {
    bool operator()(const BlockEntry &lhs, const BlockEntry &rhs) const {
        return lhs.block_index == rhs.block_index && lhs.file_path == rhs.file_path;
    }
};

/// <summary>
/// Block在内存中的数据结构
/// </summary>
struct ActiveBlockInfo {
    /// <summary>
    /// 引用计数器
    /// </summary>
    int use_count;
    /// <summary>
    /// 块数据
    /// </summary>
    void *addr;
    LRUNodeHandle<BlockEntry> handle;
    /// <summary>
    /// 是否dirty
    /// </summary>
    bool modified = false;
};

class BlockManager {
    friend class LRUEvictor<BlockEntry>;

private:
    LRU<BlockEntry> *lru;
    unordered_map<BlockEntry, ActiveBlockInfo, hash<BlockEntry>, equal_to<BlockEntry>> active_blocks;
    unordered_map<string, FILE *> active_files;
    FILE *use_file(const string &path);
    void block_writeback(const BlockEntry &entry, void *data);

public:
    BlockManager(int max_blocks = 1024);
    BlockManager(const BlockManager &) = delete;

    /// <summary>
    /// 销毁。写回所有改动到硬盘
    /// </summary>
    ~BlockManager();

    /// <summary>
    /// 将所有改动立刻写回硬盘
    /// </summary>
    void flush();

    /// <summary>
    /// 获取一个文件内的块数量
    /// </summary>
    /// <param name="file_path"></param>
    /// <returns></returns>
    int file_blocks(const string &file_path);

    /// <summary>
    /// 在文件末尾追加一个块，其内容为全0
    /// </summary>
    /// <param name="file_path"></param>
    /// <returns></returns>
    int file_append_block(const string &file_path);

    void file_delete(const string &file_path);

    /// <summary>
    /// 获取一个块的访问权，返回块内存地址，大小为BLOCK_SIZE
    /// </summary>
    /// <param name="entry"></param>
    /// <returns></returns>
    void *use_block(const BlockEntry &entry);

    /// <summary>
    /// 返还一个块的访问权
    /// </summary>
    /// <param name="entry"></param>
    void return_block(const BlockEntry &entry);

    /// <summary>
    /// 标记该块已被修改，将在之后写回文件
    /// </summary>
    /// <param name="entry"></param>
    void set_block_modified(const BlockEntry &entry);
};

/// <summary>
/// 利用RAII简化块的获取与返还。在BlockGuard构造时获取块访问权，析构时返还访问权。
/// </summary>
class BlockGuard {
private:
    BlockManager *mgr;
    BlockEntry entry;
    void *m_addr = nullptr;

public:
    BlockGuard(BlockManager *mgr, const BlockEntry &entry);
    BlockGuard(BlockManager *mgr, const string &file_path, int block_index);
    BlockGuard(const BlockGuard &guard) = delete;
    BlockGuard(BlockGuard &&guard) noexcept;
    ~BlockGuard();

    /// <summary>
    /// 获取块地址，T参数设置地址类型
    /// </summary>
    /// <typeparam name="T"></typeparam>
    /// <returns></returns>
    template <typename T>
    T *addr() {
        return (T *)m_addr;
    }

    /// <summary>
    /// 获取块地址，T参数设置地址类型，offset为地址偏移量
    /// </summary>
    /// <typeparam name="T"></typeparam>
    /// <param name="offset"></param>
    /// <returns></returns>
    template <typename T>
    T *addr(int offset) {
        return (T *)((uint8_t *)m_addr + offset);
    }

    /// <summary>
    /// getter
    /// </summary>
    /// <returns></returns>
    void *addr() { return m_addr; }

    /// <summary>
    /// 设置该块已被修改
    /// </summary>
    void set_modified();
};