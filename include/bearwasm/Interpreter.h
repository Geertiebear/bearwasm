#ifndef BEARWASM_INTERPRETER_H
#define BEARWASM_INTERPRETER_H

#include <optional>
#include <vector>
#include <fstream>
#include <stack>
#include <bearwasm/Stack.h>
#include <bearwasm/Format.h>

namespace bearwasm {

static constexpr int PC_END = -1;

struct Instruction;

struct MemArg {
	uint32_t align, offset;
};

using InstructionArg = std::variant<
	uint8_t,
	uint32_t,
	uint64_t,
	int32_t,
	int64_t,
	float,
	double,
	std::vector<Instruction>,
	MemArg>;

struct LocalInstance {
	Local type;
	Value value;
};

struct FunctionInstance {
	FunctionType signature;
	std::vector<Instruction> expression;
	std::vector<LocalInstance> locals;
	std::string name;
	int size;
};

class MemoryInstance {
public:
	MemoryInstance(int size) {
		resize(size);
	}

	void resize(int new_size) {
		bytes.resize(new_size * PAGE_SIZE);
		size = new_size * PAGE_SIZE;
	}

	void copy(char *data, int num, int pos) {
		for (int i = 0; i < num; i++)
			bytes[pos + i] = data[i];
	}

	int get_size() const {
		return size;
	}

	template<typename T>
	void store(T value, int pos) {
		auto data = reinterpret_cast<char*>(&value);
		for (size_t i = 0; i < sizeof(T); i++)
			bytes[pos + i] = data[i];
	}

	template<typename T>
	T load(int pos) {
		T ret;
		auto data = reinterpret_cast<char*>(&ret);
		for (size_t i = 0; i < sizeof(T); i++)
			data[i] = bytes[pos + i];
		return ret;
	}
private:
	int size;
	std::vector<char> bytes;
};

struct TableInstance {
	int max;
	std::vector<int> function_address;
};

struct Frame {
	int pc;
	int prev;
};

struct InterpreterState {
	InterpreterState() : pc(0) {}
	std::vector<FunctionInstance> functions;
	std::vector<MemoryInstance> memory;
	std::vector<TableInstance> tables;
	std::vector<GlobalValue> globals;
	Stack stack;
	std::stack<Frame> callstack;

	int current_function;
	size_t pc;
};

struct Instruction {
	Instructions type;
	InstructionArg arg;
};

class Interpreter {
public:
	static bool interpret(InterpreterState &state);
	static std::optional<GlobalValue> interpret_global(
			std::ifstream &stream);
	static std::vector<Instruction> decode_code(std::ifstream &stream);
};

}/* namespace bearwasm*/

#endif
