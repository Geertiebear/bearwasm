#ifndef BEARWASM_VM_H
#define BEARWASM_VM_H

#include <string>
#include <bearwasm/Interpreter.h>
#include <bearwasm/Module.h>

namespace bearwasm {

class VirtualMachine {
public:
	VirtualMachine(const std::string &path);
	int execute();
private:
	void build_function_instances();
	void build_memory_instances();

	InterpreterState state;
	Module module;
};

} /* namespace bearwasm */

#endif
