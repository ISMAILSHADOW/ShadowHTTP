#pragma once

#include <cstdio>

#ifndef NDEBUG
#define LOG_DEBUG(fmt, ...)                                                                                            \
    do {                                                                                                               \
        std::fprintf(stderr, "[DEBUG] %s:%d (%s): " fmt "\n", __FILE__, __LINE__,                                      \
                     __func__ __VA_OPT__(, ) __VA_ARGS__);                                                             \
    } while (0)

#define UNUSED(x) ((void)(x))
#else
#define LOG_DEBUG(fmt, ...) ((void)0)
#endif
