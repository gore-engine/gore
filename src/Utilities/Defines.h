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

#define TEMPLATE_ENABLE_IF_SAME_TYPE_IGNORE_CV(TEMPLATE_TYPE_NAME, EQUAL_CHECK_REAL_TYPE) \
    template <typename TEMPLATE_TYPE_NAME,                                                \
              typename = typename std::enable_if_t<std::is_same_v<std::decay_t<TEMPLATE_TYPE_NAME>, EQUAL_CHECK_REAL_TYPE>>>

#define TEMPLATE_ENABLE_IF_SAME_TYPE_IGNORE_CV_BEFORE_DEFINITION(TEMPLATE_TYPE_NAME, EQUAL_CHECK_REAL_TYPE) \
    template <typename TEMPLATE_TYPE_NAME, typename>
