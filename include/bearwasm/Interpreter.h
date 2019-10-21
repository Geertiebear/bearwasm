#ifndef BEARWASM_INTERPRETER_H
#define BEARWASM_INTERPRETER_H

#include <optional>
#include <vector>
#include <fstream>
#include <stack>
#include <memory>
#include <bearwasm/Stack.h>
#include <bearwasm/Format.h>

namespace bearwasm {

static constexpr int PC_END = -1;
static constexpr int STACK_SIZE = 0x400000;

struct Instruction;

struct MemArg {
	uint32_t align, offset;
};

/* TODO: change expression to a pointer somehow
 * to avoid copies when loading the module */
struct Block {
	BinaryType type;
	Expression expression;
};

struct LocalInstance {
	Local type;
	Value value;
};

struct FunctionInstance {
	FunctionType signature;
	Expression expression;
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
	int labelstack_size;
	int prev;
	const Expression *prev_expr;
};

struct Label {
	const Expression *prev;
	int pc_cont, pc_end;
};

struct InterpreterState {
	InterpreterState() : stack(STACK_SIZE), pc(0)
	{}
	std::vector<FunctionInstance> functions;
	std::vector<MemoryInstance> memory;
	std::vector<TableInstance> tables;
	std::vector<GlobalValue> globals;
	Stack stack;
	std::stack<Frame> callstack;
	std::stack<Label> labelstack;

	int current_function;
	size_t pc;
};

struct Instruction {
	Instructions type;
	union {
		uint8_t uint8_val;
		uint32_t uint32_val;
		uint64_t uint64_val;
		int32_t int32_val;
		int64_t int64_val;
		float float_val;
		double double_val;
		Block *block;
		MemArg memarg;
	} arg;
};

class Interpreter {
public:
	static bool interpret(InterpreterState &state);
	static std::optional<GlobalValue> interpret_global(
			std::ifstream &stream);
	static std::optional<uint32_t> interpret_offset(
			std::ifstream &stream);
	static std::vector<Instruction> decode_code(std::ifstream &stream);
};

}/* namespace bearwasm*/

#endif
