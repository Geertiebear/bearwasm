#ifndef BEARWASM_VM_H
#define BEARWASM_VM_H

#include <frg/string.hpp>
#include <frg/hash.hpp>
#include <frg/hash_map.hpp>
#include <bearwasm/FriggAllocator.h>
#include <bearwasm/Interpreter.h>
#include <bearwasm/Module.h>

extern "C" int vm_enter(void *state);

namespace bearwasm {

class VirtualMachine {
public:
	VirtualMachine(DataStream *stream);
	void init();

	void register_handler(const frg::string<frg_allocator> &name,
            NativeHandler handler);

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
	frg::hash_map<frg::string<frg_allocator>,
        NativeHandler, frg::hash<frg::string<frg_allocator>>,
        frg_allocator> handlers;
};

} /* namespace bearwasm */

#endif
