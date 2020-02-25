#ifndef BEARWASM_HOST_HPP
#define BEARWASM_HOST_HPP

#include <frg/optional.hpp>

struct frg_allocator {
    void *allocate(size_t size);

    void free(void *p);

    void deallocate(void *p, size_t n);
};

extern void bearwasm_log(int level, const char *str);
extern void bearwasm_abort();

namespace bearwasm {

enum log_level {
	BEARWASM_DEBUG,
	BEARWASM_WARN,
	BEARWASM_ERR,
	BEARWASM_INFO,
};

class DataStream {
public:
    enum SeekType {
        BWASM_SEEK_CUR,
        BWASM_SEEK_END,
        BWASM_SEEK_SET,
    };
    /*
     * Return the next character in the stream.
     * Returns frg::null_opt when there are no more characters
     * or an error occured.
     */
    virtual frg::optional<char> get() = 0;

    /*
     * Read size bytes from the stream and places them in buf.
     * Returns true upon success, false upon failure to
     * satisfy all size bytes.
     */ 
    virtual bool read(char *buf, size_t size) = 0;

    /*
     * Sets stream position depending on the seek
     * type. Returns -1 on error
     */
    virtual int seek(long pos, SeekType type) = 0;

    virtual long tell() = 0;
};

} /* namespace bearwasm */
#endif
