#ifndef BEARWASM_INTERPRETER_H
#define BEARWASM_INTERPRETER_H

#include <frg/vector.hpp>
#include <frg/stack.hpp>
#include <frg/optional.hpp>
#include <frg/string.hpp>
#include <bearwasm/FriggAllocator.h>
#include <bearwasm/Stack.h>
#include <bearwasm/Format.h>

namespace bearwasm {

static constexpr int PC_END = -1;
static constexpr int STACK_SIZE = 0x400000;

struct InterpreterState;
struct Instruction;

using NativeHandler = int (*)(InterpreterState *state);

struct MemArg {
	uint32_t align, offset;
};

struct Block {
	unsigned int size;
	BinaryType type;
};

struct LocalInstance {
	Local type;
	Value value;
};

enum InstanceType {
	FUNCTION_WASM, //function in wasm
	FUNCTION_NATIVE, //function from C
};

struct FunctionInstance {
	InstanceType type;
	NativeHandler native_handler;
	FunctionType signature;
	Expression expression;
	frg::vector<LocalInstance, frg_allocator> locals;
	frg::string<frg_allocator> name;
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

	char *data() {
		return bytes.data();
	}
private:
	int size;
	frg::vector<char, frg_allocator> bytes;
};

struct TableInstance {
	int max;
	frg::vector<int, frg_allocator> function_address;
};

struct Frame {
	int pc;
	int labelstack_size;
	int prev;
	const Expression *prev_expr;
};

struct Label {
	int pc_cont, pc_end;
};

struct InterpreterState {
	InterpreterState() : stack(STACK_SIZE), pc(0) {}
	frg::vector<FunctionInstance, frg_allocator> functions;
	frg::vector<MemoryInstance, frg_allocator> memory;
	frg::vector<TableInstance, frg_allocator> tables;
	frg::vector<GlobalValue, frg_allocator> globals;
	Stack stack;
	frg::stack<Frame, frg_allocator> callstack;
	frg::stack<Label, frg_allocator> labelstack;

	int current_function;
	size_t pc;
};

struct ASMInterpreterState {
	char *stack;
	uint32_t pc;

	int32_t expression_no;
	uint8_t **expressions;
	uint64_t **locals;

	char **memories;
	int *memory_sizes;

	uint64_t *globals;
	uint8_t *expression_arg_no;
};

struct Instruction {
	/* instructions are only a byte but
	 * this would make the struct unaligned
	 * since the size of the union is 8 bytes
	 * so we make the struct 16 bytes long
	 * to keep it aligned */
	uint64_t type;
	union {
		uint8_t uint8_val;
		uint32_t uint32_val;
		uint64_t uint64_val;
		int32_t int32_val;
		int64_t int64_val;
		float float_val;
		double double_val;
		Block block;
		MemArg memarg;
	} arg;
};

class Interpreter {
public:
	static bool interpret(InterpreterState &state);
	static frg::optional<GlobalValue> interpret_global(
			DataStream *stream);
	static frg::optional<uint32_t> interpret_offset(
			DataStream *stream);
	static frg::vector<Instruction, frg_allocator> decode_code(DataStream
            *stream);
};

}/* namespace bearwasm*/

#endif
