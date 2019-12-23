#ifndef BEARWASM_VM_H
#define BEARWASM_VM_H

#include <string>
#include <bearwasm/Interpreter.h>
#include <bearwasm/Module.h>

extern "C" int vm_enter(void *state);

namespace bearwasm {

class VirtualMachine {
public:
	VirtualMachine(const std::string &path);
	void init();

	void register_handler(const std::string &name, NativeHandler handler);

	int execute(int argc, char **argv);
	int execute_asm(int argc, char **argv);
private:
	void build_import_instances();
	void build_function_instances();
	void build_memory_instances();
	void build_data_instances();

	InterpreterState state;
	ASMInterpreterState *asm_state;
	Module module;
	std::unordered_map<std::string, NativeHandler> handlers;
};

} /* namespace bearwasm */

#endif
