#include <bearwasm/Interpreter.h>

#include <algorithm>
#include <iterator>

namespace bearwasm {

static void pop_args(InterpreterState &state, int idx) {
	auto &next = state.functions[idx];
	const auto &params = next.signature.parameters;
	for (size_t i = params.size() - 1; i >= 0; i--) {
		auto value = state.stack.top();
		state.stack.pop();
		next.locals[i].value = value;
		next.locals[i].type = params[i];
	}
}

static void invoke_function(InterpreterState &state, int idx,
		const Expression *expression) {
	FunctionInstance &instance = state.functions[idx];
	if (instance.type == FUNCTION_NATIVE) {
		auto value = instance.native_handler(&state);
		state.stack.push(value);
		return;
	}

	pop_args(state, idx);

	Frame frame;
	frame.pc = state.pc;
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
		state.functions[idx].locals[i].value.int32_val = 0;
}

bool Interpreter::interpret(InterpreterState &state) {
	uint64_t num_instr = 0;
	auto &current_function = state.current_function;
	auto pc = state.pc;
	auto &functions = state.functions;
	auto &stack = state.stack;
	auto &memory = state.memory[0];
	const Expression *expression = &functions[current_function]
		.expression;

	static void *dispatch_table[255] = {};
	std::fill_n(dispatch_table, 255, &&instr_unknown);
	dispatch_table[I_32_CONST] = &&i_32_const;
	dispatch_table[I_64_CONST] = &&i_64_const;
	dispatch_table[GLOBAL_GET] = &&global_get;
	dispatch_table[GLOBAL_SET] = &&global_set;
	dispatch_table[LOCAL_SET] = &&local_set;
	dispatch_table[LOCAL_GET] = &&local_get;
	dispatch_table[LOCAL_TEE] = &&local_tee;
	dispatch_table[I_32_EQZ] = &&i_32_eqz;
	dispatch_table[I_32_EQ] = &&i_32_eq;
	dispatch_table[I_32_NE] = &&i_32_ne;
	dispatch_table[I_32_LT_S] = &&i_32_lt_s;
	dispatch_table[I_32_LT_U] = &&i_32_lt_u;
	dispatch_table[I_32_GT_S] = &&i_32_gt_s;
	dispatch_table[I_32_GT_U] = &&i_32_gt_u;
	dispatch_table[I_32_LE_S] = &&i_32_le_s;
	dispatch_table[I_32_LE_U] = &&i_32_le_u;
	dispatch_table[I_32_ADD] = &&i_32_add;
	dispatch_table[I_32_SUB] = &&i_32_sub;
	dispatch_table[I_32_MUL] = &&i_32_mul;
	dispatch_table[I_32_AND] = &&i_32_and;
	dispatch_table[I_32_OR] = &&i_32_or;
	dispatch_table[I_32_SHL] = &&i_32_shl;
	dispatch_table[I_32_SHR_S] = &&i_32_shr_s;
	dispatch_table[I_32_DIV_S] = &&i_32_div_s;
	dispatch_table[I_32_REM_S] = &&i_32_rem_s;
	dispatch_table[I_32_STORE] = &&i_32_store;
	dispatch_table[I_32_LOAD] = &&i_32_load;
	dispatch_table[I_32_LOAD_8_U] = &&i_32_load_8_u;
	dispatch_table[I_32_LOAD_8_S] = &&i_32_load_8_s;
	dispatch_table[INSTR_CALL] = &&instr_call;
	dispatch_table[INSTR_RETURN] = &&instr_return;
	dispatch_table[INSTR_BLOCK] = &&instr_block;
	dispatch_table[INSTR_LOOP] = &&instr_loop;
	dispatch_table[INSTR_IF] = &&instr_if;
	dispatch_table[BR] = &&br;
	dispatch_table[BR_IF] = &&br_if;
	dispatch_table[INSTR_DROP] = &&instr_drop;
	dispatch_table[INSTR_SELECT] = &&instr_select;
	dispatch_table[INSTR_END] = &&instr_end;
#define DISPATCH() log_debug("pc %d\n", pc); \
	instruction = (*expression)[pc++]; \
	num_instr++; \
    log_debug("instr %d\n", instruction.type); \
	goto *dispatch_table[instruction.type];

	while (pc < expression->size()) {
		num_instr++;
		Instruction instruction;
		DISPATCH();
		i_32_const: {
			stack.emplace(instruction.arg.int32_val);
			DISPATCH();
		}
		i_64_const: {
			stack.emplace(instruction.arg.int64_val);
			DISPATCH();
		}
		/* TODO: floating point const instructions */
		global_get: {
			auto idx = instruction.arg.uint32_val;
			stack.push(state.globals[idx].value);
			DISPATCH();
		}
		global_set: {
			auto idx = instruction.arg.uint32_val;
			state.globals[idx].value = stack.top();
			stack.pop();
			DISPATCH();
		}
		local_set: {
			auto idx = instruction.arg.uint32_val;
			state.functions[current_function].locals[idx].
				value = stack.top();
			stack.pop();
			DISPATCH();
		}
		local_get: {
			auto idx = instruction.arg.uint32_val;
			auto local = state.functions[current_function].
				locals[idx];
			stack.push(local.value);
			DISPATCH();
		}
		local_tee: {
			auto idx = instruction.arg.uint32_val;
			auto value = stack.top();
			state.functions[current_function].locals[idx].
				value = value;
			DISPATCH();
		}
		i_32_eqz: {
			auto arg = stack.top();
			stack.pop();
			stack.emplace(arg.int32_val == 0);
			DISPATCH();
		}
		i_32_eq: {
			auto arg2 = stack.top();
			stack.pop();
			auto arg1 = stack.top();
			stack.pop();
			stack.emplace(arg1.int32_val == arg2.int32_val);
			DISPATCH();
		}
		i_32_ne: {
			auto arg2 = stack.top();
			stack.pop();
			auto arg1 = stack.top();
			stack.pop();
			stack.emplace(arg1.int32_val != arg2.int32_val);
			DISPATCH();
		}
		i_32_lt_s: {
			auto arg2 = stack.top();
			stack.pop();
			auto arg1 = stack.top();
			stack.pop();
			stack.emplace(arg1.int32_val < arg2.int32_val);
			DISPATCH();
		}
		i_32_lt_u: {
			auto arg2 = stack.top();
			stack.pop();
			auto arg1 = stack.top();
			stack.pop();
			stack.emplace(arg1.uint32_val < arg2.uint32_val);
			DISPATCH();
		}
		i_32_gt_s: {
			auto arg2 = stack.top();
			stack.pop();
			auto arg1 = stack.top();
			stack.pop();
			stack.emplace(arg1.int32_val > arg2.int32_val);
			DISPATCH();
		}
		i_32_gt_u: {
			auto arg2 = stack.top();
			stack.pop();
			auto arg1 = stack.top();
			stack.pop();
			stack.emplace(arg1.uint32_val > arg2.uint32_val);
			DISPATCH();
		}
		i_32_le_s: {
			auto arg2 = stack.top();
			stack.pop();
			auto arg1 = stack.top();
			stack.pop();
			stack.emplace(arg1.int32_val <= arg2.int32_val);
			DISPATCH();
		}
		i_32_le_u: {
			auto arg2 = stack.top();
			stack.pop();
			auto arg1 = stack.top();
			stack.pop();
			stack.emplace(arg1.uint32_val <= arg2.uint32_val);
			DISPATCH();
		}
		i_32_add: {
			auto arg2 = stack.top();
			stack.pop();
			auto arg1 = stack.top();
			stack.pop();
			stack.emplace(arg1.int32_val + arg2.int32_val);
			DISPATCH();
		}
		i_32_sub: {
			auto arg2 = stack.top();
			stack.pop();
			auto arg1 = stack.top();
			stack.pop();
			//TODO: spec says to mod 2^32
			stack.emplace(arg1.int32_val - arg2.int32_val);
			DISPATCH();
		}
		i_32_mul: {
			auto arg2 = stack.top();
			stack.pop();
			auto arg1 = stack.top();
			stack.pop();
			stack.emplace(arg1.int32_val * arg2.int32_val);
			DISPATCH();
		}
		i_32_and: {
			auto arg2 = stack.top();
			stack.pop();
			auto arg1 = stack.top();
			stack.pop();
			stack.emplace(arg1.int32_val & arg2.int32_val);
			DISPATCH();
		}
		i_32_or: {
			auto arg2 = stack.top();
			stack.pop();
			auto arg1 = stack.top();
			stack.pop();
			stack.emplace(arg1.int32_val | arg2.int32_val);
			DISPATCH();
		}
		i_32_shl: {
			auto arg2 = stack.top();
			stack.pop();
			auto arg1 = stack.top();
			stack.pop();
			stack.emplace(arg1.int32_val << arg2.int32_val);
			DISPATCH();
		}
		i_32_shr_s: {
			auto arg2 = stack.top();
			stack.pop();
			auto arg1 = stack.top();
			stack.pop();
			stack.emplace(arg1.int32_val >> arg2.int32_val);
			DISPATCH();
		}
		i_32_div_s: {
			auto arg2 = stack.top();
			stack.pop();
			auto arg1 = stack.top();
			stack.pop();
			stack.emplace(arg1.int32_val / arg2.int32_val);
			DISPATCH();
		}
		i_32_rem_s: {
			auto arg2 = stack.top();
			stack.pop();
			auto arg1 = stack.top();
			stack.pop();
			stack.emplace(arg1.int32_val % arg2.int32_val);
			DISPATCH();
		}
		i_32_store: {
			auto memarg = instruction.arg.memarg;
			auto t = stack.top().int32_val;
			stack.pop();
			auto i = stack.top().int32_val;
			stack.pop();
			auto limit = static_cast<int>(
					i + memarg.offset + 4);
			if (limit > memory.get_size())
				panic("Reading too far!");
			log_debug("Storing at: %d\n", i + memarg.offset);
			memory.store<int32_t>(t, i + memarg.offset);
			DISPATCH();
		}
		i_32_load: {
			auto memarg = instruction.arg.memarg;
			auto i = stack.top().int32_val;
			auto limit = static_cast<int>(i +
					memarg.offset + 4);
			if (limit > memory.get_size())
				panic("Reading too far!");
			log_debug("reading from %d\n", i + memarg.offset);
			auto result = memory.load<int32_t>(i + memarg.offset);
			stack.emplace(result);
			DISPATCH();
		}
		i_32_load_8_u: {
			auto memarg = instruction.arg.memarg;
			auto i = stack.top().int32_val;
			stack.pop();
			auto limit = static_cast<int>(i +
					memarg.offset + 4);
			if (limit > memory.get_size())
				panic("Reading too far!");
			log_debug("reading from %d\n", i + memarg.offset);
			auto result = memory.load<uint8_t>(i + memarg.offset);
			stack.emplace(result);
			DISPATCH();
		}
		i_32_load_8_s: {
			auto memarg = instruction.arg.memarg;
			auto i = stack.top().int32_val;;
			stack.pop();
			auto limit = static_cast<int>(i +
					memarg.offset + 4);
			if (limit > memory.get_size())
				panic("Reading too far!");
			log_debug("reading from %d\n", i + memarg.offset);
			auto result = memory.load<int8_t>(i + memarg.offset);
			stack.emplace(result);
			DISPATCH();
		}
		instr_call: {
			auto idx = instruction.arg.uint32_val;
			state.pc = pc;
			invoke_function(state, idx, expression);
			pc = state.pc;
			expression = &functions[current_function].expression;
			DISPATCH();
		}
		instr_return: {
			auto frame = state.callstack.top();
			state.callstack.pop();
			if (frame.pc == PC_END) return true;
			pc = frame.pc;
			state.current_function = frame.prev;
			expression = frame.prev_expr;

			//restore labelstack
			const auto size = state.labelstack.size();
			for (size_t i = frame.labelstack_size; i < size; i++)
				state.labelstack.pop();
			DISPATCH();
		}
		instr_block: {
			auto arg = instruction.arg.block;

			Label label;
			label.pc_cont = pc + arg.size;
			state.labelstack.push(label);
			DISPATCH();
		}
		instr_loop: {
			Label label;
			label.pc_cont = pc - 1;
			state.labelstack.push(label);
			DISPATCH();
		}
                instr_if: {
                         auto c = stack.top().int32_val;
			 stack.pop();
                         auto arg = instruction.arg.block;

                         Label label;
                         label.pc_cont = pc + arg.size;
                         state.labelstack.push(label);
                         if (c) {
                                 DISPATCH();
                         } else {
                                 /* TODO: properly handle else clause */
                                 pc = label.pc_cont;
				 state.labelstack.pop();
                                 DISPATCH();
                         }
                }
		br: {
			auto idx = instruction.arg.uint32_val;
			for (unsigned int i = 0; i < idx; i++)
				state.labelstack.pop();
			auto &label = state.labelstack.top();
			state.labelstack.pop();
			pc = label.pc_cont;
			DISPATCH();
		}
		br_if: {
			auto c = stack.top().int32_val;
			stack.pop();
			if (!c) {
				DISPATCH();
			}
			auto idx = instruction.arg.uint32_val;
			for (unsigned int i = 0; i < idx; i++)
				state.labelstack.pop();
			auto &label = state.labelstack.top();
			state.labelstack.pop();
			pc = label.pc_cont;
			DISPATCH();
		}
		instr_drop: {
			stack.pop();
			DISPATCH();
		}
		instr_select: {
			auto c = stack.top().int32_val;
			stack.pop();
			auto val2 = stack.top();
			stack.pop();
			auto val1 = stack.top();
			stack.pop();
			if (c) stack.push(val1);
			else stack.push(val2);
			DISPATCH();
		}
		instr_end: {
			if (state.labelstack.empty()) {
				return true;
			}
			state.labelstack.pop();
			DISPATCH();
		}
		instr_unknown: {
				panic("Unknown instruction encountered %d",
						(*expression)[pc].type);
		}
	}
	return true;
}

frg::optional<GlobalValue> Interpreter::interpret_global(
		DataStream *stream) {
	GlobalValue ret;
	auto type = stream_read<BinaryType>(stream);
	if (!type)
	    panic("Could not read global type");
	ret.type = *type;
	ret.mut = static_cast<bool>(stream->get());

	InterpreterState state;
	state.functions.resize(1);
	/* TODO: write a span so we can avoid copies */
	auto start_pos = stream->tell();
	state.functions[0].expression = decode_code(stream);
	state.functions[0].size = stream->tell() - start_pos;
	state.current_function = 0;
	if(!interpret(state)) return frg::null_opt;

	ret.value = state.stack.top();
	state.stack.pop();
	return ret;
}

frg::optional<uint32_t> Interpreter::interpret_offset(
		DataStream *stream) {
	InterpreterState state;
	state.functions.resize(1);
	/* TODO: write a span so we can avoid copies */
	auto start_pos = stream->tell();
	state.functions[0].expression = decode_code(stream);
	state.functions[0].size = stream->tell() - start_pos;
	state.current_function = 0;
	if(!interpret(state)) return frg::null_opt;

	return state.stack.top().uint32_val;
}

frg::vector<Instruction, frg_allocator> Interpreter::decode_code(
        DataStream *stream) {
	frg::vector<Instruction, frg_allocator> ret;

	auto instruction = stream_read<Instructions>(stream);
	if (!instruction)
		panic("Unable to read instruction");
	while (true) {
		Instruction inst;

		if (*instruction == INSTR_END) {
			inst.type = *instruction;
			ret.push(inst);
			return ret;
		}

		auto arg_size = instruction_sizes.find(*instruction);
		if (arg_size == instruction_sizes.end())
			panic("Don't know size of instruction %d",
				*instruction);

		switch (arg_size->template get<1>()) {
			case SIZE_BLOCK: {
				/* TODO: free this somewhere */
				Block block;
				auto type = stream_read<BinaryType>(stream);
				if (!type)
					panic("Unable to read block type");
				block.type = *type;

				auto instructions = decode_code(stream);
				block.size = instructions.size();
				inst.arg.block = block;
				inst.type = *instruction;
				ret.push(inst);
				std::copy(instructions.begin(), instructions.end(),
						std::back_inserter(ret));
				instruction = stream_read<Instructions>(stream);
				continue;
			}
			case SIZE_U8: {
				auto value = stream_read<uint8_t>(stream);
				if (!value)
					panic("Unable to read value");
				inst.arg.uint8_val = *value;
				inst.type = *instruction;
				break;
			}
			case SIZE_I32: {
				auto value = decode_varint<int32_t>(stream);
				if (!value)
					panic("Unable to read value");
				inst.arg.int32_val = *value;
				inst.type = *instruction;
				break;
			}
			case SIZE_I64: {
				auto value = decode_varint<int64_t>(stream);
				if (!value)
					panic("Unable to read value");
				inst.arg.int64_val = *value;
				inst.type = *instruction;
				break;
			}
			case SIZE_U32: {
				auto value = decode_varuint<uint32_t>(stream);
				if (!value)
					panic("Unable to read value");
				inst.arg.uint32_val = *value;
				inst.type = *instruction;
				break;
			}
			case SIZE_U64: {
				auto value = decode_varuint<uint64_t>(stream);
				if (!value)
					panic("Unable to read value");
				inst.arg.uint64_val = *value;
				inst.type = *instruction;
				break;
			}
			case SIZE_0: {
				inst.type = *instruction;
				break;
			}
			case SIZE_MEMARG: {
				auto align = decode_varuint<uint32_t>(stream);
				auto offset = decode_varuint<uint32_t>(stream);
				if (!align || !offset)
					panic("Unable to read value");
				MemArg arg;
				arg.align = *align;
				arg.offset = *offset;

				inst.arg.memarg = arg;
				inst.type = *instruction;
				break;
			}
			default:
				panic("Unable to handle size");
		}
		ret.push(inst);

		instruction = stream_read<Instructions>(stream);
	}

	return ret;
}

} /* namespace bearwasm */
