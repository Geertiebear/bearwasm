#ifndef BEARWASM_FRIGG_ALLOCATOR_H
#define BEARWASM_FRIGG_ALLOCATOR_H

struct frg_allocator {
    void *allocate(size_t size);

    void free(void *p);

    void deallocate(void *p, size_t n);
};

#endif
