#pragma once
#include "common.hpp"
#include "nullable.hpp"
#include "block_mgr.hpp"
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
	Nullable<Value> from, to;
};
