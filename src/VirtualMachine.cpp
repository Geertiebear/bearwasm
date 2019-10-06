#include <bearwasm/VirtualMachine.h>

namespace bearwasm {

VirtualMachine::VirtualMachine(const std::string &path) :
       module(path) {

	build_function_instances();
	build_memory_instances();
	state.current_function = 2;
	state.globals = module.globals;
	//argc
	state.functions[2].locals[0].value = static_cast<int32_t>(2);
	//argv
	state.functions[2].locals[1].value = static_cast<int32_t>(1);
	
	Frame frame;
	frame.pc = PC_END;
	frame.prev = 0;
	state.callstack.push(frame);

	char argv[] = "\xD\x0\x0\x0\xE\x0\x0\x0\x33\x34";
	state.memory[0].copy(argv, sizeof(argv), 5);
}

int VirtualMachine::execute() {
	Interpreter::interpret(state);
	return 0;
}

void VirtualMachine::build_function_instances() {
	for (size_t i = 0; i < module.function_code.size(); i++) {
		FunctionInstance instance;
		instance.expression = module.function_code[i].expression;
		instance.size = module.function_code[i].size;
		instance.signature = module.function_types[module.functions[i]];
		for (const auto param : instance.signature.parameters) {
			LocalInstance local_instance;
			local_instance.type = param;
			local_instance.value = 0;
			instance.locals.push_back(local_instance);
		}
		for (auto &local : module.function_code[i].locals) {
			LocalInstance local_instance;
			local_instance.type = local;
			local_instance.value = 0;
			instance.locals.push_back(local_instance);
		}
		/* TODO: function name */
		state.functions.push_back(instance);
	}
}

void VirtualMachine::build_memory_instances() {
	for (auto &mem : module.memory_types)
		state.memory.emplace_back(mem.first);
}

} /* namespace bearwasm */
