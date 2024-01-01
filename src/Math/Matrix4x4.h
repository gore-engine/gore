#pragma once

#include "Export.h"

#include "Math/Defines.h"

#include "rtm/matrix4x4f.h"

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
    // This is exactly how XMFLOAT4X4 declares its members
    // Yes its constructor names parameters starting from "m00", which is different
    // Since I'm simply copying calculations from SimpleMath.h (after removing its dependencies to types in DirectXMath)
    // I'm keeping it this way
    union
    {
        struct
        {
            float _11, _12, _13, _14;
            float _21, _22, _23, _24;
            float _31, _32, _33, _34;
            float _41, _42, _43, _44;
        };
        float m[4][4];
    };

public:
    MATHF_SIMD_SET_VALUE_TYPE(rtm::matrix4x4f);
    MATHF_SIMD_CONVERSION_WITH_VALUE_TYPE_DECLARATIONS(Matrix4x4);

    SHALLOW_COPYABLE(Matrix4x4);

    MATHF_COMMON_UNARY_OPERATOR_DECLARATIONS(Matrix4x4);
    MATHF_MATRIX_COMPARISON_OPERATOR_DECLARATIONS(Matrix4x4);
    MATHF_MATRIX_COMPOUND_ASSIGNMENT_OPERATOR_DECLARATIONS(Matrix4x4);

    // clang-format off
    Matrix4x4() noexcept = default;
    constexpr Matrix4x4(
        float m00, float m01, float m02, float m03,
        float m10, float m11, float m12, float m13,
        float m20, float m21, float m22, float m23,
        float m30, float m31, float m32, float m33) noexcept :
        _11(m00), _12(m01), _13(m02), _14(m03),
        _21(m10), _22(m11), _23(m12), _24(m13),
        _31(m20), _32(m21), _33(m22), _34(m23),
        _41(m30), _42(m31), _43(m32), _44(m33)
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

    static Matrix4x4 CreatePerspectiveFieldOfView(float fov, float aspectRatio, float nearPlane, float farPlane) noexcept;
    static Matrix4x4 CreatePerspective(float width, float height, float nearPlane, float farPlane) noexcept;
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