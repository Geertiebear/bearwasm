#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <bearwasm/VirtualMachine.h>
#include <bearwasm/host.hpp>

void *frg_allocator::allocate(size_t size) {
	return malloc(size);
}

void frg_allocator::free(void *p) {
	if (p)
            ::free(p);
}

void frg_allocator::deallocate(void *p, size_t n) {
        (void) n;
        ::free(p);
}

class FileStream : public bearwasm::DataStream {
public:
	FileStream(const std::string &path) : stream(path) {
	}

	frg::optional<char> get() {
		char c = stream.get();
		if (c == EOF) return frg::null_opt;
		return c;
	}

	bool read(char *buf, size_t size) {
		return !(stream.read(buf, size).fail());
	}

	int seek(long pos, SeekType type) {
		bool success = false;
		switch (type) {
			case BWASM_SEEK_SET:
				success = !(stream.seekg(pos).fail());
			case BWASM_SEEK_CUR:
				success = !(stream.seekg(pos,
							std::ios::cur).fail());
			case BWASM_SEEK_END:
				success =!(stream.seekg(pos,
							std::ios::end).fail());
		}
		return success ? 0 : -1;
	}

	long tell() {
		return stream.tellg();
	}
private:
	std::ifstream stream;
};

static int print(bearwasm::InterpreterState *state) {
	auto val = state->stack.top();
	state->stack.pop();
	int ptr = val.int32_val;
	char *str = reinterpret_cast<char*>(
			state->memory[0].data() + ptr);
	return printf(str);
}

void bearwasm_abort() {
	exit(1);
}

void bearwasm_log(int level, const char *str) {
	(void)level;
	printf("%s", str);
}

int main(int argc, char **argv) {
	if (argc < 2) {
		std::cout << "Please provide the path to a wasm binary" << std::endl;
		return 0;
	}

	FileStream stream{std::string(argv[1])};
	bearwasm::VirtualMachine vm{&stream};
	vm.register_handler("print", &print);
	vm.init();
	std::cout << "Starting to execute program" << std::endl;
	auto res = vm.execute(argc - 2, argv + 2);
	std::cout << "Program exit code: " << res << std::endl;
	return 0;
}
