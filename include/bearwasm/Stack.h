#ifndef BEARWASM_STACK_H
#define BEARWASM_STACK_H

#include <iostream>
#include <stdint.h>
#include <vector>
#include <cassert>

#include <bearwasm/Format.h>

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
		std::cout << "pushing: " << value << std::endl;
		static_assert(sizeof(T) < UINT8_MAX);
		/* TODO: decide if we want to dynamically grow stack */
		assert(sp - sizeof(T) - 1  >= 0);

		auto data = reinterpret_cast<uint8_t*>(&value);
		for (size_t i = 0; i < sizeof(T); i++)
			bytes[sp - i] = data[i];

		bytes[sp - sizeof(T)] = static_cast<uint8_t>(sizeof(T));
		sp -= sizeof(T) + 1;
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
		std::cout << "popping: " << ret << std::endl;
		return ret;
	}

	void drop() {
		int size = bytes[sp];
		std::cout << "Dropping size: " << size << std::endl;
		sp += size + 1;
	}

	Value pop_variant(BinaryType type) {
		switch (type) {
			case I_32:
				return pop<int32_t>();
			case I_64:
				return pop<int64_t>();
			case F_32:
				return pop<float>();
			case F_64:
				return pop<double>();
			default:
				return 0;
		}
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


template<>
inline void Stack::push<Value>(Value value) {
	std::visit([this] (auto&& arg) {
		push<decltype(arg)>(arg);
	}, value);
}

} /* namespace bearwasm */
#endif
