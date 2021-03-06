#pragma once
#include <memory>
#include "common.hpp"
#include "expression.hpp"
#include "index.hpp"

class Scanner {
public:
    virtual Record& current() = 0;
    virtual bool next() = 0;
    virtual const Relation& rel_out() = 0;
    virtual ~Scanner() {}
};

template<typename Cont, typename It = typename Cont::iterator>
class ContainerScanner : public Scanner {
    Cont container;
    It iterator;
    bool first = true;
public:
    ContainerScanner(Cont container) : container(move(container)) {

    }
    Record& current() {
        return *iterator;
    }
    bool next() {
        if (first) iterator = container.begin();
        else ++iterator;
        return iterator != container.end();
    }
    const Relation& rel_out() {
        throw logic_error("Not supported!");
    }
};


class DiskScanner : public Scanner {
    BlockManager* block_mgr;
    Relation rel;
    BlockGuard bg_rel;
    int record_length;

    RecordPosition cur_pos;
    Record cur_record;
    unique_ptr<IndexIterator> index_it;
    void parse_cur_record(RecordEntryData* rec_entry, RecordPosition rp);

public:
    DiskScanner(BlockManager* block_mgr, Relation rel, unique_ptr<IndexIterator> index_it);
    Record& current() { return cur_record; }
    bool next();
    const Relation& rel_out() { return rel; }
};

class FilterScanner : public Scanner {
    unique_ptr<Scanner> _from;
    unique_ptr<Expression> _pred;
    Record cur_record;

public:
    FilterScanner(unique_ptr<Scanner> from, unique_ptr<Expression> pred);
    Record& current() { return cur_record; }
    bool next();
    const Relation& rel_out() { return _from->rel_out(); }
};

class ProjectScanner : public Scanner {
    unique_ptr<Scanner> _from;
    vector<unique_ptr<Expression>> _fields;
    Relation _rel_out;
    Record _cur_record;
    void resolve(const Relation& rel) {
        for (auto& f : _fields) {
            f->resolve(rel);
        }
    }
public:
    ProjectScanner(unique_ptr<Scanner> from, vector<unique_ptr<Expression>> fields);
    Record& current() { return _cur_record; }
    bool next();
    const Relation& rel_out() { return _rel_out; }
};