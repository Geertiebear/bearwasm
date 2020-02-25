#ifndef BEARWASM_UTIL_H
#define BEARWASM_UTIL_H

#include <stdarg.h>
#include <type_traits>
#include <utility>
#include <bearwasm/host.hpp>
#include <bearwasm/libc.h>
#include <frg/optional.hpp>
#include <frg/string.hpp>
#include <frg/tuple.hpp>

namespace bearwasm {

#ifdef BEARWASM_DEBUG
static inline void log_debug(const char *msg, ...) {
	va_list va;
	va_start(va, msg);
	char buf[256];
	vsnprintf(buf, 256, msg, va);
	bearwasm_log(BEARWASM_DEBUG, buf);
	va_end(va);
}
#else
static inline void log_debug(const char *msg, ...) {
	(void) msg;
}
#endif

static inline void panic(const char *msg, ...) {
	va_list va;
	va_start(va, msg);
	char buf[256];
	vsnprintf(buf, 256, msg, va);
	bearwasm_log(BEARWASM_ERR, buf);
	va_end(va);
	bearwasm_abort();
}

static inline void log_info(const char *msg, ...) {
	va_list va;
	va_start(va, msg);
	char buf[256];
	vsnprintf(buf, 256, msg, va);
	bearwasm_log(BEARWASM_INFO, buf);
	va_end(va);
}

static inline void log_warn(const char *msg, ...) {
	va_list va;
	va_start(va, msg);
	char buf[256];
	vsnprintf(buf, 256, msg, va);
	bearwasm_log(BEARWASM_WARN, buf);
	va_end(va);
}

using Limit = frg::tuple<uint32_t, uint32_t>;

template<typename T>
frg::optional<T> stream_read(DataStream *stream) {
	T ret;
	auto res = stream->read(reinterpret_cast<char*>(&ret),
			sizeof(T));
	if (!res) return frg::null_opt;
	return ret;
}

template<typename T>
frg::optional<T> decode_varuint(DataStream *stream) {
	static_assert(std::is_unsigned<T>::value);

	T ret = 0;
	int shift = 0;
	while (true) {
		auto c = stream->get();
		if (!c)
			return frg::null_opt;
		ret |= (*c & 0x7f) << shift;
		if (!(*c & 0x80))
			break;
		shift += 7;
	}

	return ret;
}

template<typename T>
frg::optional<T> decode_varint(DataStream *stream) {
	static_assert(std::is_signed<T>::value);

	T ret = 0;
	int shift = 0;
	int size = sizeof(T) * 8;
	frg::optional<char> c;

	do {
		c = stream->get();
		if (!c)
			return frg::null_opt;
		ret |= (*c & 0x7f) << shift;
		shift += 7;
		/* TODO: check if we dont go over max amount of bits */
	} while (*c & 0x80);
	
	if ((shift < size) && (*c & 0x40)) {
		ret |= -(1 << shift);
	}

	return ret;
}

extern frg::optional<Limit> decode_limit(DataStream *stream);

extern frg::optional<frg::string<frg_allocator>> read_string(DataStream *stream);

} /* namespace bearwasm */

#endif
