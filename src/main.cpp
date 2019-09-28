#include <iostream>
#include <bearwasm/Module.h>

int main(int argc, char **argv) {
	if (argc < 2) {
		std::cout << "Please provide the path to a wasm binary" << std::endl;
		return 0;
	}

	bearwasm::Module module((std::string(argv[1])));	
	return 0;
}
