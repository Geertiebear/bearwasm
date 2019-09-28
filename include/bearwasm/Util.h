#ifndef BEARWASM_UTIL_H
#define BEARWASM_UTIL_H

#include <type_traits>
#include <fstream>
#include <functional>
#include <optional>
#include <iostream>
#include <string>
#include <utility>
#include <variant>

namespace bearwasm {
namespace {

using Limit = std::pair<short, short>;

template<typename T>
std::optional<T> stream_read(std::ifstream &stream) {
	T ret;
	auto &res = stream.read(reinterpret_cast<char*>(&ret),
			sizeof(T));
	if (res.fail()) return std::nullopt;
	return ret;
}

void panic(const std::string &error) {
	std::cout << error << std::endl;
	exit(1);
}

template<typename T>
std::optional<T> decode_varuint_f(
		std::function<std::optional<char>()> get) {
	static_assert(std::is_unsigned<T>::value);

	T ret = 0;
	int shift = 0;
	while (true) {
		auto c = get();
		if (!c)
			return std::nullopt;
		ret |= (*c & 0x7f) << shift;
		if (!(*c & 0x80))
			break;
		shift += 7;
	}

	return ret;
}

template<typename T>
std::optional<T> decode_varuint_s(std::ifstream &stream) {
	return decode_varuint_f<T>([&stream]() -> std::optional<char> {
			auto c = stream.get();
			if (c == EOF) return std::nullopt;
			return static_cast<char>(c);
	});
}

template<typename T>
std::optional<T> decode_varint_f(
		std::function<std::optional<char>()> get) {
	static_assert(std::is_signed<T>::value);

	T ret = 0;
	int shift = 0;
	int size = sizeof(T) * 8;
	std::optional<char> c;

	do {
		c = get();
		if (!c)
			return std::nullopt;
		ret |= (*c & 0x7f) << shift;
		shift += 7;
		/* TODO: check if we dont go over max amount of bits */
	} while (*c & 0x80);
	
	if ((shift < size) && (*c & 0x40)) {
		ret |= -(1 << shift);
	}

	return ret;
}

template<typename T>
std::optional<T> decode_varint_s(std::ifstream &stream) {
	return decode_varint_f<T>([&stream]() -> std::optional<char> {
			auto c = stream.get();
			if (c == EOF) return std::nullopt;
			return static_cast<char>(c);
	});
}

std::optional<Limit> decode_limit(std::ifstream &stream) {
	auto has_max = stream_read<uint8_t>(stream);
	auto min = stream_read<uint8_t>(stream);
	if (!min || !has_max)
		return std::nullopt;
	std::optional<uint8_t> max = 0;
	if (*has_max) {
		max = stream_read<uint8_t>(stream);
		if (!max)
			return std::nullopt;
	}
	return std::make_pair(*min, *max);
}

std::optional<std::string> read_string(std::ifstream &stream) {
	std::string ret;

	auto length = stream_read<uint8_t>(stream);
	if (!length) return std::nullopt;

	ret.resize(*length);
	auto &res = stream.read(&ret[0], *length);
	if (res.fail()) return std::nullopt;
	return ret;
}
/*
 * In the future I may want to abstract away the stream
 * of bytes that's being decoded so i'll keep this here for now
 *
* thank you cppreference.com
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

using ViewSource = std::variant<
	std::vector<uint8_t>,
	std::ifstream*>;

class View {
public:
	template<typename T>
	View(T source) : source(source), position(0) {}

	std::optional<char> get() {
		return std::visit(overloaded {
			[this] (std::vector<uint8_t> arg) -> std::optional<char> {
				if (position >= arg.size())
					return std::nullopt;
				return arg[position++];
			},
			[] (std::ifstream *arg) -> std::optional<char> {
				auto c = arg->get();
				if (c == EOF)
					return std::nullopt;
				return c;
			},
			[] (auto arg) -> std::optional<char> {
				return std::nullopt;
			}
		}, source);
	}
private:
	int position;
	ViewSource source;
};
*/
}
} /* namespace bearwasm */

#endif