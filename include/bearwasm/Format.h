#ifndef BEARWASM_FORMAT_H
#define BEARWASM_FORMAT_H

#include <bearwasm/FriggAllocator.h>
#include <bearwasm/BinaryFormat.h>
#include <bearwasm/Util.h>

#include <frg/vector.hpp>
#include <frg/string.hpp>

namespace bearwasm {

struct Instruction;

using Expression = frg::vector<Instruction, frg_allocator>;
using MemoryType = Limit;
using Value = union {
		int32_t int32_val; 
		int64_t int64_val;
		float float_val; 
		double double_val;
	};
using Local = BinaryType;

struct FunctionType {
    frg::vector<BinaryType, frg_allocator> results, parameters;
};

struct Table {
	TableType type;
	Limit limit;
	/* for now just contains a vector of
	 * function types but that may change later */
    frg::vector<uint32_t, frg_allocator> data;
};

struct Code {
	uint32_t size;
	Expression expression;
    frg::vector<Local, frg_allocator> locals;
};

struct GlobalValue {
	BinaryType type;
	Value value;
	bool mut;
};

struct Export {
	frg::string<frg_allocator> name;
	int index;
};

struct Import {
	frg::string<frg_allocator> module, name;
	int description, idx;
};

struct Exports {
    frg::vector<Export, frg_allocator> func, table, mem, global;
};

struct DataEntry {
	int memidx;
	int offset;
    frg::vector<uint8_t, frg_allocator> bytes;
};

}/* namespace bearwasm */

#endif
