#pragma once
#include <regex>
#include <vector>
#include <string>
#include <stdexcept>

using namespace std;

enum class TokenType {
    literal,
    keyword,
    op,
    identifier,
    punctuation
};

struct Token {
    TokenType type;
    string content;
};

struct TokenRegex {
    TokenType type;
    regex regex;
};

/// <summary>
/// TODO 感觉没必要单独搞一个class
/// 或者说Lexer Parser全局使用，只构造一次
/// </summary>
class QueryLexer {
public:
    vector<Token> tokenize(const string& str);
};