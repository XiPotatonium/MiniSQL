#include "query_parser.h"

void QueryParser::assert(bool p) {
    if (!p) {
        stringstream ss;
        ss << "Syntax error. pos=" << _pos;
        throw logic_error(ss.str());
    }
}

bool QueryParser::consume(Token& match, initializer_list<const char*> contents) {
    if (_pos >= _tokens.size()) return false;
    for (auto str : contents)
    {
        if (_tokens[_pos].content == str)
        {
            match = _tokens[_pos];
            _pos++;
            return true;
        }
    }
    return false;
}

bool QueryParser::consume(Token& match, initializer_list<TokenType> types) {
    if (_pos >= _tokens.size()) return false;
    for (auto t : types)
    {
        if (_tokens[_pos].type == t)
        {
            match = _tokens[_pos];
            _pos++;
            return true;
        }
    }
    return false;
}

BinaryExpression::Operator QueryParser::binary_op(const string& op) {
    using Op = BinaryExpression::Operator;
    static unordered_map<string, Op> lookup = {
        { "==", Op::EQ },
        { "=", Op::EQ },
        { "is", Op::EQ },
        { "!=", Op::NE },
        { "<", Op::LT },
        { ">", Op::GT },
        { "<=", Op::LE },
        { ">=", Op::GE },
        { "+", Op::ADD },
        { "-", Op::SUB },
        { "*", Op::MUL },
        { "/", Op::DIV },
        { "and", Op::AND },
        { "or", Op::OR }
    };
    return lookup.at(op);
}

pair<Type, Value> QueryParser::literal(const string& lit) {
    if (lit.front() == '\"' && lit.back() == '\"') {
        // string
        string value = lit.substr(1, lit.length() - 2);
        return pair<Type, Value>(Type::create_CHAR(value.length() + 1), Value::create_CHAR(move(value)));
    }
    else if (lit.find('.', 0) != string::npos) {
        // float
        float value;
        stringstream ss(lit);
        ss >> value;
        return pair<Type, Value>(Type::create_FLOAT(), Value::create_FLOAT(value));
    }
    else {
        // int
        int value;
        stringstream ss(lit);
        ss >> value;
        return pair<Type, Value>(Type::create_INT(), Value::create_INT(value));
    }
}


bool QueryParser::left_binary_op_seq(ExprGen child, unique_ptr<Expression> & expr, initializer_list<const char*> ops) {
    int save = _pos;
    if (reset(save) && (this->*child)(expr)) {
        Token op;
        while (consume(op, ops)) {
            unique_ptr<Expression> next;
            assert((this->*child)(next));
            // TODO: parse operator
            expr = unique_ptr<Expression>(new BinaryExpression(binary_op(op.content), move(expr), move(next)));
        }
        return true;
    }
    return false;
}

bool QueryParser::factor(unique_ptr<Expression> & expr) {
    int save = _pos;
    Token match;
    if (reset(save) && consume(match, TokenType::literal)) {
        auto lit = literal(match.content);
        expr = unique_ptr<Expression>(new ConstantExpression(lit.first, lit.second));
        return true;
    }
    else if (reset(save) && consume(match, TokenType::identifier)) {
        expr = unique_ptr<Expression>(new FieldExpression(match.content));
        return true;
    }
    else if (reset(save) && consume("(") && expression(expr) && consume(")")) {
        return true;
    }
    return false;
}

bool QueryParser::uniary(unique_ptr<Expression> & expr) {
    // TODO: uniary
    if (consume("-") && factor(expr)) {
        expr = unique_ptr<Expression>(new UniaryExpression(UniaryExpression::Operator::NEG, move(expr)));
        return true;
    }
    return factor(expr);
}

bool QueryParser::select_field(SelectField & f) {
    int start, end;
    assert(expression(f.expr, start, end));

    bool as = consume("as");
    Token t_alias;
    if (consume(t_alias, TokenType::identifier)) {
        f.expr->name = t_alias.content;
    }
    else {
        assert(!as);
        stringstream ss_name;
        for (int i = start; i < end; i++) {
            ss_name << _tokens[i].content;
        }
        f.expr->name = ss_name.str();
    }

    return true;
}

bool QueryParser::select_list(vector<SelectField> & fields) {
    bool first = true;
    while (1) {
        if (first || consume(",")) {
            SelectField f;
            assert(select_field(f));
            fields.push_back(move(f));
        }
        else {
            break;
        }
        first = false;
    }
    return fields.size() > 0;
}

bool QueryParser::select_source(SelectSource & source) {
    int save = _pos;
    if (consume("(") && select_stmt_inner(source.subquery) && consume(")")) {
        source.type = SelectSource::Type::subquery;
        return true;
    }
    Token rel_physical;
    if (consume(rel_physical, TokenType::identifier)) {
        source.type = SelectSource::Type::physical;
        source.physical = rel_physical.content;
        return true;
    }
    return false;
}

bool QueryParser::select_stmt_inner(unique_ptr<SelectStatement> & stmt) {
    int save = _pos;
    if (reset(save) && consume("select")) {
        vector<SelectField> select;
        unique_ptr<Expression> where;
        SelectSource from;
        select_list(select);
        if (consume("from")) {
            assert(select_source(from));
        }
        if (consume("where")) {
            assert(expression(where));
        }
        stmt = unique_ptr<SelectStatement>(new SelectStatement());
        stmt->select = move(select);
        stmt->from = move(from);
        stmt->where = move(where);
        return true;
    }
    return false;
}

bool QueryParser::select_stmt(unique_ptr<Statement> & stmt) {
    unique_ptr<SelectStatement> select;
    if (select_stmt_inner(select)) {
        stmt = static_cast_unique_ptr<Statement>(move(select));
        return true;
    }
    return false;
}

bool QueryParser::stmt(unique_ptr<Statement> & s) {
    int save = _pos;
    if (reset(save) && select_stmt(s)) {
        return true;
    }
    return false;
}

unique_ptr<Statement> QueryParser::parse(vector<Token> tokens) {
    _tokens = move(tokens);
    _pos = 0;
    unique_ptr<Statement> s;
    assert(stmt(s));
    return s;
}
