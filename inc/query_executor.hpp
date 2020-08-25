#pragma once
#include "query_parser.hpp"
#include "storage_engine.hpp"
#include <typeinfo>
#include <unordered_map>

struct QueryResult {
    Relation relation;
    vector<Record> records;
    /// <summary>
    /// 执行完成后的说明，OK还是不OK，错误原因
    /// </summary>
    string prompt;
    /// <summary>
    /// TODO 和prompt有信息重复
    /// </summary>
    bool failed = false;
};

class QueryExecutor {
    // CatalogManager* _cm;
    // RecordManager* _rm;
    // TODO shared_ptr
    StorageEngine *_storage_eng;

    bool value_less(optional<Type> t, optional<Value> v1, optional<Value> v2);
    optional<IndexUsage> search_index(BinaryExpression *exp, Relation &relation);
    // TODO 下面的stmt指针全部可以改成const ref
    unique_ptr<Scanner> select_scanner(SelectStatement *stmt);
    QueryResult select_exe(SelectStatement *stmt);
    QueryResult update_exe(UpdateStatement *stmt);
    QueryResult insert_exe(InsertStatement *stmt);
    QueryResult delete_exe(DeleteStatement *stmt);
    QueryResult create_table_exe(CreateTableStatement *stmt);
    QueryResult create_index_exe(CreateIndexStatement *stmt);
    QueryResult drop_table_exe(DropTableStatement *stmt);
    QueryResult drop_index_exe(DropIndexStatement *stmt);

public:
    QueryExecutor(StorageEngine *storage_eng) : _storage_eng(storage_eng){};
    QueryResult execute(unique_ptr<Statement> stmt);
};

inline QueryResult execute_safe(QueryExecutor &executor, const string &query) {
    try {
        return executor.execute(QueryParser().parse(QueryLexer().tokenize(query)));
    } catch (logic_error &e) {
        QueryResult r;
        r.failed = true;
        r.prompt = "Query failed. " + string(e.what());
        return r;
    }
}