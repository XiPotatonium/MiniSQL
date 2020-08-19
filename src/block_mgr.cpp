﻿#include "../inc/block_mgr.hpp"

BlockGuard::BlockGuard(BlockManager *mgr, const BlockEntry &entry) : entry(entry), mgr(mgr) {
    m_addr = this->mgr->use_block(this->entry);
}

BlockGuard::BlockGuard(BlockManager *mgr, const string &file_path, int block_index)
    : entry(BlockEntry(file_path, block_index)), mgr(mgr) {
    m_addr = this->mgr->use_block(this->entry);
}

BlockGuard::BlockGuard(BlockGuard &&guard) noexcept : entry(guard.entry), mgr(guard.mgr) {
    m_addr = guard.m_addr;
    guard.m_addr = nullptr;
}

void BlockGuard::set_modified() { mgr->set_block_modified(entry); }

BlockGuard::~BlockGuard() {
    if (m_addr) {
        mgr->return_block(this->entry);
    }
}

BlockEntry::BlockEntry(string file_path, int block_index) {
    this->file_path = file_path;
    this->block_index = block_index;
}

template <>
class LRUEvictor<BlockEntry> {
    BlockManager *mgr;

public:
    LRUEvictor(BlockManager *mgr) : mgr(mgr){};
    bool try_evict(BlockEntry tag) {
        auto &info = mgr->active_blocks.at(tag);
        bool succ = info.use_count == 0;
        if (succ) {
            if (info.modified)
                mgr->block_writeback(tag, info.addr);
            delete info.addr;
            mgr->active_blocks.erase(tag);
        }
        return succ;
    }
};

/// <summary>
/// 创建relation的时候调用这个会自动创建一个文件
/// </summary>
/// <param name="path">允许是不存在的path，会自动创建</param>
/// <returns></returns>
FILE *BlockManager::use_file(const string &path) {
    auto active = active_files.find(path);
    if (active == active_files.end()) {
        FILE *fp = fopen(path.c_str(), "rb+");
        if (!fp) {
            fp = fopen(path.c_str(), "wb+");
        }
        active_files.emplace(path, fp);
        // active_files.insert(pair<string, FILE*>(path, fp));
        return fp;
    } else {
        return active->second;
    }
}

void BlockManager::block_writeback(const BlockEntry &entry, void *data) {
    FILE *fp = use_file(entry.file_path.c_str());
    if (entry.block_index >= file_blocks(entry.file_path))
        throw logic_error("Unexpected writeback error. The file may be deleted.");
    fseek(fp, entry.block_index * BLOCK_SIZE, SEEK_SET);
    fwrite(data, BLOCK_SIZE, 1, fp);
}

BlockManager::BlockManager(int max_blocks) {
    auto evictor = new LRUEvictor<BlockEntry>(this);
    lru = new LRU<BlockEntry>(max_blocks, evictor);
}

BlockManager::~BlockManager() {
    flush();
    for (auto &p : active_blocks) {
        delete p.second.addr;
    }
    for (auto &p : active_files) {
        fclose(p.second);
    }
    delete lru;
}

void BlockManager::flush() {
    for (auto &p : active_blocks) {
        if (p.second.modified) {
            block_writeback(p.first, p.second.addr);
        }
        p.second.modified = false;
    }
    for (auto &p : active_files) {
        fflush(p.second);
    }
}

int BlockManager::file_blocks(const string &file_path) {
    FILE *fp = use_file(file_path.c_str());
    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);
    return size / BLOCK_SIZE;
}

int BlockManager::file_append_block(const string &file_path) {
    FILE *fp = use_file(file_path.c_str());
    fseek(fp, 0, SEEK_END);
    int len = ftell(fp);
    int index = ftell(fp) / BLOCK_SIZE;
    static uint8_t data[BLOCK_SIZE] = {0};
    fwrite(&data, sizeof(data), 1, fp);
    return index;
}

void BlockManager::file_delete(const string &file_path) {
    auto active = active_files.find(file_path);
    if (active != active_files.end()) {
        // clean blocks
        vector<int> block_used;
        for (auto &p : active_blocks) {
            if (p.first.file_path == file_path) {
                block_used.push_back(p.first.block_index);
                if (p.second.use_count > 0)
                    throw logic_error("Unexpected error. The file is still being used.");
            }
        }
        for (int block_index : block_used) {
            BlockEntry entry(file_path, block_index);
            active_blocks.erase(entry);
        }

        fclose(active->second);
        remove(file_path.c_str());
        active_files.erase(file_path);
    }
}

void *BlockManager::use_block(const BlockEntry &entry) {
    auto active = active_blocks.find(entry);
    if (active == active_blocks.end()) {
        // miss 从硬盘读取
        FILE *fp = use_file(entry.file_path.c_str());
        fseek(fp, 0, SEEK_END);

        int size = ftell(fp);
        int size_expect = (entry.block_index + 1) * BLOCK_SIZE;
        if (size < size_expect) {
            // 错误的block数目
            // QUESTION 什么情况下会发生
            throw logic_error("File is not long enough. Check block count first.");
        } else {
            fseek(fp, entry.block_index * BLOCK_SIZE, SEEK_SET);
            void *addr = new uint8_t[BLOCK_SIZE];
            fread(addr, BLOCK_SIZE, 1, fp);     // QUESTION 有c++API吗？

            ActiveBlockInfo info;
            info.use_count = 1;
            info.addr = addr;
            info.handle = lru->add(entry);
            active_blocks.emplace(entry, info);
            return addr;
        }
    } else {
        // hit
        auto &info = active->second;
        info.use_count++;
        lru->use(info.handle);
        return info.addr;
    }
}

void BlockManager::return_block(const BlockEntry &entry) {
    auto &info = active_blocks.at(entry);
    info.use_count--;
}

void BlockManager::set_block_modified(const BlockEntry &entry) {
    auto &info = active_blocks.at(entry);
    info.modified = true;
}
