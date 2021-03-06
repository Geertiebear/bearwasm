#include <bearwasm/VirtualMachine.h>
#include <string.h>

namespace bearwasm {

VirtualMachine::VirtualMachine(DataStream *stream) :
	module(stream), handlers(frg::hash<frg::string<
		       frg_allocator>>{})	{

	asm_state = new ASMInterpreterState;
}

void VirtualMachine::init() {
	build_import_instances();
	build_function_instances();
	build_memory_instances();
	build_data_instances();
	state.globals = module.globals;

	Frame frame;
	frame.pc = PC_END;
	frame.prev = 0;
	frame.prev_expr = 0;
	frame.labelstack_size = 0;
	state.callstack.push(frame);
}

void VirtualMachine::register_handler(const frg::string<frg_allocator> &name,
		NativeHandler handler) {
	handlers[name] = handler;
}

int VirtualMachine::execute(int argc, char **argv) {
	state.current_function = -1;
	for (const auto &func : module.exports.func)
		if (func.name == "main")
			state.current_function = func.index;
	if (state.current_function == -1)
		panic("Could not find main function!");

	//argc
	state.functions[state.current_function].locals[0].value.int32_val =
		static_cast<int32_t>(argc);
	state.functions[state.current_function].locals[0].type = I_32;
	//argv
	state.functions[state.current_function].locals[1].value.int32_val =
		static_cast<int32_t>(1);
	state.functions[state.current_function].locals[1].type = I_32;

	int offset = 0;
	for (int i = 0; i < argc; i++) {
		auto arg = argv[i];
		auto length = strlen(arg);
		uint32_t location = (5 + (argc * 4)  + offset);
		state.memory[0].copy(reinterpret_cast<char*>(&location), 4, 5 + (i * 4));
		state.memory[0].copy(arg, length, location);
		offset += length;
	}
	Interpreter::interpret(state);
	auto res = state.stack.top();
	return res.int32_val;
}

int VirtualMachine::execute_asm(int argc, char **argv) {
	asm_state->stack = new char[STACK_SIZE];
	asm_state->pc = 0;

	asm_state->expression_no = -1;
	for (const auto &func : module.exports.func)
		if (func.name == "main")
			asm_state->expression_no = func.index;
	if (asm_state->expression_no == -1)
		panic("Could not find main function!");

	asm_state->expressions = new uint8_t*[state.functions.size()];
	asm_state->locals = new uint64_t*[state.functions.size()];
	asm_state->expression_arg_no = new uint8_t[state.functions.size()];

	for (size_t i = 0; i < state.functions.size(); i++) {
		const auto &instance = state.functions[i];
		asm_state->expressions[i] = new uint8_t[instance.size];
		memcpy(asm_state->expressions[i], instance.expression.data(), instance.size);
		asm_state->expression_arg_no[i] = instance.signature.parameters.size();
		asm_state->locals[i] = new uint64_t[instance.locals.size()];
		for (size_t j = 0; j < instance.locals.size(); j++)
			asm_state->locals[i][j] = 0;
	}

	//argv
	asm_state->locals[asm_state->expression_no][0] =
		static_cast<uint64_t>(argc);
	//argc
	asm_state->locals[asm_state->expression_no][1] = 1;


	asm_state->memory_sizes = new int[1];
	asm_state->memory_sizes[0] = state.memory[0].get_size();
	asm_state->memories = new char*[1];
	asm_state->memories[0] = new char[asm_state->memory_sizes[0]];

	int offset = 0;
	for (int i = 0; i < argc; i++) {
		auto arg = argv[i];
		auto length = strlen(arg);
		auto location = (5 + (argc * 4)  + offset);
		memcpy((asm_state->memories[0] + 5 + (i * 4)),
				reinterpret_cast<char*>(&location), 4);
		memcpy(location + asm_state->memories[0], arg, length);
		offset += length;
	}

	asm_state->globals = new uint64_t[state.globals.size()];
	//return vm_enter(asm_state);
        return 1;
}

void VirtualMachine::build_function_instances() {
	for (size_t i = 0; i < module.function_code.size(); i++) {
		FunctionInstance instance;
		instance.type = FUNCTION_WASM;
		instance.expression = module.function_code[i].expression;
		instance.size = instance.expression.size() * sizeof(Instruction);
		instance.signature = module.function_types[module.functions[i]];
		auto name_it = module.function_names.find(i);
		if (name_it != module.function_names.end())
			instance.name = name_it->template get<1>();
		for (const auto param : instance.signature.parameters) {
			LocalInstance local_instance;
			local_instance.type = param;
			local_instance.value.int32_val = 0;
			instance.locals.push(local_instance);
		}
		for (auto &local : module.function_code[i].locals) {
			LocalInstance local_instance;
			local_instance.type = local;
			local_instance.value.int32_val = 0;
			instance.locals.push(local_instance);
		}
		state.functions.push(instance);
    }
}

void VirtualMachine::build_memory_instances() {
	for (auto &mem : module.memory_types)
		state.memory.emplace_back(mem.template get<0>());
}

void VirtualMachine::build_import_instances() {
	for (auto &import : module.imports) {
		switch(import.description) {
			case EXPORT_FUNC: {
				if (import.module == "env") {
					FunctionInstance instance;
					instance.type = FUNCTION_NATIVE;

					auto handler = handlers.find(import.name);
					if(handler == handlers.end())
						panic("could not resolve "
					   "native import %s",
							import.name.data());
					instance.native_handler =
						handler->template get<1>();

					state.functions.push(instance);
				}
				break;
			}
			default:
				panic("unkown import descriptor");
		}
	}
}

void VirtualMachine::build_data_instances() {
	for (auto &data : module.data) {
		state.memory[data.memidx].copy(
				reinterpret_cast<char*>(data.bytes.data()),
				data.bytes.size(), data.offset);
	}
}

} /* namespace bearwasm */
