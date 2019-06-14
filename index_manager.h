#pragma once

#include "common.h"
#include "nullable.h"
#include "index.h"
#include "scanner.h"

class IndexManager {
public:
    IndexManager(BlockManager* block_mgr) {};
    // ��һ��������������ڴ����������ݣ���˴���һ��scanner����ȡ�������ݡ�
    void add_index(const Relation& rel, int field_index, unique_ptr<Scanner> scanner);
    void remove_index(const Relation& rel, int field_index);
    IndexIterator get_index(const Relation& rel, IndexUsage index_usage);
    void add_item(const Relation& rel, int field_index, const Value& value, RecordPosition record_pos);
    void remove_item(const Relation& rel, int field_index, const Value& value);
};

