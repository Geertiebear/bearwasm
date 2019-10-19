#include <bearwasm/Interpreter.h>

namespace bearwasm {

static void pop_args(InterpreterState &state, int idx) {
	auto &next = state.functions[idx];
	const auto &params = next.signature.parameters;
	for (size_t i = params.size(); i > 0; i--) {
		const auto param = params[i - 1];
		switch (param) {
			case I_32: {
				auto value = state.stack.pop<
					int32_t>();
				next.locals[i - 1].value = value;
				break;
			}
			case I_64: {
				auto value = state.stack.pop<
					int64_t>();
				next.locals[i - 1].value = value;
				break;
			}
			case F_32: {
				auto value = state.stack.pop<
					float>();
				next.locals[i - 1].value = value;
				break;
			}
			case F_64: {
				auto value = state.stack.pop<
					double>();
				next.locals[i - 1].value = value;
				break;
			}
			default:
				   panic("Unkown function arg");
		}
	}
}

static void invoke_function(InterpreterState &state, int idx,
		const Expression *expression) {
	pop_args(state, idx);

	Frame frame;
	frame.pc = state.pc + 1;
	frame.prev = state.current_function;
	frame.prev_expr = expression;
	frame.labelstack_size = state.labelstack.size();
	state.callstack.push(frame);

	state.current_function = idx;
	state.pc = 0;
	const auto num_params = state.functions[idx].signature.
		parameters.size();
	const auto num_locals = state.functions[idx].locals.size();
	for (auto i = num_params; i < num_locals; i++)
		state.functions[idx].locals[i].value = 0;
}

bool Interpreter::interpret(InterpreterState &state) {
	auto &current_function = state.current_function;
	auto &pc = state.pc;
	auto &functions = state.functions;
	auto &stack = state.stack;
	auto &memory = state.memory[0];
	const Expression *expression = &functions[current_function]
		.expression;

	while (pc < expression->size()) {
		std::cout << "pc " << pc << std::endl;
		std::cout << "current_function: " << current_function << std::endl;
		const auto &instruction = (*expression)[pc];
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
			case GLOBAL_GET: {
				auto idx = std::get<uint32_t>(instruction.arg);
				stack.push(state.globals[idx].value);
				break;
			}
			case GLOBAL_SET: {
				auto idx = std::get<uint32_t>(instruction.arg);
				auto type = state.globals[idx].type;
				state.globals[idx].value = stack.
					pop_variant(type);
				break;
			}
			case LOCAL_SET: {
				auto idx = std::get<uint32_t>(instruction.arg);
				auto type = state.functions[current_function].
					locals[idx].type;
				state.functions[current_function].locals[idx].
					value = stack.pop_variant(type);
				break;
			}
			case LOCAL_GET: {
				auto idx = std::get<uint32_t>(instruction.arg);
				stack.push(state.functions[current_function].
						locals[idx].value);
				break;
			}
			case LOCAL_TEE: {
				auto idx = std::get<uint32_t>(instruction.arg);
				auto type = state.functions[current_function].
					locals[idx].type;
				auto value = stack.pop_variant(type);
				stack.push(value);
				state.functions[current_function].locals[idx].
					value = value;
				break;
			}
			case I_32_EQZ: {
				auto arg = stack.pop<int32_t>();
				stack.push<int32_t>(arg == 0);
				break;
			}
			case I_32_EQ: {
				auto arg2 = stack.pop<int32_t>();
				auto arg1 = stack.pop<int32_t>();
				stack.push<int32_t>(arg1 == arg2);
				break;
			}
			case I_32_NE: {
				auto arg2 = stack.pop<int32_t>();
				auto arg1 = stack.pop<int32_t>();
				stack.push<int32_t>(arg1 != arg2);
				break;
			}
			case I_32_LT_S: {
				auto arg2 = stack.pop<int32_t>();
				auto arg1 = stack.pop<int32_t>();
				stack.push<int32_t>(arg1 < arg2);
				break;
			}
			case I_32_LT_U: {
				auto arg2 = stack.pop<uint32_t>();
				auto arg1 = stack.pop<uint32_t>();
				stack.push<int32_t>(arg1 < arg2);
				break;
			}
			case I_32_GT_S: {
				auto arg2 = stack.pop<int32_t>();
				auto arg1 = stack.pop<int32_t>();
				stack.push<int32_t>(arg1 > arg2);
				break;
			}
			case I_32_GT_U: {
				auto arg2 = stack.pop<uint32_t>();
				auto arg1 = stack.pop<uint32_t>();
				stack.push<int32_t>(arg1 > arg2);
				break;
			}
			case I_32_LE_S: {
				auto arg2 = stack.pop<int32_t>();
				auto arg1 = stack.pop<int32_t>();
				stack.push<int32_t>(arg1 <= arg2);
				break;
			}
			case I_32_LE_U: {
				auto arg2 = stack.pop<uint32_t>();
				auto arg1 = stack.pop<uint32_t>();
				stack.push<int32_t>(arg1 <= arg2);
				break;
			}
			case I_32_ADD: {
				auto arg2 = stack.pop<int32_t>();
				auto arg1 = stack.pop<int32_t>();
				std::cout << "Adding " << arg2 << " " << arg1 << std::endl;
				stack.push<int32_t>(arg1 + arg2);
				break;
			}
			case I_32_SUB: {
				auto arg2 = stack.pop<int32_t>();
				auto arg1 = stack.pop<int32_t>();
				//TODO: spec says to mod 2^32
				stack.push<int32_t>((arg1 - arg2));
				break;
			}
			case I_32_MUL: {
				auto arg2 = stack.pop<int32_t>();
				auto arg1 = stack.pop<int32_t>();
				stack.push<int32_t>(arg1 * arg2);
				break;
			}
			case I_32_AND: {
				auto arg2 = stack.pop<int32_t>();
				auto arg1 = stack.pop<int32_t>();
				stack.push<int32_t>(arg1 & arg2);
				break;
			}
			case I_32_OR: {
				auto arg2 = stack.pop<int32_t>();
				auto arg1 = stack.pop<int32_t>();
				stack.push<int32_t>(arg1 | arg2);
				break;
			}
			case I_32_SHL: {
				auto arg2 = stack.pop<int32_t>();
				auto arg1 = stack.pop<int32_t>();
				stack.push<int32_t>(arg1 << arg2);
				break;
			}
			case I_32_SHR_S: {
				auto arg2 = stack.pop<int32_t>();
				auto arg1 = stack.pop<int32_t>();
				stack.push<int32_t>(arg1 >> arg2);
				break;
			}
			case I_32_DIV_S: {
				auto arg2 = stack.pop<int32_t>();
				auto arg1 = stack.pop<int32_t>();
				stack.push<int32_t>(arg1 / arg2);
				break;
			}
			case I_32_REM_S: {
				auto arg2 = stack.pop<int32_t>();
				auto arg1 = stack.pop<int32_t>();
				stack.push<int32_t>(arg1 % arg2);
				break;
			}
			case I_32_STORE: {
				auto memarg = std::get<MemArg>(instruction.arg);
				auto t = stack.pop<int32_t>();
				auto i = stack.pop<int32_t>();
				auto limit = static_cast<int>(
						i + memarg.offset + 4);
				if (limit > memory.get_size())
					panic("Reading too far!");
				std::cout << "Storing at " << i + memarg.offset << std::endl;
				memory.store<int32_t>(t, i + memarg.offset);
				break;
			}
			case I_32_LOAD: {
				auto memarg = std::get<MemArg>(instruction.arg);
				auto i = stack.pop<int32_t>();
				auto limit = static_cast<int>(i +
						memarg.offset + 4);
				if (limit > memory.get_size())
					panic("Reading too far!");
				std::cout << "reading from " << i + memarg.offset << std::endl;
				auto result = memory.load<int32_t>(i + memarg.offset);
				stack.push<int32_t>(result);
				break;
			}
			case I_32_LOAD_8_U: {
				auto memarg = std::get<MemArg>(instruction.arg);
				auto i = stack.pop<int32_t>();
				auto limit = static_cast<int>(i +
						memarg.offset + 4);
				if (limit > memory.get_size())
					panic("Reading too far!");
				std::cout << "reading from " << i + memarg.offset << std::endl;
				auto result = memory.load<uint8_t>(i + memarg.offset);
				stack.push<int32_t>(result);
				break;
			}
			case I_32_LOAD_8_S: {
				auto memarg = std::get<MemArg>(instruction.arg);
				auto i = stack.pop<int32_t>();
				auto limit = static_cast<int>(i +
						memarg.offset + 4);
				if (limit > memory.get_size())
					panic("Reading too far!");
				std::cout << "reading from " << i + memarg.offset << std::endl;
				auto result = memory.load<int8_t>(i + memarg.offset);
				stack.push<int32_t>(result);
				break;
			}
			case INSTR_CALL: {
				auto idx = std::get<uint32_t>(instruction.arg);
				invoke_function(state, idx, expression);
				expression = &functions[current_function].expression;
				continue;
			}
			case INSTR_RETURN: {
				auto frame = state.callstack.top();
				state.callstack.pop();
				if (frame.pc == PC_END) return true;
				state.pc = frame.pc;
				state.current_function = frame.prev;
				expression = frame.prev_expr;

				//restore labelstack
				const auto size = state.labelstack.size();
				for (size_t i = frame.labelstack_size; i < size; i++)
					state.labelstack.pop();
				continue;
			}
			case INSTR_BLOCK: {
				auto &arg = std::get<Block>(instruction.arg);

				Label label;
				label.prev = expression;
				label.pc_cont = pc + 1;
				label.pc_end = pc + 1;
				std::cout << "pushing label with end " << label.pc_end << std::endl;
				state.labelstack.push(label);

				state.pc = 0;
				expression = &arg.expression;
				continue;
			}
			case INSTR_LOOP: {
				auto &arg = std::get<Block>(instruction.arg);

				Label label;
				label.prev = expression;
				label.pc_cont = pc;
				label.pc_end = pc + 1;
				std::cout << "pushing label with end " << label.pc_end << std::endl;
				state.labelstack.push(label);

				state.pc = 0;
				expression = &arg.expression;
				continue;
			}
			case BR: {
				auto idx = std::get<uint32_t>(instruction.arg);
				for (unsigned int i = 0; i < idx; i++)
					state.labelstack.pop();
				auto &label = state.labelstack.top();
				state.labelstack.pop();
				expression = label.prev;
				std::cout << "breaking to pc " << label.pc_cont << std::endl;
				state.pc = label.pc_cont;
				continue;
			}
			case BR_IF: {
				auto c = stack.pop<int32_t>();
				if (!c)
					break;
				auto idx = std::get<uint32_t>(instruction.arg);
				for (unsigned int i = 0; i < idx; i++)
					state.labelstack.pop();
				auto &label = state.labelstack.top();
				state.labelstack.pop();
				expression = label.prev;
				std::cout << "breaking to pc " << label.pc_cont << std::endl;
				state.pc = label.pc_cont;
				continue;
			}
			case INSTR_DROP: {
				stack.drop();
				break;
			}
			case INSTR_SELECT: {
				auto c = stack.pop<int32_t>();
				auto val2 = stack.pop<int32_t>();
				auto val1 = stack.pop<int32_t>();
				if (c) stack.push<int32_t>(val1);
				else stack.push<int32_t>(val2);
				break;
			}
			case INSTR_END: {
				if (state.labelstack.empty())
					return true;
				auto label = state.labelstack.top();
				state.labelstack.pop();
				expression = label.prev;
				state.pc = label.pc_end;
				continue;
			}
			default:
				panic("Unknown instruction encountered " +
						std::to_string((*expression)[pc].type));
		};
		pc++;
	}
	return true;
}

std::optional<GlobalValue> Interpreter::interpret_global(
		std::ifstream &stream) {
	GlobalValue ret;
	auto type = static_cast<BinaryType>(stream.get());
	ret.type = type;
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
		case EMPTY:
			break;
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

std::optional<uint32_t> Interpreter::interpret_offset(
		std::ifstream &stream) {
	InterpreterState state;
	state.functions.resize(1);
	/* TODO: write a span so we can avoid copies */
	auto start_pos = stream.tellg();
	state.functions[0].expression = decode_code(stream);
	state.functions[0].size = stream.tellg() - start_pos;
	state.current_function = 0;
	if(!interpret(state)) return std::nullopt;

	return state.stack.pop<uint32_t>();
}

std::vector<Instruction> Interpreter::decode_code(std::ifstream &stream) {
	std::vector<Instruction> ret;

	auto instruction = stream_read<Instructions>(stream);
	if (!instruction)
		panic("Unable to read instruction");
	while (true) {
		Instruction inst;

		if (*instruction == INSTR_END) {
			inst.type = *instruction;
			ret.push_back(inst);
			return ret;
		}

		auto arg_size = instruction_sizes.find(*instruction);
		if (arg_size == instruction_sizes.end())
			panic("Don't know size of instruction " + 
				std::to_string(*instruction));

		switch (arg_size->second) {
			case SIZE_BLOCK: {
				Block block;
				auto type = stream_read<BinaryType>(stream);
				if (!type)
					panic("Unable to read block type");
				block.type = *type;

				auto instructions = decode_code(stream);
				block.expression = instructions;
				inst.arg = block;
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
			case SIZE_0: {
				inst.type = *instruction;
				break;
			}
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
