#pragma once

#include <ostream>

#include "Export.h"

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

    Matrix4x4(const rtm::matrix3x4f& F) noexcept;

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
    [[nodiscard]] Vector3 Up() const noexcept;
    void Up(const Vector3& v) noexcept;

    [[nodiscard]] Vector3 Down() const noexcept;
    void Down(const Vector3& v) noexcept;

    [[nodiscard]] Vector3 Right() const noexcept;
    void Right(const Vector3& v) noexcept;

    [[nodiscard]] Vector3 Left() const noexcept;
    void Left(const Vector3& v) noexcept;

    [[nodiscard]] Vector3 Forward() const noexcept;
    void Forward(const Vector3& v) noexcept;

    [[nodiscard]] Vector3 Backward() const noexcept;
    void Backward(const Vector3& v) noexcept;

    [[nodiscard]] Vector3 Translation() const noexcept;
    void Translation(const Vector3& v) noexcept;

    // Matrix operations
    bool Decompose(Vector3 & scale, Quaternion & rotation, Vector3 & translation) noexcept;

    Matrix4x4 Transpose() const noexcept;
    void Transpose(Matrix4x4 & result) const noexcept;

    Matrix4x4 Invert() const noexcept;
    void Invert(Matrix4x4 & result) const noexcept;

    float Determinant() const noexcept;

    // Computes rotation about y-axis (y), then x-axis (x), then z-axis (z)
    Vector3 ToEuler() const noexcept;

    // Static functions
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

    static Matrix4x4 CreateTranslation(const Vector3& position) noexcept;
    static Matrix4x4 CreateTranslation(float x, float y, float z) noexcept;

    static Matrix4x4 CreateScale(const Vector3& scales) noexcept;
    static Matrix4x4 CreateScale(float xs, float ys, float zs) noexcept;
    static Matrix4x4 CreateScale(float scale) noexcept;

    static Matrix4x4 CreateRotationX(float radians) noexcept;
    static Matrix4x4 CreateRotationY(float radians) noexcept;
    static Matrix4x4 CreateRotationZ(float radians) noexcept;

    static Matrix4x4 CreateFromAxisAngle(const Vector3& axis, float angle) noexcept;

    static Matrix4x4 CreatePerspectiveFieldOfViewLH(float fov, float aspectRatio, float nearPlane, float farPlane) noexcept;
    static Matrix4x4 CreatePerspectiveOffCenter(float left, float right, float bottom, float top, float nearPlane, float farPlane) noexcept;
    static Matrix4x4 CreateOrthographic(float width, float height, float zNearPlane, float zFarPlane) noexcept;
    static Matrix4x4 CreateOrthographicOffCenter(float left, float right, float bottom, float top, float zNearPlane, float zFarPlane) noexcept;

    static Matrix4x4 CreateLookAt(const Vector3& position, const Vector3& target, const Vector3& up) noexcept;
    static Matrix4x4 CreateWorld(const Vector3& position, const Vector3& forward, const Vector3& up) noexcept;

    static Matrix4x4 CreateFromQuaternion(const Quaternion& quat) noexcept;

    // Rotates about y-axis (yaw), then x-axis (pitch), then z-axis (roll)
    static Matrix4x4 CreateFromYawPitchRoll(float yaw, float pitch, float roll) noexcept;

    // Rotates about y-axis (angles.y), then x-axis (angles.x), then z-axis (angles.z)
    static Matrix4x4 CreateFromYawPitchRoll(const Vector3& angles) noexcept;

    static Matrix4x4 CreateShadow(const Vector3& lightDir, const Plane& plane) noexcept;

    static Matrix4x4 CreateReflection(const Plane& plane) noexcept;

    static void Lerp(const Matrix4x4& M1, const Matrix4x4& M2, float t, Matrix4x4& result) noexcept;
    static Matrix4x4 Lerp(const Matrix4x4& M1, const Matrix4x4& M2, float t) noexcept;

    static void Transform(const Matrix4x4& M, const Quaternion& rotation, Matrix4x4& result) noexcept;
    static Matrix4x4 Transform(const Matrix4x4& M, const Quaternion& rotation) noexcept;

public:
    // Common Values
    static const Matrix4x4 Identity;
};

MATHF_MATRIX_BINARY_OPERATOR_DECLARATIONS(Matrix4x4);

} // namespace gore