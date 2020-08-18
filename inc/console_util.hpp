#pragma once
#include "query_executor.hpp"

bool execute_safe_print(QueryExecutor& executor, const string& expr);
// void disp_records(QueryResult& result);
void draw_line(int* max, int size);
bool execute_file(QueryExecutor& executor, const string& filename);