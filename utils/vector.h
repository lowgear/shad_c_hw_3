#define DEF_VECTOR(name, T) \
struct name { \
    size_t size; \
    T array[1]; \
}
