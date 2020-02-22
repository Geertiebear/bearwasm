#include <bearwasm/Module.h>
#include <bearwasm/Util.h>
#include <bearwasm/BinaryFormat.h>
#include <bearwasm/Interpreter.h>

namespace bearwasm {

Module::Module(DataStream *stream) :
	stream(stream), function_names(frg::hash<int>{}) {

	if(!verify_signature()) {
		panic("Error verifiying module signature\n");
	}

	auto version = stream_read<uint32_t>(stream);
	if (!version)
		panic("Error reading wasm version");
	log_info("Webassembly version %d\n", *version);
	
	read_sections();
}

bool Module::verify_signature() {
	auto c = stream->get();
	if (!c) return false;
	if (c != '\0') return false;

	frg::string<frg_allocator> magic;
	magic.resize(3);
	stream->read(magic.data(), 3);
	if (magic != "asm") return false;

	return true;
}

void Module::read_sections() {
	while (auto id = stream_read<uint8_t>(stream)) {
		auto length = decode_varuint<uint32_t>(stream);
		switch (*id) {
			case SECTION_TYPE:
				parse_type_section();
				dump_function_types();
				break;
			case SECTION_IMPORT:
				parse_import_section();
				dump_imports();
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
			case SECTION_DATA:
				parse_data_section();
				break;
			case SECTION_CUSTOM:
				parse_custom_section(*length);
				break;
			default:
				log_warn("Encountered unknown section with id %d\n",
					static_cast<int>(*id));
				stream->seek(*length, DataStream::BWASM_SEEK_CUR);
				break;
		}
	}
}

void Module::parse_type_section() {
	auto num_types = stream_read<uint8_t>(stream);
	for (int i = 0; i < num_types; i++) {
		FunctionType function_type;

		auto start = stream_read<uint8_t>(stream);
		if (*start != 0x60)
			panic("Expected 0x60 while parsing type section");

		auto num_params = stream_read<uint8_t>(stream);
		for (int j = 0; j < *num_params; j++) {
			auto type = stream_read<BinaryType>(stream);
			function_type.parameters.push(*type);
		}

		auto num_results = stream_read<uint8_t>(stream);
		for (int j = 0; j < *num_results; j++) {
			auto type = stream_read<BinaryType>(stream);
			function_type.results.push(*type);
		}

		function_types.push(function_type);
	}
}

void Module::parse_function_section() {
	auto num_functions = stream_read<uint8_t>(stream);
	for (int i = 0; i < num_functions; i++) {
		auto type_idx = decode_varuint<uint32_t>(stream);
		if (!type_idx)
			panic("Error reading type idx in function section");
		functions.push(*type_idx);
	}
}

void Module::parse_table_section() {
	auto num_tables = stream_read<uint8_t>(stream);
	for (int i = 0; i < num_tables; i++) {
		Table table;
		auto table_type = stream_read<TableType>(stream);
		if (!table_type)
			panic("Error reading table type!");
		table.type = *table_type;

		auto limit = decode_limit(stream);
		if (!limit)
			panic("Error reading limit");
		table.limit = *limit;

		tables.push(table);
	}
}

void Module::parse_memory_section() {
	auto num_memory_types = stream_read<uint8_t>(stream);
	for (int i = 0; i < num_memory_types; i++) {
		/* memory types are just limits */
		auto memory_type = decode_limit(stream);
		if (!memory_type)
			panic("Error reading memory type");
		memory_types.push(*memory_type);
	}
}

void Module::parse_global_section() {
	auto num_global_types = stream_read<uint8_t>(stream);
	for (int i = 0; i < num_global_types; i++) {
		auto ret = Interpreter::interpret_global(stream);
		if (!ret)
			panic("Error decoding global");
		globals.push(*ret);
	}
}

void Module::parse_export_section() {
	auto num_exports = stream_read<uint8_t>(stream);
	for (int i = 0; i < num_exports; i++) {
		auto name = read_string(stream);
		if (!name)
			panic("Unable to read export name");
		auto type = stream->get();
		if (!type)
		    panic("Unable to read type");
		auto index = decode_varuint<uint32_t>(stream);
		if (!index)
			panic("Unable to read export index");

		Export exp;
		exp.name = *name;
		exp.index = *index;
		switch (*type) {
			case EXPORT_FUNC:
				exports.func.push(exp);
				break;
			case EXPORT_TABLE:
				exports.table.push(exp);
				break;
			case EXPORT_MEM:
				exports.mem.push(exp);
				break;
			case EXPORT_GLOBAL:
				exports.global.push(exp);
				break;
			default:
				panic("Unrecognized export type %d",
						*type);
		}
	}
}

void Module::parse_code_section() {
	auto num_functions = stream_read<uint8_t>(stream);
	for (int i = 0; i < num_functions; i++) {
		Code code;

		auto size = decode_varuint<uint32_t>(stream);
		if (!size)
			panic("Unable to read function size");
		code.size = *size;

		auto num_locals = stream_read<uint8_t>(stream);
		for (int j = 0; j < num_locals;) {
			auto count = decode_varuint<uint32_t>(stream);
			if (!count)
				panic("Unable to read local count");
			auto type = stream_read<BinaryType>(stream);
			for (size_t k = 0; k < *count; k++)
				code.locals.push(*type);
			j += *count;
		}

		auto start_pos = stream->tell();
		auto instructions = Interpreter::decode_code(stream);

		code.size = stream->tell() - start_pos;
		code.expression = instructions;

		function_code.push(code);		
	}
}

void Module::parse_data_section() {
	auto num_entries = decode_varuint<uint32_t>(stream);
	if (!num_entries)
		panic("Error reading number of data entries");
	for (size_t i = 0; i < *num_entries; i++) {
		DataEntry entry;
		auto memidx = decode_varuint<uint32_t>(stream);
		if (!memidx)
			panic("Error reading memidx");
		auto offset = Interpreter::interpret_offset(stream);
		if (!offset)
			panic("Error reading offset");

		entry.memidx = *memidx;
		entry.offset = *offset;

		auto num_bytes = decode_varuint<uint32_t>(stream);
		if (!num_bytes)
			panic("Error reading size of bytes");
		for (size_t j = 0; j < *num_bytes; j++) {
			auto byte = stream_read<uint8_t>(stream);
			entry.bytes.push(*byte);
		}

		data.push(entry);
	}
}

void Module::parse_custom_section(int length) {
	auto name = read_string(stream);
	if (!name)
		panic("Error reading name of custom section");
	if (name == "name") {
		/* name section */
		/* TODO: parse names of other types besides functions */
		auto type = decode_varuint<uint8_t>(stream);
		if (!type) panic("Error reading names type");
		auto length = decode_varuint<uint32_t>(stream);
		if (!length) panic("Error reading names length");
		auto num_names = decode_varuint<uint32_t>(stream);
		if (!num_names) panic("Error reading number of names");

		for (int i = 0; i < num_names; i++) {
			auto name_index = decode_varuint<uint32_t>(stream);
			auto name = read_string(stream);
			if (!name) panic("Error reading function name");
			function_names.insert(*name_index, *name);
		}
	} else {
		log_warn("Encountered unknown custom section %s\n",
			(*name).data());
		stream->seek(length - (*name).size(), DataStream::BWASM_SEEK_CUR);
	}
}

void Module::parse_import_section() {
	auto num_entries = decode_varuint<uint32_t>(stream);
	if (!num_entries)
		panic("Error reading num entries of import");

	for (int i = 0; i < num_entries; i++) {
		Import import;

		auto module = read_string(stream);
		if (!module) panic("error reading import module!");
		import.module = *module;

		auto name = read_string(stream);
		if (!name) panic("error reading import name!");
		import.name = *name;

		auto description = stream_read<uint8_t>(stream);
		if (!description) panic("error reading import desc!");
		import.description = *description;

		auto idx = decode_varuint<uint32_t>(stream);
		if (!idx) panic ("error reading import idx");
		import.idx = *idx;

		imports.push(import);
	}
}

static frg::string<frg_allocator> type_to_string(BinaryType type) {
	switch (type) {
		case EMPTY: return "empty";
		case I_32: return "i32";
		case I_64: return "i64";
		case F_32: return "f32";
		case F_64: return "f64";
	}
	return frg::string<frg_allocator>();
}

void Module::dump_function_types() {
	//log_info("Types:\n");
	int i = 0;
	for (auto &function_type : function_types) {
		//log_info("\t[%d] (", i);
		for (auto parameter : function_type.parameters) {
			//log_info("%s ", type_to_string(parameter));
		}
		//std::cout << ") -> ";
		for (auto result : function_type.results)
			//std::cout << type_to_string(result) << " ";
		//std::cout << std::endl;
		i++;
	}
}

void Module::dump_functions() {
	//std::cout << "Functions:" << std::endl;
	for (size_t i = 0; i < functions.size(); i++) {
		auto type_idx = functions[i];
		//std::cout << "\t[" << i << "] typeidx " << type_idx
			//<< std::endl;
	}
}

static frg::string<frg_allocator> table_type_to_string(TableType type) {
	switch (type) {
		case TABLE_FUNCREF: return "funcref";
		default: return "unkown";
	}
}

void Module::dump_tables() {
	//std::cout << "Tables:" << std::endl;
	for (size_t i = 0; i < tables.size(); i++) {
		auto &table = tables[i];
		//std::cout << "\t[" << i << "] type: " <<
			//table_type_to_string(table.type)
			//<< " min: " << table.limit.first << " max: "
			//<< table.limit.second << std::endl;
	}
}

void Module::dump_memory() {
	//std::cout << "Memory:" << std::endl;
	int i = 0;
	for (auto &type : memory_types) {
		//std::cout << "\t[" << i << "] min: " <<
			//type.first << " max: " << type.second
			//<< std::endl;
		i++;
	}
}

void Module::dump_globals() {
	//std::cout << "Globals:" << std::endl;
	int i = 0;
	for (auto &global : globals) {
		//std::cout << "\t[" << i << "] value: ";
		//std::cout << " mut: " << global.mut << std::endl;
		i++;
	}
}

void Module::dump_exports() {
	//std::cout << "Exports" << std::endl;
	for (auto &exp : exports.func) {
		//std::cout << "\t func[" << exp.index << "] name: "
			//<< exp.name << std::endl;
	}
	for (auto &exp : exports.table) {
		//std::cout << "\t table[" << exp.index << "] name: "
			//<< exp.name << std::endl;
	}
	for (auto &exp : exports.mem) {
		//std::cout << "\t mem[" << exp.index << "] name: "
			//<< exp.name << std::endl;
	}
	for (auto &exp : exports.global) {
		//std::cout << "\t global[" << exp.index << "] name: "
			//<< exp.name << std::endl;
	}
}

void Module::dump_code() {
	//std::cout << "Code:" << std::endl;
	int i = 0;
	for (auto &code : function_code) {
		//std::cout << "\t[" << i << "] size: "
			//<< code.size << " locals: ";
		//for (auto &local : code.locals)
			//std::cout << type_to_string(local) << " ";
		//std::cout << std::endl;
		i++;
	}
}

void Module::dump_imports() {
	//std::cout << "Imports:" << std::endl;
	int i = 0;
	for (auto &import : imports) {
		//std::cout << "\t[" << i << "] name: "
			//<< import.module << "." <<
			//import.name << " type: " <<
			//import.description << std::endl;
		i++;
	}
}

} /* namespace bearwasm */
