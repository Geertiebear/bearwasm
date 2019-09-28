#ifndef BEARWASM_INTERPRETER_H
#define BEARWASM_INTERPRETER_H

#include <optional>
#include <vector>
#include <fstream>
#include <bearwasm/Stack.h>
#include <bearwasm/Format.h>

namespace bearwasm {

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


struct InterpreterState {
	InterpreterState() : pc(0) {}
	std::vector<Code> functions;
	int current_function;
	int pc;
	Stack stack;
};

struct Instruction {
	Instructions type;
	InstructionArg arg;
};

class Interpreter {
public:
	bool interpret(InterpreterState &state);
	std::optional<GlobalValue> interpret_global(
			std::ifstream &stream);
	std::vector<Instruction> decode_code(std::ifstream &stream);
};

}/* namespace bearwasm*/

#endif
