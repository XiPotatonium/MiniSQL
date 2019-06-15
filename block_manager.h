#pragma once

#include "lru.h"
#include <stdexcept>
#include <stdio.h>
#include <string>
#include <unordered_map>
#include <utility>
#include <stdint.h>

using namespace std;

// ���С
const int BLOCK_SIZE = 4096;

// ����Ψһȷ��һ���飬���ļ�·�����ļ��ڿ�Ź���
struct BlockEntry {
    string file_path;
    int block_index = -1;
    BlockEntry() = default;
    BlockEntry(string file_path, int block_index);
};

namespace std {
    template<>
    struct hash<BlockEntry> {
        size_t operator()(const BlockEntry& entry) const {
            size_t h1 = hash<string>()(entry.file_path);
            size_t h2 = hash<int>()(entry.block_index);
            return  h1 ^ (h2 << 1);
        }
    };

    template<>
    struct equal_to<BlockEntry> {
        bool operator()(const BlockEntry& lhs, const BlockEntry& rhs) const
        {
            return lhs.block_index == rhs.block_index && lhs.file_path == rhs.file_path;
        }
    };
}

struct ActiveBlockInfo {
    int use_count;
    void* addr;
    LRUNodeHandle<BlockEntry> handle;
    bool modified = false;
};

class BlockManager {
    friend class LRUEvictor<BlockEntry>;
private:
    LRU<BlockEntry>* lru;
    unordered_map<BlockEntry, ActiveBlockInfo> active_blocks;
    unordered_map<string, FILE*> active_files;
    FILE* use_file(const string& path);
    void block_writeback(const BlockEntry& entry, void* data);
public:
    BlockManager(int max_blocks = 1024);
    BlockManager(const BlockManager&) = delete;

    // ���١�д�����иĶ���Ӳ��
    ~BlockManager();

    // �����иĶ�����д��Ӳ��
    void flush();

    // ��ȡһ���ļ��ڵĿ�����
    int file_blocks(const string& file_path);

    // ���ļ�ĩβ׷��һ���飬������Ϊȫ0
    int file_append_block(const string& file_path);

    void file_delete(const string& file_path);

    // ��ȡһ����ķ���Ȩ�����ؿ��ڴ��ַ����СΪBLOCK_SIZE
    void* use_block(const BlockEntry& entry);

    // ����һ����ķ���Ȩ
    void return_block(const BlockEntry& entry);

    // ��Ǹÿ��ѱ��޸ģ�����֮��д���ļ�
    void set_block_modified(const BlockEntry& entry);
};

// ����RAII�򻯿�Ļ�ȡ�뷵������BlockGuard����ʱ��ȡ�����Ȩ������ʱ��������Ȩ��
class BlockGuard {
private:
    BlockManager* mgr;
    BlockEntry entry;
    void* m_addr = nullptr;
public:
    BlockGuard(BlockManager* mgr, const BlockEntry& entry);
    BlockGuard(BlockManager* mgr, const string& file_path, int block_index);
    BlockGuard(const BlockGuard& guard) = delete;
    BlockGuard(BlockGuard&& guard) noexcept;
    ~BlockGuard();

    // ��ȡ���ַ��T�������õ�ַ����
    template<typename T> T* addr() { return (T*)m_addr; }

    // ��ȡ���ַ��T�������õ�ַ���ͣ�offsetΪ��ַƫ����
    template<typename T> T* addr(int offset) { return (T*)((uint8_t*)m_addr + offset); }

    // ��ȡ���ַ
    void* addr() { return m_addr; }

    // ���øÿ��ѱ��޸�
    void set_modified();
};