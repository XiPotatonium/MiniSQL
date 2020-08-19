#include <iostream>

#include "../inc/console_util.hpp"
#include "../inc/query_executor.hpp"
#include "../inc/query_lexer.hpp"
#include "../inc/query_parser.hpp"

using namespace std;

int main(void) {
    StorageEngine eng;
    QueryExecutor executor = QueryExecutor(&eng);
    string str;
    stringstream ss_expr;
    char c;
    cout << "MiniSQL v1.0" << endl;
    while (true) {
        eng.flush();
        cout << endl << ">>>";
        ss_expr = stringstream();
        while (true) {
            c = getchar();
            if (c == ';') {
                // 指令结束
                break;
            }
            if (c == '\n') {
                // 允许多行同一条指令
                cout << "-->";
                c = ' ';
            }
            ss_expr << c;
        }
        ss_expr >> str;
        if (str.size() == 0) {
            continue;   // 空指令
        }
        if (str == "exit") {
            break;
        } else if (str == "exe") {
            // 执行SQL脚本
            if (ss_expr.eof()) {
                cout << "Please specify the file address." << endl;
            } else {
                // TODO 这里可以考虑支持双引号
                ss_expr >> str;
                cout << "Executing file: " << str << endl;
                execute_file(executor, str);
            }
        } else {
            str = ss_expr.str();
            cout << "Executing command: " << str << endl;
            execute_safe_print(executor, str);
        }

		// TODO 清除缓冲区中多余的内容，有没有更优雅的方法？
        while (getchar() != '\n')
            ;
    }

    return 0;
}
