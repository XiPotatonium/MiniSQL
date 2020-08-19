#pragma once

#include "block_mgr.hpp"
#include "files.hpp"
#include <sstream>
#include <stdarg.h>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

const int NAME_LENGTH = 32;         // 名称最多多长
const int MAX_FIELDS = 32;          // 一个表最多有几个字段
const int MAX_RELATIONS = 32;       // 最多有几个表
const int MAX_INDEXES = 16;         // 最多有几个索引
const int MAX_RECORD_LENGTH = 1024; // 一条记录最多多长

// 代表数据库中的一个类型，可以是INT、FLOAT、或是CHAR(n)
struct Type {
    // tagged union
    enum class Tag { INT, CHAR, FLOAT } tag;
    // TODO 删掉这个union
    union {
        struct {
            int len;
        } CHAR;
    };
    int length() const;
    static Type create_INT() {
        Type t;
        t.tag = Tag::INT;
        return t;
    }
    static Type create_FLOAT() {
        Type t;
        t.tag = Tag::FLOAT;
        return t;
    }
    static Type create_CHAR(int len) {
        Type t;
        t.tag = Tag::CHAR;
        t.CHAR.len = len;
        return t;
    }
};

struct FieldData {
    /// <summary>
    /// 名字
    /// </summary>
    char name[NAME_LENGTH] = {0};
    bool unique;
    bool has_index;
    char index_name[NAME_LENGTH] = {0};
    Type type;
};

struct Field {
    // 这几个field在QueryExecutor::create_table_exe里面确定了
    string name;
    bool unique = false;
    bool has_index = false;
    string index_name;
    Type type;
    // 这个在Relation::update的时候才被确定
    int offset;

    FieldData to_file() const;
    void from_file(const FieldData &f);
    Field() = default;
    Field(string name, Type type) : name(move(name)), type(type){};
};

struct RelationData {
    int field_count;
    FieldData fields[MAX_FIELDS];
    // TODO: 索引信息
};
static_assert(sizeof(RelationData) <= BLOCK_SIZE,
              "RelationData cannot be contained by a single block. Consider reorganize data.");

struct Relation {
    string name;
    vector<Field> fields;
    vector<int> indexes;
    void update();
    int record_length() const;
    RelationData to_file() const;
    void from_file(const RelationData &f);
    Relation() = default;
    Relation(string name) : name(move(name)){};
    // Relation(Relation&& rel) = default;
};

struct IndexNameLocationData {
    char index_name[NAME_LENGTH];
    char relation_name[NAME_LENGTH];
    int field;
};

struct IndexLocation {
    string relation_name;
    int field;
    void from_file(const IndexNameLocationData &f);
};

// 数据库在文件中的表示。必须放在独立的文件中。
struct DatabaseData {
    /// <summary>
    /// 存储relation名。rel_names[i]的relation，将处于本文件第i+1个块中。
    /// </summary>
    char rel_names[MAX_RELATIONS][NAME_LENGTH] = {0};
    IndexNameLocationData indexes[MAX_INDEXES] = {0};
    int get_block(const char *relation_name) const;
    int get_free_block() const;
    int get_index(const char *index_name) const;
    int get_free_index() const;
    int block_to_rel(int i) { return i - 1; }
    int rel_to_block(int i) { return i + 1; }
};

// TODO 数据库元数据要小于4K，是不是太少了
static_assert(sizeof(DatabaseData) <= BLOCK_SIZE,
              "DatabaseData cannot be contained by a single block. Consider reorganize data.");

// 代表一个记录位置
struct RecordPosition {
    int block_index = -1;
    int pos = -1;
    RecordPosition() = default;
    RecordPosition(int block_index, int pos) : block_index(block_index), pos(pos){};
    /// <summary>
    /// 是否未初始化
    /// </summary>
    /// <returns></returns>
    bool nil() const { return block_index < 0 || pos < 0; }

    static int cmp(RecordPosition p1, RecordPosition p2) {
        if (p1.nil() || p2.nil())
            throw logic_error("Cannot compare nil value.");
        if (p1.block_index != p2.block_index)
            return p1.block_index - p2.block_index;
        return p1.pos - p2.pos;
    }

    RecordPosition next(int record_len) const {
        RecordPosition n = *this;
        n.pos += record_len;
        if (BLOCK_SIZE - n.pos <= record_len) {
            n.block_index++;    // 记录不会跨block
            n.pos = 0;
        }
        return n;
    }
};

/// <summary>
/// 记录文件中，第一条记录的起始位置
/// TODO 第一个block的一半，这...要么第二个block的开头？
/// </summary>
const RecordPosition RECORD_START = RecordPosition(0, 2048);

inline bool operator<(RecordPosition rp1, RecordPosition rp2) { return RecordPosition::cmp(rp1, rp2) < 0; }

/// <summary>
/// relation文件的头部，第一个block
/// </summary>
struct RelationEntryData {
    bool deleted = false;
    RecordPosition free_head;   // 内碎片
    RecordPosition empty;       // 后边的空间，注意可能未分配
};

// 两个链表，一个是使用中，一个是空闲
struct RecordEntryData {
    bool use;
    /// <summary>
    /// 他是吧RecordEntryData顺便当链表用了？
    /// </summary>
    RecordPosition free_next;
    uint8_t values[0];
};

// 代表数据库中的一个值
struct Value {
private:
    template <typename T>
    static int cmp(T t1, T t2) {
        if (t1 < t2)
            return -1;
        if (t1 > t2)
            return 1;
        return 0;
    }

public:
    union {
        int INT = 0;
        float FLOAT;
    };
    string CHAR;
    bool greater_than(const Value &x, Type type);
    static int cmp(Type type, const Value &v1, const Value &v2) {
        switch (type.tag) {
        case Type::Tag::INT:
            return cmp(v1.INT, v2.INT);
        case Type::Tag::FLOAT:
            return cmp(v1.FLOAT, v2.FLOAT);
        case Type::Tag::CHAR:
            return cmp(v1.CHAR, v2.CHAR);
        default:
            throw logic_error("Unexpected type.");
        }
    }
    void write(void *addr, Type type) const;
    void parse(void *addr, Type type);
    Value &operator=(Value &&v) noexcept {
        if (&v != this) {
            INT = v.INT;
            CHAR = move(v.CHAR);
        }
        return *this;
    }
    Value &operator=(const Value &v) {
        INT = v.INT;
        CHAR = v.CHAR;
        return *this;
    }
    Value() = default;
    Value(const Value &v) { *this = v; }
    Value(Value &&v) noexcept { *this = move(v); }
    static Value create_INT(int i) {
        Value v;
        v.INT = i;
        return v;
    }
    static Value create_FLOAT(float f) {
        Value v;
        v.FLOAT = f;
        return v;
    }
    static Value create_CHAR(string s) {
        Value v;
        v.CHAR = move(s);
        return v;
    }
};

struct Record {
    vector<Value> values;
    RecordPosition physical_position;
};

template <typename To, typename From>
inline unique_ptr<To> static_cast_unique_ptr(unique_ptr<From> &&old) {
    return unique_ptr<To>{static_cast<To *>(old.release())};
}

string string_format(const string fmt_str, ...);