#pragma once

#include "common.h"
#include "nullable.h"
#include "index.h"
#include "scanner.h"

class IndexManager {
public:
    IndexManager(BlockManager* block_mgr) {};

    // TODO: ��һ��������������ڴ����������ݣ���˴���һ��scanner����ȡ�������ݡ�
    void add_index(const Relation& rel, int field_index, unique_ptr<Scanner> scanner) {}
    
    // TODO: ��һ��ɾ������
    void remove_index(const Relation& rel, int field_index) {}

    // TODO: ��ȡһ���ϵ��������������������������������һ�еĴ���λ��
    IndexIterator get_index(const Relation& rel, IndexUsage index_usage) { return IndexIterator();  };
    
    // TODO: ��һ�����һ��ֵ
    void add_item(const Relation& rel, int field_index, const Value& value, RecordPosition record_pos) {}
    
    // TODO: ��һ��ɾ��һ��ֵ
    void remove_item(const Relation& rel, int field_index, const Value& value) {}
};

