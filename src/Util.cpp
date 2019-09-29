#include <bearwasm/Util.h>

namespace bearwasm {

void panic(const std::string &error) {
	std::cout << error << std::endl;
	exit(1);
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

} /* namespace bearwasm */
