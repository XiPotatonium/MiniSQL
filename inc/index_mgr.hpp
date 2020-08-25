#pragma once

#include "common.hpp"
#include "index.hpp"
#include "scanner.hpp"
#include <algorithm>
#include <vector>
#include <optional>

struct tree_information {
    int root;
    int degree;
};
struct treenode {
    bool Is_root;
    bool Is_leaf;
    int size;
    int fa;
    int next;
    int before;
};
class IndexManager {

public:
    IndexManager(BlockManager *block_mgr);
    // TODO: ��һ���������������ڴ����������ݣ���˴���һ��scanner����ȡ�������ݡ�
    void add_index(const Relation &rel, int field_index, unique_ptr<Scanner> scanner);
    // TODO: ��ȡһ���ϵ��������������������������������һ�еĴ���λ��
    IndexIterator get_index(const Relation &rel, IndexUsage index_usage) {
        vector<RecordPosition> vr;
        const string &scheme = Files::index(rel.name, index_usage.field_index);
        Type type = rel.fields[index_usage.field_index].type;
        optional<Value> bj = index_usage.from;
        optional<Value> end_bj = index_usage.to;
        BlockGuard tree_bg(block_mgr, scheme, 0);
        tree_information tot_info = *tree_bg.addr<tree_information>(0);
        int num = tot_info.root;
        int degree = tot_info.degree;
        while (num > 0) {
            BlockGuard bg(block_mgr, scheme, num);
            treenode *tree_info = bg.addr<treenode>();
            RecordPosition temp_record;
            if (tree_info->Is_leaf == true) {
                int i;
                int flag = 0;
                int size = tree_info->size;
                Value temp_value;
                if (!bj)
                    i = 0;
                else
                    for (i = 0; i < size; i++) {
                        temp_value = get_value(bg, scheme, type, num, degree, i);
                        if (Value::cmp(type, bj.value(), temp_value) <= 0)
                            break;
                    }
                while (num != 0) {
                    BlockGuard value_bg(block_mgr, scheme, num);
                    treenode *value_info = value_bg.addr<treenode>();
                    while (i < value_info->size) {
                        temp_record = get_record(scheme, type, num, degree, i);
                        temp_value = get_value(value_bg, scheme, type, num, degree, i);
                        if (!end_bj || Value::cmp(type, end_bj.value(), temp_value) >= 0) {
                            vr.push_back(temp_record);
                            i++;
                        } else {
                            flag = 1;
                            break;
                        }
                    }
                    if (flag == 1)
                        break;
                    if (value_info->next != 0) {
                        num = value_info->next;
                        i = 0;
                    } else
                        break;
                }
                break;
            } else {
                int i;
                int size = tree_info->size;
                Value temp_value;
                BlockGuard value_bg(block_mgr, scheme, num);
                if (!bj)
                    i = 0;
                else
                    for (i = 0; i < size; i++) {
                        temp_value = get_value(value_bg, scheme, type, num, degree, i);
                        if (Value::cmp(type, temp_value, bj.value()) >= 0)
                            break;
                    }
                int temp_block = get_block(scheme, type, num, degree, i);
                num = temp_block;
            }
        }
        sort(vr.begin(), vr.end());
        return IndexIterator(vr);
    };
    // TODO: ��һ��ɾ��һ��ֵ
    void remove_item(const Relation &rel, int field_index, const Value &value);
    // TODO: ��һ������һ��ֵ
    void add_item(const Relation &rel, int field_index, const Value &value, RecordPosition record_pos);
    void remove_index(const Relation &rel, int field_index) {
        const string &scheme = Files::index(rel.name, field_index);
        remove(scheme.c_str());
    }
    optional<RecordPosition> find(const Relation &rel, int field_index, Value value);

private:
    BlockManager *block_mgr;
    tree_information IndexManager::get_information(const Relation &rel, int field_index, const string &scheme);
    void insert_value(const string &scheme, Type type, const Value &value, int degree, int num, int pos);
    void insert_record(const string &scheme, Type type, RecordPosition record_pos, int degree, int num, int pos);
    void insert_block(const string &scheme, Type type, int block_index, int degree, int num, int pos);
    Value find_min(const string &scheme, Type type, int num, int degree);
    void IndexManager::change_fa(const string &scheme, Type type, int num, int degree, int new_fa);
    RecordPosition get_record(const string &scheme, Type type, int num, int degree, int pos);
    int get_block(const string &scheme, Type type, int num, int degree, int pos);
    Value IndexManager::get_value(BlockGuard &bg, const string &scheme, Type type, int num, int degree, int pos);
    void shift_value(const string &scheme, Type type, int degree, int num, int pos, int direction);
    void shift_block(const string &scheme, Type type, int degree, int num, int pos, int direction);
    void shift_record(const string &scheme, Type type, int degree, int num, int pos, int direction);
    void merge_leaf(string &scheme, Type type, int left_leaf, int right_leaf, int degree);
    void merge_nonleaf(string &scheme, Type type, int left_nonleaf, int right_nonleaf, int degree);
    optional<RecordPosition> find_record(const string &scheme, Type type, int degree, Value value, int num);
    void delete_parent_information(string &scheme, Type type, int num, Value value, int degree);
    int get_new_block(const string &scheme);

    void work(const string &scheme, const Relation &rel, int field_index, const Value &value, RecordPosition record_pos,
              int num, int degree);

    void fresh_information(string &scheme, Type type, int num, int degree, Value old_value, Value new_value);

    void remove_single_item(string &scheme, Type type, int degree, Value value, int num);
};
