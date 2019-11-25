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
	int execute(int argc, char **argv);
	int execute_asm(int argc, char **argv);
private:
	void build_function_instances();
	void build_memory_instances();

	InterpreterState state;
	ASMInterpreterState *asm_state;
	Module module;
};

} /* namespace bearwasm */

#endif
