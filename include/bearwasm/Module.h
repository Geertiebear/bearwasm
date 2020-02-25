#ifndef BEARWASM_MODULE_H
#define BEARWASM_MODULE_H

#include <frg/vector.hpp>
#include <frg/string.hpp>
#include <frg/hash.hpp>
#include <frg/hash_map.hpp>
#include <bearwasm/host.hpp>
#include <bearwasm/Util.h>
#include <bearwasm/Format.h>
#include <bearwasm/Interpreter.h>

namespace bearwasm {

using FunctionTypes = frg::vector<FunctionType, frg_allocator>;
using Functions = frg::vector<uint32_t, frg_allocator>;
using Tables = frg::vector<Table, frg_allocator>;
using MemoryTypes = frg::vector<MemoryType, frg_allocator>;
using Globals = frg::vector<GlobalValue, frg_allocator>;
using FunctionCodes = frg::vector<Code, frg_allocator>;
using FunctionNames = frg::hash_map<uint32_t, frg::string<frg_allocator>,
      frg::hash<int>, frg_allocator>;
using Data = frg::vector<DataEntry, frg_allocator>;
using Imports = frg::vector<Import, frg_allocator>;

class Module {
public:
	Module(DataStream *stream);

	FunctionTypes function_types;
	Functions functions;
	Tables tables;
	MemoryTypes memory_types;
	Globals globals;
	Exports exports;
	FunctionCodes function_code;
	FunctionNames function_names;
	Data data;
	Imports imports;
private:
	void read_sections();
	void parse_type_section();
	void parse_function_section();
	void parse_table_section();
	void parse_memory_section();
	void parse_global_section();
	void parse_export_section();
	void parse_code_section();
	void parse_data_section();
	void parse_import_section();
	void parse_custom_section(int length);
	bool verify_signature();
	
	void dump_function_types();
	void dump_functions();
	void dump_tables();
	void dump_memory();
	void dump_globals();
	void dump_exports();
	void dump_code();
	void dump_imports();

	DataStream *stream;
};

} /* namespace bearwasm */

#endif
