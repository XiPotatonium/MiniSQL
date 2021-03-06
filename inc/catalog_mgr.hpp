#pragma once

#include <string>
#include <unordered_map>
#include <optional>

#include "common.hpp"
#include "block_mgr.hpp"
#include "files.hpp"

using namespace std;

class CatalogManager {
    BlockManager* block_mgr;
    void set_index_rel(const string& rel_name, int field_index, bool use_index);

public:
    CatalogManager(BlockManager* block_mgr);

    void add_relation(const Relation& relation);
    void remove_relation(const string& name);
    optional<Relation> get_relation(const string &name);

    void add_index(const string& rel_name, const string& field_name, const string& index_name);
    void remove_index(const string& index_name);
    optional<IndexLocation> get_index_location(const string& index_name);

};
