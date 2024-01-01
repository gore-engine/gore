#pragma once

#define MATHF_SIMD_SET_VALUE_TYPE(VALUE_TYPE) using ValueType = VALUE_TYPE;

#define MATHF_SIMD_CONVERSION_WITH_VALUE_TYPE_DECLARATIONS(CLASS_NAME) \
    explicit operator ValueType() const noexcept;                      \
    constexpr explicit CLASS_NAME(const ValueType& F) noexcept;        \
    constexpr explicit CLASS_NAME(ValueType&& F) noexcept;             \
    CLASS_NAME& operator=(const ValueType& F) noexcept;


#define MATHF_COMMON_COMPARISON_OPERATOR_DECLARATIONS(CLASS_NAME) \
    bool operator==(const CLASS_NAME& V) const noexcept;          \
    bool operator!=(const CLASS_NAME& V) const noexcept;

#define MATHF_VECTOR_COMPARISON_OPERATOR_DECLARATIONS(CLASS_NAME) \
    MATHF_COMMON_COMPARISON_OPERATOR_DECLARATIONS(CLASS_NAME)

#define MATHF_MATRIX_COMPARISON_OPERATOR_DECLARATIONS(CLASS_NAME) \
    MATHF_COMMON_COMPARISON_OPERATOR_DECLARATIONS(CLASS_NAME)


#define MATHF_COMMON_COMPOUND_ASSIGNMENT_OPERATOR_DECLARATIONS(CLASS_NAME) \
    CLASS_NAME& operator+=(const CLASS_NAME& V) noexcept;         \
    CLASS_NAME& operator-=(const CLASS_NAME& V) noexcept;         \
    CLASS_NAME& operator*=(const CLASS_NAME& V) noexcept;         \
    CLASS_NAME& operator*=(float S) noexcept;                     \
    CLASS_NAME& operator/=(float S) noexcept;

#define MATHF_VECTOR_COMPOUND_ASSIGNMENT_OPERATOR_DECLARATIONS(CLASS_NAME) \
    MATHF_COMMON_COMPOUND_ASSIGNMENT_OPERATOR_DECLARATIONS(CLASS_NAME)

#define MATHF_MATRIX_COMPOUND_ASSIGNMENT_OPERATOR_DECLARATIONS(CLASS_NAME) \
    MATHF_COMMON_COMPOUND_ASSIGNMENT_OPERATOR_DECLARATIONS(CLASS_NAME)


#define MATHF_COMMON_UNARY_OPERATOR_DECLARATIONS(CLASS_NAME) \
    CLASS_NAME operator+() const noexcept;                   \
    CLASS_NAME operator-() const noexcept;

#define MATHF_COMMON_BINARY_OPERATOR_DECLARATIONS(CLASS_NAME)                  \
    CLASS_NAME operator+(const CLASS_NAME& V1, const CLASS_NAME& V2) noexcept; \
    CLASS_NAME operator-(const CLASS_NAME& V1, const CLASS_NAME& V2) noexcept; \
    CLASS_NAME operator*(const CLASS_NAME& V1, const CLASS_NAME& V2) noexcept; \
    CLASS_NAME operator*(const CLASS_NAME& V, float S) noexcept;               \
    CLASS_NAME operator/(const CLASS_NAME& V1, const CLASS_NAME& V2) noexcept; \
    CLASS_NAME operator/(const CLASS_NAME& V, float S) noexcept;               \
    CLASS_NAME operator*(float S, const CLASS_NAME& V) noexcept;

#define MATHF_VECTOR_BINARY_OPERATOR_DECLARATIONS(CLASS_NAME) \
    MATHF_COMMON_BINARY_OPERATOR_DECLARATIONS(CLASS_NAME)

#define MATHF_MATRIX_BINARY_OPERATOR_DECLARATIONS(CLASS_NAME) \
    MATHF_COMMON_BINARY_OPERATOR_DECLARATIONS(CLASS_NAME)
