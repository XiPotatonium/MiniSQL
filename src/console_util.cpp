#include "../inc/console_util.hpp"
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>

//#define DISABLE_PROMPT

using namespace std;

void disp_records(QueryResult &result) {
    int* max = new int[result.relation.fields.size()];
    for (int i = 0; i < result.relation.fields.size(); ++i) {
        max[i] = 2 + result.relation.fields[i].name.size();
        if (max[i] < 8) {
            max[i] = 8;
        }
    }
    for (int i = 0; i < result.records.size(); ++i) {
        for (int j = 0; j < result.relation.fields.size(); ++j) {
            if (result.relation.fields[j].type.tag == Type::Tag::CHAR) {
                if (max[j] < result.records[i].values[j].CHAR.size() + 2) {
                    max[j] = result.records[i].values[j].CHAR.size() + 2;
                }
            }
        }
    }
    draw_line(max, result.relation.fields.size());
    setiosflags(ios::left);
    for (int i = 0; i < result.relation.fields.size(); i++) {
        cout << "| " << setfill(' ') << setw(max[i]) << result.relation.fields[i].name << ' ';
    }
    cout << '|' << endl;
    draw_line(max, result.relation.fields.size());
    for (int i = 0; i < result.records.size(); i++) {
        for (int j = 0; j < result.relation.fields.size(); j++) {
            cout << "| " << setfill(' ') << setw(max[j]);
            if (result.relation.fields[j].type.tag == Type::Tag::INT) {
                cout << get<int>(result.records[i].values[j].basic_v) << ' ';
            } else if (result.relation.fields[j].type.tag == Type::Tag::FLOAT) {
                cout << get<int>(result.records[i].values[j].basic_v) << ' ';
            } else if (result.relation.fields[j].type.tag == Type::Tag::CHAR) {
                cout << result.records[i].values[j].CHAR << ' ';
            }
        }
        cout << '|' << endl;
    }
    draw_line(max, result.relation.fields.size());
    delete[] max;
}

bool execute_safe_print(QueryExecutor &executor, const string &expr) {
    clock_t start = clock();
    QueryResult result = execute_safe(executor, expr);
    clock_t end = clock();
#ifndef DISABLE_PROMPT
    if (result.relation.fields.size() != 0) {
        // Show select results
        disp_records(result);
    }
    cout << result.prompt << " (" << fixed << setprecision(5) << ((double)end - start) / CLK_TCK << "s)" << endl
         << endl;
#else
    if (result.failed) {
        cout << result.prompt << " (" << fixed << setprecision(5) << ((double)end - start) / CLK_TCK << "s)" << endl
             << endl;
    }
#endif
    cout.unsetf(ios::fixed);
    return !result.failed;
}

void draw_line(int *max, int size) {
    for (int i = 0; i < size; i++) {
        cout << "+-";
        for (int j = 0; j <= max[i]; j++) {
            cout << '-';
        }
    }
    cout << '+' << endl;
}

/// <summary>
/// 
/// </summary>
/// <param name="executor"></param>
/// <param name="filename">文件名</param>
/// <returns></returns>
bool execute_file(QueryExecutor &executor, const string &filename) {
    ifstream ifs = ifstream(filename, ios::in);
    string str;
    char c;
    if (!ifs.good()) {
        printf("Fail to open file\n");
    } else {
        while (true) {
            stringstream ss_expr = stringstream();
            while (!ifs.eof() && ifs.get() == '\n')
                ;
            if (!ifs.eof()) {
                ifs.unget();
            }
            while (true) {
                c = ifs.get();
                if (ifs.eof()) {
                    break;
                }
                if (c == ';') {
                    break;
                }
                ss_expr << c;
            }
            if (ifs.eof()) {
                break;
            }
            str = ss_expr.str();
#ifndef DISABLE_PROMPT
            cout << "Executing command: " << str << endl;
#endif
            bool succeeded = execute_safe_print(executor, str);
            if (!succeeded) {
                printf("Executing aborted due to previous error.\n");
                return false;
            }
        }
    }
    return true;
}
