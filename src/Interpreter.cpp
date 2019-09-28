#include <bearwasm/Interpreter.h>

namespace bearwasm {

bool Interpreter::interpret(InterpreterState &state) {
	auto &current_function = state.current_function;
	auto &pc = state.pc;
	auto &functions = state.functions;
	auto &stack = state.stack;
	auto &expression = functions[current_function].expression;

	while (pc < expression.size()) {
		const auto &instruction = expression[pc];
		switch (instruction.type) {
			case I_32_CONST: {
				stack.push(std::get<int32_t>(instruction.arg));
				break;
			}
			case I_64_CONST: {
				stack.push(std::get<int64_t>(instruction.arg));
				break;
			}
				/* TODO: floating point const instructions */
			default:
				panic("Unknown instruction encountered " +
						std::to_string(expression[pc].type));
		};
		pc++;
	}
	return true;
}

std::optional<GlobalValue> Interpreter::interpret_global(
		std::ifstream &stream) {
	GlobalValue ret;
	auto type = static_cast<BinaryType>(stream.get());
	ret.mut = static_cast<bool>(stream.get());

	InterpreterState state;
	state.functions.resize(1);
	/* TODO: write a span so we can avoid copies */
	auto start_pos = stream.tellg();
	state.functions[0].expression = decode_code(stream);
	state.functions[0].size = stream.tellg() - start_pos;
	state.current_function = 0;
	if(!interpret(state)) return std::nullopt;

	switch (type) {
		case I_32:
		       ret.value = state.stack.pop<int32_t>();
		       break;
		case I_64:
		       ret.value = state.stack.pop<int64_t>();
		       break;
		case F_32:
		       ret.value = state.stack.pop<float>();
		       break;
		case F_64:
		       ret.value = state.stack.pop<double>();
		       break;
	}
	return ret;
}

std::vector<Instruction> Interpreter::decode_code(std::ifstream &stream) {
	std::vector<Instruction> ret;

	auto instruction = stream_read<Instructions>(stream);
	if (!instruction)
		panic("Unable to read instruction");
	while (*instruction != 0x0B) {
		Instruction inst;

		auto arg_size = instruction_sizes.find(*instruction);
		if (arg_size == instruction_sizes.end())
			panic("Don't know size of instruction " + 
				std::to_string(*instruction));

		switch (arg_size->second) {
			case SIZE_VARIABLE: {
				auto instructions = decode_code(stream);
				inst.arg = instructions;
				inst.type = *instruction;
				break;
			} 
			case SIZE_U8: {
				auto value = stream_read<uint8_t>(stream);
				if (!value)
					panic("Unable to read value");
				inst.arg = *value;
				inst.type = *instruction;
				break;
			}
			case SIZE_I32: {
				auto value = decode_varint_s<int32_t>(stream);
				if (!value)
					panic("Unable to read value");
				inst.arg = *value;
				inst.type = *instruction;
				break;
			}
			case SIZE_I64: {
				auto value = decode_varint_s<int64_t>(stream);
				if (!value)
					panic("Unable to read value");
				inst.arg = *value;
				inst.type = *instruction;
				break;
			}
			case SIZE_U32: {
				auto value = decode_varuint_s<uint32_t>(stream);
				if (!value)
					panic("Unable to read value");
				inst.arg = *value;
				inst.type = *instruction;
				break;
			}
			case SIZE_U64: {
				auto value = decode_varuint_s<uint64_t>(stream);
				if (!value)
					panic("Unable to read value");
				inst.arg = *value;
				inst.type = *instruction;
				break;
			}
			case SIZE_0: break;
			case SIZE_MEMARG: {
				auto align = decode_varuint_s<uint32_t>(stream);
				auto offset = decode_varuint_s<uint32_t>(stream);
				if (!align || !offset)
					panic("Unable to read value");
				MemArg arg;
				arg.align = *align;
				arg.offset = *offset;

				inst.arg = arg;
				inst.type = *instruction;
				break;
			}
			default:
				panic("Unable to handle size");
		}
		ret.push_back(inst);

		instruction = stream_read<Instructions>(stream);
	}

	return ret;
}

} /* namespace bearwasm */
