#ifndef BEARWASM_STACK_H
#define BEARWASM_STACK_H

#include <iostream>
#include <stdint.h>
#include <vector>

#include <bearwasm/Format.h>

namespace bearwasm {

class Stack {
public:
	Stack() : sp(0) {}

	template<typename T>
	void push(T value) {
		std::cout << "pushing: " << value << std::endl;
		if (bytes.size() < (sp + sizeof(T) + 1))
			bytes.resize(sp + sizeof(T) + 1);
		auto data = reinterpret_cast<uint8_t*>(&value);
		for (size_t i = 0; i < sizeof(T); i++)
			bytes[sp + i] = data[i];
		sp += sizeof(T);
	}
	
	template<typename T>
	T pop() {
		T ret;
		auto constexpr size = static_cast<int>(sizeof(T));
		auto data = reinterpret_cast<uint8_t*>(&ret);
		for (int i = -size; i < 0; i++) {
			data[size + i] = bytes[sp + i];
		}
		sp -= size;
		std::cout << "popping: " << ret << std::endl;
		return ret;
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
	std::vector<uint8_t> bytes;
	std::vector<int> labels;
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
