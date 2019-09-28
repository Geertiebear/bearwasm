#ifndef BEARWASM_STACK_H
#define BEARWASM_STACK_H

#include <iostream>
#include <stdint.h>
#include <vector>

namespace bearwasm {

class Stack {
public:
	Stack() : sp(0) {}

	template<typename T>
	void push(T value) {
		if (bytes.size() < (sp + sizeof(T) + 1))
			bytes.resize(sp + sizeof(T) + 1);
		auto data = reinterpret_cast<uint8_t*>(&value);
		for (size_t i = 0; i < sizeof(T); i++)
			bytes[sp + i] = data[i];
		sp += sizeof(T) - 1;
	}

	template<typename T>
	T pop() {
		T ret;
		auto constexpr size = static_cast<int>(sizeof(T));
		auto data = reinterpret_cast<uint8_t*>(&ret);
		for (int i = -size; i < 0; i++) {
			data[size + i] = bytes[sp + i + 1];
		}
		sp -= size - 1;
		return ret;
	}
private:
	std::vector<uint8_t> bytes;
	int sp;
};

} /* namespace bearwasm */
#endif
