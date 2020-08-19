#pragma once
#include "block_mgr.hpp"
#include "catalog_mgr.hpp"
#include "common.hpp"
#include "index_mgr.hpp"
#include "record_mgr.hpp"

#include <functional>

class StorageEngine {
    BlockManager bm;
    CatalogManager cm;
    RecordManager rm;
    IndexManager im;

    unique_ptr<IndexIterator> get_index_iterator(const Relation &rel, Nullable<IndexUsage> index_usage) {
        if (index_usage) {
            return unique_ptr<IndexIterator>(new IndexIterator(im.get_index(rel, index_usage.value())));
        } else {
            return nullptr;
        }
    }

public:
    StorageEngine(int max_blocks = 1024) : bm(max_blocks), cm(&bm), rm(&bm), im(&bm) {}

    /// <summary>
    /// 将dirty的block写回硬盘
    /// </summary>
    void flush() { bm.flush(); }

    /// <summary>
    /// 将一个新relation写到硬盘上
    /// </summary>
    /// <param name="relation"></param>
    void add_relation(const Relation &relation) {
        cm.add_relation(relation);
        rm.add_relation(relation);
        for (auto &field : relation.fields) {
            if (field.has_index) {
                add_index(relation.name, field.name, field.index_name);
            }
        }
    }

    void remove_relation(const string &name) {
        Relation rel = get_relation_exist(name);
        for (auto &field : rel.fields) {
            if (field.has_index) {
                remove_index(field.index_name);
            }
        }
        cm.remove_relation(name);
        rm.remove_relation(name);
    }

    Nullable<Relation> get_relation(const string &name) { return cm.get_relation(name); }

    Relation get_relation_exist(const string &name) {
        Nullable<Relation> nrel = get_relation(name);
        if (nrel.null())
            throw logic_error("Relation not found");
        return nrel.value();
    }

    void add_index(const string &rel_name, const string &field_name, const string &index_name) {
        cm.add_index(rel_name, field_name, index_name);
        Relation rel = get_relation_exist(rel_name);
        IndexLocation il = cm.get_index_location(index_name).value();
        im.add_index(rel, il.field, rm.select_record(rel, nullptr));
    }

    void remove_index(const string &index_name) {
        Nullable<IndexLocation> il = cm.get_index_location(index_name);
        if (il.null())
            throw logic_error("Index not found");

        Relation rel = get_relation_exist(il->relation_name);

        cm.remove_index(index_name);
        im.remove_index(rel, il->field);
    }

    void insert_record(const string &rel_name, Record &&record) {
        Relation rel = get_relation_exist(rel_name);
        RecordPosition pos = rm.insert_record(rel, record);

        for (int field_index : rel.indexes) {
            im.add_item(rel, field_index, record.values[field_index], pos);
        }
    }

    void update_record(const string &rel_name, Record &&record,
                       function<Nullable<Value>(const Record &record, int field_index)> new_value) {
        if (record.physical_position.nil())
            throw logic_error("Unexpected error. This record does not has a physical position.");
        Relation rel = get_relation_exist(rel_name);

        Record record_origin = record;
        for (int i = 0; i < rel.fields.size(); ++i) {
            Nullable<Value> new_v = new_value(record_origin, i);
            if (new_v) {
                record.values[i] = new_v.value();
                if (rel.fields[i].has_index) {
                    im.remove_item(rel, i, record_origin.values[i]);
                    im.add_item(rel, i, new_v.value(), record.physical_position);
                }
            }
        }
        rm.update_record(rel, record.physical_position, record);
    }

    void delete_record(const string &rel_name, Record &&record) {
        if (record.physical_position.nil())
            throw logic_error("Unexpected error. This record does not has a physical position.");
        Relation rel = get_relation_exist(rel_name);

        rm.delete_record(rel, record.physical_position);
        for (int field_index : rel.indexes) {
            im.remove_item(rel, field_index, record.values[field_index]);
        }
    }

    unique_ptr<Scanner> select_record(const string &rel_name, Nullable<IndexUsage> index_usage) {
        Relation rel = get_relation_exist(rel_name);
        return rm.select_record(rel, get_index_iterator(rel, index_usage));
    }
};