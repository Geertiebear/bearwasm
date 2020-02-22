#include <bearwasm/Util.h>

namespace bearwasm {

frg::optional<Limit> decode_limit(DataStream *stream) {
	auto has_max = stream_read<uint8_t>(stream);
	auto min = decode_varuint<uint32_t>(stream);
	if (!min || !has_max)
		return frg::null_opt;
	frg::optional<uint8_t> max = 0;
	if (*has_max) {
		max = decode_varuint<uint32_t>(stream);
		if (!max)
			return frg::null_opt;
	}
	return frg::make_tuple(*min, *max);
}

frg::optional<frg::string<frg_allocator>> read_string(DataStream *stream) {
	frg::string<frg_allocator> ret;

	auto length = stream_read<uint8_t>(stream);
	if (!length) return frg::null_opt;

	ret.resize(*length);
	auto res = stream->read(&ret[0], *length);
	if (!res) return frg::null_opt;
	return ret;
}

} /* namespace bearwasm */
