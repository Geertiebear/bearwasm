#include <bearwasm/Module.h>
#include <bearwasm/Util.h>
#include <bearwasm/BinaryFormat.h>
#include <bearwasm/Interpreter.h>

#include <iostream>

namespace bearwasm {

Module::Module(const std::string &path) {
	file.open(path);
	if (!file.is_open()) {
		std::cerr << "Error opening wasm module" << std::endl;
		exit(1);
	}

	if(!verify_signature()) {
		std::cerr << "Error verifiying module signature" << std::endl;
		exit(1);
	}
	std::cout << "Verified wasm binary!" << std::endl;

	auto version = stream_read<uint32_t>(file);
	if (!version)
		panic("Error reading wasm version");
	std::cout << "Webassembly version " << *version << std::endl;
	
	read_sections();
}

bool Module::verify_signature() {
	char c = file.get();
	if (c != '\0') return false;

	std::string magic;
	magic.resize(3);
	file.read(&magic[0], 3);
	if (magic != "asm") return false;

	return true;
}

void Module::read_sections() {
	while (auto id = stream_read<uint8_t>(file)) {
		auto length = decode_varuint_s<uint32_t>(file);
		switch (*id) {
			case SECTION_TYPE:
				parse_type_section();
				dump_function_types();
				break;
			case SECTION_FUNCTION:
				parse_function_section();
				dump_functions();
				break;
			case SECTION_TABLE:
				parse_table_section();
				dump_tables();
				break;
			case SECTION_MEMORY:
				parse_memory_section();
				dump_memory();
				break;
			case SECTION_GLOBAL:
				parse_global_section();
				dump_globals();
				break;
			case SECTION_EXPORT:
				parse_export_section();
				dump_exports();
				break;
			case SECTION_CODE:
				parse_code_section();
				dump_code();
				break;
			case SECTION_CUSTOM:
				parse_custom_section(*length);
				break;
			default:
				std::cout << "Encountered unknown section with id "
					<< static_cast<int>(*id) << std::endl;
				file.seekg(*length, std::ios::cur);
				break;
		}
	}
}

void Module::parse_type_section() {
	auto num_types = stream_read<uint8_t>(file);
	for (int i = 0; i < num_types; i++) {
		FunctionType function_type;

		auto start = stream_read<uint8_t>(file);
		if (*start != 0x60)
			panic("Expected 0x60 while parsing type section");

		auto num_params = stream_read<uint8_t>(file);
		for (int j = 0; j < *num_params; j++) {
			auto type = stream_read<BinaryType>(file);
			function_type.parameters.push_back(*type);
		}

		auto num_results = stream_read<uint8_t>(file);
		for (int j = 0; j < *num_results; j++) {
			auto type = stream_read<BinaryType>(file);
			function_type.results.push_back(*type);
		}


		function_types.push_back(function_type);
	}
}

void Module::parse_function_section() {
	auto num_functions = stream_read<uint8_t>(file);
	for (int i = 0; i < num_functions; i++) {
		auto type_idx = decode_varuint_s<uint32_t>(file);
		if (!type_idx)
			panic("Error reading type idx in function section");
		functions.push_back(*type_idx);
	}
}

void Module::parse_table_section() {
	auto num_tables = stream_read<uint8_t>(file);
	for (int i = 0; i < num_tables; i++) {
		Table table;
		auto table_type = stream_read<TableType>(file);
		if (!table_type)
			panic("Error reading table type!");
		table.type = *table_type;

		auto limit = decode_limit(file);
		if (!limit)
			panic("Error reading limit");
		table.limit = *limit;

		tables.push_back(table);
	}
}

void Module::parse_memory_section() {
	auto num_memory_types = stream_read<uint8_t>(file);
	for (int i = 0; i < num_memory_types; i++) {
		/* memory types are just limits */
		auto memory_type = decode_limit(file);
		if (!memory_type)
			panic("Error reading memory type");
		memory_types.push_back(*memory_type);
	}
}

void Module::parse_global_section() {
	auto num_global_types = stream_read<uint8_t>(file);
	for (int i = 0; i < num_global_types; i++) {
		auto ret = Interpreter::interpret_global(file);
		if (!ret)
			panic("Error decoding global");
		globals.push_back(*ret);
	}
}

void Module::parse_export_section() {
	auto num_exports = stream_read<uint8_t>(file);
	for (int i = 0; i < num_exports; i++) {
		auto name = read_string(file);
		if (!name)
			panic("Unable to read export name");
		int type = file.get();
		auto index = decode_varuint_s<uint32_t>(file);
		if (!index)
			panic("Unable to read export index");

		Export exp;
		exp.name = *name;
		exp.index = *index;
		switch (type) {
			case EXPORT_FUNC:
				exports.func.push_back(exp);
				break;
			case EXPORT_TABLE:
				exports.table.push_back(exp);
				break;
			case EXPORT_MEM:
				exports.mem.push_back(exp);
				break;
			case EXPORT_GLOBAL:
				exports.global.push_back(exp);
				break;
			default:
				panic("Unrecognized export type " +
						std::to_string(type));
		}
	}
}

void Module::parse_code_section() {
	auto num_functions = stream_read<uint8_t>(file);
	for (int i = 0; i < num_functions; i++) {
		Code code;

		auto size = decode_varuint_s<uint32_t>(file);
		if (!size)
			panic("Unable to read function size");
		code.size = *size;

		auto num_locals = stream_read<uint8_t>(file);
		for (int j = 0; j < num_locals;) {
			auto count = decode_varuint_s<uint32_t>(file);
			if (!count)
				panic("Unable to read local count");
			auto type = stream_read<BinaryType>(file);
			for (size_t k = 0; k < *count; k++)
				code.locals.push_back(*type);
			j += *count;
		}

		auto start_pos = file.tellg();
		auto instructions = Interpreter::decode_code(file);

		code.size = file.tellg() - start_pos;
		code.expression = instructions;

		function_code.push_back(code);		
	}
}

void Module::parse_custom_section(int length) {
	auto name = read_string(file);
	if (!name)
		panic("Error reading name of custom section");
	if (name == "name") {
		/* name section */
		/* TODO: parse names of other types besides functions */
		auto type = decode_varuint_s<uint8_t>(file);
		if (!type) panic("Error reading names type");
		auto length = decode_varuint_s<uint32_t>(file);
		if (!length) panic("Error reading names length");
		auto num_names = decode_varuint_s<uint32_t>(file);
		if (!num_names) panic("Error reading number of names");

		for (int i = 0; i < num_names; i++) {
			auto name_index = decode_varuint_s<uint32_t>(file);
			auto name = read_string(file);
			if (!name) panic("Error reading function name");
			function_names.insert(std::make_pair(*name_index, *name));
		}
	} else {
		std::cout << "Encountered unknown custom section " <<
			*name << std::endl;
		file.seekg(length - (*name).length(), std::ios::cur);
	}
}

static std::string type_to_string(BinaryType type) {
	switch (type) {
		case I_32: return "i32";
		case I_64: return "i64";
		case F_32: return "f32";
		case F_64: return "f64";
	}
	return std::string();
}

void Module::dump_function_types() {
	std::cout << "Types:" << std::endl;
	int i = 0;
	for (auto &function_type : function_types) {
		std::cout << "\t[" << i << "] (";
		for (auto parameter : function_type.parameters) {
			std::cout << type_to_string(parameter) << " ";
		}
		std::cout << ") -> ";
		for (auto result : function_type.results)
			std::cout << type_to_string(result) << " ";
		std::cout << std::endl;
		i++;
	}
}

void Module::dump_functions() {
	std::cout << "Functions:" << std::endl;
	for (size_t i = 0; i < functions.size(); i++) {
		auto type_idx = functions[i];
		std::cout << "\t[" << i << "] typeidx " << type_idx
			<< std::endl;
	}
}

static std::string table_type_to_string(TableType type) {
	switch (type) {
		case TABLE_FUNCREF: return "funcref";
		default: return "unkown";
	}
}

void Module::dump_tables() {
	std::cout << "Tables:" << std::endl;
	for (size_t i = 0; i < tables.size(); i++) {
		auto &table = tables[i];
		std::cout << "\t[" << i << "] type: " <<
			table_type_to_string(table.type)
			<< " min: " << table.limit.first << " max: "
			<< table.limit.second << std::endl;
	}
}

void Module::dump_memory() {
	std::cout << "Memory:" << std::endl;
	int i = 0;
	for (auto &type : memory_types) {
		std::cout << "\t[" << i << "] min: " <<
			type.first << " max: " << type.second
			<< std::endl;
		i++;
	}
}

void Module::dump_globals() {
	std::cout << "Globals:" << std::endl;
	int i = 0;
	for (auto &global : globals) {
		std::cout << "\t[" << i << "] value: ";
		std::visit([](auto &&arg) { std::cout << arg; },
				global.value);
		std::cout << " mut: " << global.mut << std::endl;
	}
}

void Module::dump_exports() {
	std::cout << "Exports" << std::endl;
	for (auto &exp : exports.func) {
		std::cout << "\t func[" << exp.index << "] name: "
			<< exp.name << std::endl;
	}
	for (auto &exp : exports.table) {
		std::cout << "\t table[" << exp.index << "] name: "
			<< exp.name << std::endl;
	}
	for (auto &exp : exports.mem) {
		std::cout << "\t mem[" << exp.index << "] name: "
			<< exp.name << std::endl;
	}
	for (auto &exp : exports.global) {
		std::cout << "\t global[" << exp.index << "] name: "
			<< exp.name << std::endl;
	}
}

void Module::dump_code() {
	std::cout << "Code:" << std::endl;
	int i = 0;
	for (auto &code : function_code) {
		std::cout << "\t[" << i << "] size: "
			<< code.size << " locals: ";
		for (auto &local : code.locals)
			std::cout << type_to_string(local) << " ";
		std::cout << std::endl;
		i++;
	}
}

} /* namespace bearwasm */
