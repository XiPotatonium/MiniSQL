#pragma once
#include "common.hpp"
#include "block_mgr.hpp"
#include <optional>

// TODO: B-Tree���
class IndexIterator {
public:
	bool next() 
	{
		i++;
		return i < x.size();
	}
    RecordPosition current() 
	{
		return x[i];
	}
	IndexIterator(vector<RecordPosition> rp) : x(move(rp)) {};
private:
	vector<RecordPosition> x;
	int i = -1;
};

struct IndexUsage {
	int field_index;
	optional<Value> from;
	optional<Value> to;
};
