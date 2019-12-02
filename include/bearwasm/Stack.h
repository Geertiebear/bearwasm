#ifndef BEARWASM_STACK_H
#define BEARWASM_STACK_H

#include <iostream>
#include <stdint.h>
#include <vector>
#include <cassert>

#include <bearwasm/Format.h>
#include <bearwasm/Util.h>

namespace bearwasm {

class Stack {
public:
	Stack(int size) : sp(size - 1) {
		bytes = new char[size];
	}

	~Stack() {
		delete bytes;
	}

	template<typename T>
	void push(T value) {
		log_debug("pushing: %d\n", value);
		static_assert(sizeof(T) < UINT8_MAX);
		/* TODO: decide if we want to dynamically grow stack */
		assert(sp - sizeof(T) - 1  >= 0);

		auto data = reinterpret_cast<uint8_t*>(&value);
		for (size_t i = 0; i < sizeof(T); i++)
			bytes[sp - i] = data[i];

		bytes[sp - sizeof(T)] = static_cast<uint8_t>(sizeof(T));
		sp -= sizeof(T) + 1;
	}

	void push_val(Value value, BinaryType type) {
		switch (type) {
			case I_32:
				push(value.int32_val);
				break;
			case I_64:
				push(value.int64_val);
				break;
			case F_32:
				push(value.float_val);
				break;
			case F_64:
				push(value.double_val);
				break;
			default:
				return;
		}
	}
	
	template<typename T>
	T pop() {
		T ret;
		auto constexpr size = static_cast<int>(sizeof(T));
		auto data = reinterpret_cast<uint8_t*>(&ret);

		sp += 1; // pop off size, we don't need it
		for (int i = -size; i < 0; i++) {
			data[size + i] = bytes[sp - i];
		}
		sp += size;
		log_debug("popping: %d\n", ret);
		return ret;
	}

	void drop() {
		int size = bytes[sp];
		sp += size + 1;
	}

	Value pop_variant(BinaryType type) {
		Value val;
		switch (type) {
			case I_32:
				val.int32_val = pop<int32_t>();
				break;
			case I_64:
				val.int64_val = pop<int64_t>();
				break;
			case F_32:
				val.float_val =  pop<float>();
				break;
			case F_64:
				val.double_val = pop<double>();
				break;
			default:
				return val;
		}
		return val;
	}

	int get_sp() const {
		return sp;
	}

	void set_sp(int val) {
		sp = val;
	}
private:
	char *bytes;
	int sp;
};



} /* namespace bearwasm */
#endif
