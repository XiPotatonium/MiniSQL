#include "test.h"
#include "../inc/record_mgr.hpp"
#include "../inc/catalog_mgr.hpp"
#include "../inc/query_executor.hpp"
#include "../inc/console_util.hpp"
#include <windows.h>

TEST_CASE(sql0) {
	remove("data/catalog.dat");
	StorageEngine eng;
	QueryExecutor executor(&eng);
	execute_file(executor, "sql/index-create-delete-0.sql");
}
TEST_CASE(sql1) {
	remove("data/catalog.dat");
	StorageEngine eng;
	QueryExecutor executor(&eng);
	execute_file(executor, "sql/index-create-delete-1.sql");
}
TEST_CASE(sql2) {
	remove("data/catalog.dat");
	StorageEngine eng;
	QueryExecutor executor(&eng);
	execute_file(executor, "sql/index-create-delete-2.sql");
}
TEST_CASE(sql3) {
	remove("data/catalog.dat");
	StorageEngine eng;
	QueryExecutor executor(&eng);
	execute_file(executor, "sql/table-create-drop-0.sql");
}
TEST_CASE(sql4) {
	remove("data/catalog.dat");
	StorageEngine eng;
	QueryExecutor executor(&eng);
	execute_file(executor, "sql/table-create-drop-1.sql");
}
TEST_CASE(sql5) {
	remove("data/catalog.dat");
	StorageEngine eng;
	QueryExecutor executor(&eng);
	execute_file(executor, "sql/table-create-drop-2.sql");
}
TEST_CASE(sql6) {
	remove("data/catalog.dat");
	StorageEngine eng;
	QueryExecutor executor(&eng);
	execute_file(executor, "sql/table-insert-delete-0.sql");
}
TEST_CASE(sql7) {
	remove("data/catalog.dat");
	StorageEngine eng;
	QueryExecutor executor(&eng);
	execute_file(executor, "sql/table-insert-delete-1.sql");
}
TEST_CASE(sql8) {
	remove("data/catalog.dat");
	StorageEngine eng;
	QueryExecutor executor(&eng);
	execute_file(executor, "sql/table-select-0.sql");
}
TEST_CASE(sql9) {
	remove("data/catalog.dat");
	StorageEngine eng;
	QueryExecutor executor(&eng);
	execute_file(executor, "sql/table-select-1.sql");
}
TEST_CASE(sql10) {
	remove("data/catalog.dat");
	StorageEngine eng;
	QueryExecutor executor(&eng);
	execute_file(executor, "sql/test-1w.sql");
}
TEST_CASE(sql11) {
	remove("data/catalog.dat");
	StorageEngine eng;
	QueryExecutor executor(&eng);
	execute_file(executor, "sql/test-1000.sql");
}