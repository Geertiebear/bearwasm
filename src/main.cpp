#include <iostream>
#include <bearwasm/VirtualMachine.h>

int main(int argc, char **argv) {
	if (argc < 2) {
		std::cout << "Please provide the path to a wasm binary" << std::endl;
		return 0;
	}

	bearwasm::VirtualMachine vm{std::string(argv[1])};
	vm.execute(argc - 2, argv + 2);
	return 0;
}
