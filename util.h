#ifdef ARRAYSIZE
#undef ARRAYSIZE
#endif

/**
 * Determine the number of entries in a fixed size array.
 */
#define ARRAYSIZE(x) ((int)(sizeof(x) / sizeof(x[0])))