#include <iostream>
#include <stdio.h>
#include <bearwasm/VirtualMachine.h>

static int print(bearwasm::InterpreterState *state) {
	int ptr = state->stack.pop<int32_t>();
	char *str = reinterpret_cast<char*>(
			state->memory[0].data() + ptr);
	return printf(str);
}

int main(int argc, char **argv) {
	if (argc < 2) {
		std::cout << "Please provide the path to a wasm binary" << std::endl;
		return 0;
	}

	bearwasm::VirtualMachine vm{std::string(argv[1])};
	vm.register_handler("print", &print);
	vm.init();
	auto res = vm.execute(argc - 2, argv + 2);
	std::cout << "Program exit code: " << res << std::endl;
	return 0;
}
