#pragma once
#include <typeinfo>
#include "query_parser.hpp"
#include "storage_engine.hpp"
#include <unordered_map>

struct QueryResult {
    Relation relation;
    vector<Record> records;
    string prompt;
	bool failed = false;
};

class QueryExecutor {
    //CatalogManager* _cm;
    //RecordManager* _rm;
    StorageEngine* _storage_eng;

	bool value_less(Nullable<Type> t, Nullable<Value> v1, Nullable<Value> v2);
	Nullable<IndexUsage> search_index(BinaryExpression* exp, Relation& relation);
    unique_ptr<Scanner> select_scanner(SelectStatement* stmt);
    QueryResult select_exe(SelectStatement* stmt);
    QueryResult update_exe(UpdateStatement* stmt);
    QueryResult insert_exe(InsertStatement* stmt);
    QueryResult delete_exe(DeleteStatement* stmt);
	QueryResult create_table_exe(CreateTableStatement* stmt);
	QueryResult create_index_exe(CreateIndexStatement* stmt);
	QueryResult drop_table_exe(DropTableStatement* stmt);
	QueryResult drop_index_exe(DropIndexStatement* stmt);

public:
    QueryExecutor(StorageEngine* storage_eng) : _storage_eng(storage_eng) {};
    QueryResult execute(unique_ptr<Statement> stmt);
};


inline QueryResult execute_safe(QueryExecutor& executor, const string& query) {
    try {
        return executor.execute(
            QueryParser().parse(
                QueryLexer().tokenize(query)));
    }
    catch (logic_error & e) {
        QueryResult r;
        r.failed = true;
        r.prompt = "Query failed. " + string(e.what());
        return r;
    }
}