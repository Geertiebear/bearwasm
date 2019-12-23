#ifndef BEARWASM_MODULE_H
#define BEARWASM_MODULE_H

#include <string>
#include <map>
#include <fstream>
#include <bearwasm/Format.h>
#include <bearwasm/Interpreter.h>

namespace bearwasm {

using FunctionTypes = std::vector<FunctionType>;
using Functions = std::vector<uint32_t>;
using Tables = std::vector<Table>;
using MemoryTypes = std::vector<MemoryType>;
using Globals = std::vector<GlobalValue>;
using FunctionCodes = std::vector<Code>;
using FunctionNames = std::map<uint32_t, std::string>;
using Data = std::vector<DataEntry>;
using Imports = std::vector<Import>;

class Module {
public:
	Module(const std::string &path);

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

	std::ifstream file;
};

} /* namespace bearwasm */

#endif
