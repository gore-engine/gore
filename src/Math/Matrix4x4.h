#pragma once

#include <ostream>

#include "Export.h"

#include "Utilities/Defines.h"
#include "Math/Defines.h"

#include "rtm/vector4f.h"
#include "rtm/matrix3x4f.h"
#include "rtm/matrix4x4f.h"
#include "rtm/impl/matrix_common.h"

namespace gore
{

ENGINE_STRUCT(Plane);
ENGINE_STRUCT(Vector3);
ENGINE_STRUCT(Vector4);
ENGINE_STRUCT(Matrix4x4);
ENGINE_STRUCT(Quaternion);

ENGINE_STRUCT(Matrix4x4)
{
public:
    friend ENGINE_API_FUNC(std::ostream&, operator<<, std::ostream & os, const Matrix4x4& m) noexcept;

public:
    MATHF_SIMD_SET_VALUE_TYPE(rtm::matrix4x4f);
    MATHF_SIMD_CONVERSION_WITH_VALUE_TYPE_DECLARATIONS(Matrix4x4);

    SHALLOW_COPYABLE(Matrix4x4);

    MATHF_COMMON_UNARY_OPERATOR_DECLARATIONS(Matrix4x4);
    MATHF_MATRIX_COMPARISON_OPERATOR_DECLARATIONS(Matrix4x4);
    MATHF_MATRIX_COMPOUND_ASSIGNMENT_OPERATOR_DECLARATIONS(Matrix4x4);

    SIMDValueType m_M;

    // clang-format off
    Matrix4x4() noexcept = default;
    Matrix4x4(
        float m00, float m01, float m02, float m03,
        float m10, float m11, float m12, float m13,
        float m20, float m21, float m22, float m23,
        float m30, float m31, float m32, float m33) noexcept :
        m_M(rtm::matrix_set(
            rtm::vector_set(m00, m01, m02, m03),
            rtm::vector_set(m10, m11, m12, m13),
            rtm::vector_set(m20, m21, m22, m23),
            rtm::vector_set(m30, m31, m32, m33)))
    {
    }
    // clang-format on
    explicit Matrix4x4(const Vector3& r0, const Vector3& r1, const Vector3& r2) noexcept;
    explicit Matrix4x4(const Vector4& r0, const Vector4& r1, const Vector4& r2, const Vector4& r3) noexcept;

    // Properties
    [[nodiscard]] Vector3 GetUp() const noexcept;
    void SetUp(const Vector3& v) noexcept;

    [[nodiscard]] Vector3 GetDown() const noexcept;
    void SetDown(const Vector3& v) noexcept;

    [[nodiscard]] Vector3 GetRight() const noexcept;
    void SetRight(const Vector3& v) noexcept;

    [[nodiscard]] Vector3 GetLeft() const noexcept;
    void SetLeft(const Vector3& v) noexcept;

    [[nodiscard]] Vector3 GetForward() const noexcept;
    void SetForward(const Vector3& v) noexcept;

    [[nodiscard]] Vector3 GetBackward() const noexcept;
    void SetBackward(const Vector3& v) noexcept;

    [[nodiscard]] Vector3 GetTranslation() const noexcept;
    void SetTranslation(const Vector3& v) noexcept;

    [[nodiscard]] Quaternion GetRotation() const noexcept;
    void SetRotation(const Quaternion& q) noexcept;

    [[nodiscard]] Vector3 GetLossyScale() const noexcept;
    void SetScale(const Vector3& v) noexcept;

    // Matrix operations
    bool Decompose(Vector3 & scale, Quaternion & rotation, Vector3 & translation) noexcept;

    [[nodiscard]] Matrix4x4 Transpose() const noexcept;
    static void Transpose(Matrix4x4 & m) noexcept;

    [[nodiscard]] Matrix4x4 Inverse() const noexcept;
    void Invert()  noexcept;
    [[nodiscard]] static void Invert(Matrix4x4 & m) noexcept;

    float Determinant() const noexcept;

    Vector3 TransformPoint(const Vector3& position) const noexcept;
    static Vector3 TransformPoint(const Matrix4x4& M, const Vector3& position) noexcept;
    Vector3 TransformVector(const Vector3& vector) const noexcept;
    static Vector3 TransformVector(const Matrix4x4& M, const Vector3& vector) noexcept;

    static void Lerp(const Matrix4x4& M1, const Matrix4x4& M2, float t, Matrix4x4& result) noexcept;
    static Matrix4x4 Lerp(const Matrix4x4& M1, const Matrix4x4& M2, float t) noexcept;

    // Construct a matrix from a T/R/S
    static Matrix4x4 FromTranslation(const Vector3& position) noexcept;
    static Matrix4x4 FromTranslation(float x, float y, float z) noexcept;

    static Matrix4x4 FromRotationX(float radians) noexcept;
    static Matrix4x4 FromRotationY(float radians) noexcept;
    static Matrix4x4 FromRotationZ(float radians) noexcept;

    static Matrix4x4 FromAxisAngle(const Vector3& axis, float angle) noexcept;
    static Matrix4x4 FromQuaternion(const Quaternion& quat) noexcept;
    // Rotates about y-axis (yaw), then x-axis (pitch), then z-axis (roll)
    static Matrix4x4 FromYawPitchRoll(float yaw, float pitch, float roll) noexcept;
    // Rotates about y-axis (angles.y), then x-axis (angles.x), then z-axis (angles.z)
    static Matrix4x4 FromEulerAngles(const Vector3& angles) noexcept;

    static Matrix4x4 FromScale(const Vector3& scales) noexcept;
    static Matrix4x4 FromScale(float xs, float ys, float zs) noexcept;
    static Matrix4x4 FromScale(float scale) noexcept;

    // Projection matrices
    static Matrix4x4 CreatePerspectiveFieldOfViewLH(float fov, float aspectRatio, float nearPlane, float farPlane) noexcept;
    static Matrix4x4 CreatePerspectiveOffCenter(float left, float right, float bottom, float top, float nearPlane, float farPlane) noexcept;
    static Matrix4x4 CreateOrthographic(float width, float height, float zNearPlane, float zFarPlane) noexcept;
    static Matrix4x4 CreateOrthographicOffCenter(float left, float right, float bottom, float top, float zNearPlane, float zFarPlane) noexcept;

    static Matrix4x4 CreateLookAt(const Vector3& position, const Vector3& target, const Vector3& up) noexcept;

    // Misc
    static Matrix4x4 CreateWorld(const Vector3& position, const Vector3& forward, const Vector3& up) noexcept;

    static Matrix4x4 CreateShadow(const Vector3& lightDir, const Plane& plane) noexcept;

    static Matrix4x4 CreateReflection(const Plane& plane) noexcept;

    static Matrix4x4 CreateBillboard(
        const Vector3& object,
        const Vector3& cameraPosition,
        const Vector3& cameraUp,
        const Vector3* cameraForward = nullptr) noexcept;

    static Matrix4x4 CreateConstrainedBillboard(
        const Vector3& object,
        const Vector3& cameraPosition,
        const Vector3& rotateAxis,
        const Vector3* cameraForward = nullptr,
        const Vector3* objectForward = nullptr) noexcept;

public:
    // Common Values
    static const Matrix4x4 Identity;
};

TEMPLATE_ENABLE_IF_SAME_TYPE_IGNORE_CV_BEFORE_DEFINITION(TFrom, Matrix4x4::SIMDValueType)
Matrix4x4::Matrix4x4(TFrom&& F) noexcept :
    m_M(std::forward<SIMDValueType>(F))
{
}

TEMPLATE_ENABLE_IF_SAME_TYPE_IGNORE_CV_BEFORE_DEFINITION(TFrom, Matrix4x4::SIMDValueType)
Matrix4x4& Matrix4x4::operator=(TFrom&& F) noexcept
{
    m_M = std::forward<SIMDValueType>(F);
    return *this;
}

MATHF_MATRIX_BINARY_OPERATOR_DECLARATIONS(Matrix4x4);

} // namespace gore