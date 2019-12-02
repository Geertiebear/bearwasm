#ifndef BEARWASM_FORMAT_H
#define BEARWASM_FORMAT_H

#include <vector>
#include <bearwasm/BinaryFormat.h>
#include <bearwasm/Util.h>

namespace bearwasm {

struct Instruction;

using Expression = std::vector<Instruction>;
using MemoryType = Limit;
using Value = union {
		int32_t int32_val; 
		int64_t int64_val;
		float float_val; 
		double double_val;
	};
using Local = BinaryType;

struct FunctionType {
	std::vector<BinaryType> results, parameters;
};

struct Table {
	TableType type;
	Limit limit;
	/* for now just contains a vector of
	 * function types but that may change later */
	std::vector<uint32_t> data;
};

struct Code {
	uint32_t size;
	Expression expression;
	std::vector<Local> locals;
};

struct GlobalValue {
	BinaryType type;
	Value value;
	bool mut;
};

struct Export {
	std::string name;
	int index;
};

struct Exports {
	std::vector<Export> func, table, mem, global;
};

struct DataEntry {
	int memidx;
	int offset;
	std::vector<uint8_t> bytes;
};

}/* namespace bearwasm */

#endif
