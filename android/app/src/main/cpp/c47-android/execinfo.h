#ifndef DUMMY_EXECINFO_H
#define DUMMY_EXECINFO_H

static inline int backtrace(void** buffer, int size) { return 0; }
static inline char** backtrace_symbols(void* const* buffer, int size) { return 0; }

#endif
