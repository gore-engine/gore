#pragma once

#define NON_COPYABLE(CLASS_NAME)                       \
    CLASS_NAME(const CLASS_NAME&)            = delete; \
    CLASS_NAME& operator=(const CLASS_NAME&) = delete; \
    CLASS_NAME(CLASS_NAME&&)                 = delete; \
    CLASS_NAME& operator=(CLASS_NAME&&)      = delete;

#define SHALLOW_COPYABLE(CLASS_NAME)                    \
    CLASS_NAME(const CLASS_NAME&)            = default; \
    CLASS_NAME& operator=(const CLASS_NAME&) = default; \
    CLASS_NAME(CLASS_NAME&&)                 = default; \
    CLASS_NAME& operator=(CLASS_NAME&&)      = default;
