#pragma once

#include "Utilities/Defines.h"

#define MATHF_SIMD_SET_VALUE_TYPE(SIMD_VALUE_TYPE) using SIMDValueType = SIMD_VALUE_TYPE;

#define MATHF_SIMD_CONVERSION_WITH_VALUE_TYPE_DECLARATIONS(CLASS_NAME) \
    operator SIMDValueType() const noexcept;                           \
    explicit CLASS_NAME(const SIMDValueType& V) noexcept;              \
    explicit CLASS_NAME(SIMDValueType&& V) noexcept;                   \
    CLASS_NAME& operator=(const SIMDValueType& V) noexcept;

#define MATHF_COMMON_COMPARISON_OPERATOR_DECLARATIONS(CLASS_NAME) \
    bool operator==(const CLASS_NAME& V) const noexcept;          \
    bool operator!=(const CLASS_NAME& V) const noexcept;

#define MATHF_VECTOR_COMPARISON_OPERATOR_DECLARATIONS(CLASS_NAME) \
    MATHF_COMMON_COMPARISON_OPERATOR_DECLARATIONS(CLASS_NAME)

#define MATHF_MATRIX_COMPARISON_OPERATOR_DECLARATIONS(CLASS_NAME) \
    MATHF_COMMON_COMPARISON_OPERATOR_DECLARATIONS(CLASS_NAME)

// operator= is defined as default in SHALLOW_COPYABLE(CLASS_NAME)
#define MATHF_COMMON_COMPOUND_ASSIGNMENT_OPERATOR_DECLARATIONS(CLASS_NAME) \
    CLASS_NAME& operator+=(const CLASS_NAME& V) noexcept;                  \
    CLASS_NAME& operator-=(const CLASS_NAME& V) noexcept;                  \
    CLASS_NAME& operator*=(const CLASS_NAME& V) noexcept;                  \
    CLASS_NAME& operator*=(float S) noexcept;                              \
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

#define CAST_FROM_SIMD_MATRIX_HELPER(CLASS_NAME, VALUE) (static_cast<CLASS_NAME>(static_cast<CLASS_NAME::SIMDValueType>(rtm::matrix_cast((VALUE)))))
