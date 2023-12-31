#pragma once

#define NON_COPYABLE(CLASS_NAME)                       \
    CLASS_NAME(const CLASS_NAME&)            = delete; \
    CLASS_NAME& operator=(const CLASS_NAME&) = delete; \
    CLASS_NAME(CLASS_NAME&&)                 = delete; \
    CLASS_NAME& operator=(CLASS_NAME&&)      = delete;

#define FLAG_ENUM_CLASS(ENUM_CLASS_NAME, ENUM_CLASS_TYPE)                                                       \
    inline ENUM_CLASS_NAME operator|(ENUM_CLASS_NAME a, ENUM_CLASS_NAME b)                                      \
    {                                                                                                           \
        return static_cast<ENUM_CLASS_NAME>(static_cast<ENUM_CLASS_TYPE>(a) | static_cast<ENUM_CLASS_TYPE>(b)); \
    }                                                                                                           \
    inline ENUM_CLASS_NAME operator&(ENUM_CLASS_NAME a, ENUM_CLASS_NAME b)                                      \
    {                                                                                                           \
        return static_cast<ENUM_CLASS_NAME>(static_cast<ENUM_CLASS_TYPE>(a) & static_cast<ENUM_CLASS_TYPE>(b)); \
    }                                                                                                           \
    inline ENUM_CLASS_NAME operator~(ENUM_CLASS_NAME a)                                                         \
    {                                                                                                           \
        return static_cast<ENUM_CLASS_NAME>(~static_cast<ENUM_CLASS_TYPE>(a));                                  \
    }                                                                                                           \
    inline ENUM_CLASS_NAME& operator|=(ENUM_CLASS_NAME& a, ENUM_CLASS_NAME b)                                   \
    {                                                                                                           \
        a = a | b;                                                                                              \
        return a;                                                                                               \
    }                                                                                                           \
    inline ENUM_CLASS_NAME& operator&=(ENUM_CLASS_NAME& a, ENUM_CLASS_NAME b)                                   \
    {                                                                                                           \
        a = a & b;                                                                                              \
        return a;                                                                                               \
    }                                                                                                           \
    inline bool HasFlag(ENUM_CLASS_NAME a, ENUM_CLASS_NAME b)                                                   \
    {                                                                                                           \
        return (a & b) == b;                                                                                    \
    }