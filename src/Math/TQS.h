#pragma once

#include "Export.h"

#include "Math/Defines.h"

#include "Math/Vector3.h"
#include "Math/Quaternion.h"

#include "rtm/qvvf.h"

namespace gore
{

struct Vector3;
struct Matrix4x4;

ENGINE_STRUCT(TQS)
{
    Vector3 t;
    Quaternion q;
    Vector3 s;

    SHALLOW_COPYABLE(TQS);

    explicit TQS() noexcept;
    TQS(const Vector3& translation, const Quaternion& rotation, const Vector3& scale) noexcept;

public:
    [[nodiscard]] inline Matrix4x4 ToMatrix4x4() const noexcept;
    [[nodiscard]] inline Matrix4x4 ToMatrix4x4Inverse() const noexcept;

public:
    [[nodiscard]] static TQS CreateIdentity() noexcept;
    [[nodiscard]] static inline TQS Create(const Vector3& translation, const Quaternion& rotation, const Vector3& scale) noexcept;
    [[nodiscard]] static inline TQS CreateInverse(const Vector3& translation, const Quaternion& rotation, const Vector3& scale) noexcept;

public:
    [[nodiscard]] inline TQS Inverse() const noexcept;
    [[nodiscard]] inline Vector3 MulPoint3(const Vector3& v, bool useScale = true) const noexcept;
    [[nodiscard]] inline Vector3 MulVector3(const Vector3& v, bool useScale = true) const noexcept;

    [[nodiscard]] inline Vector3 InvMulPoint3(const Vector3& v, bool useScale = true) const noexcept;
    [[nodiscard]] inline Vector3 InvMulVector3(const Vector3& v, bool useScale = true) const noexcept;

    [[nodiscard]] static inline TQS Mul(const TQS& a, const TQS& b) noexcept;
    [[nodiscard]] static inline TQS Blend(const TQS& a, const TQS& b, float t) noexcept;
};


TQS::TQS() noexcept :
    t(Vector3::Zero),
    q(Quaternion::Identity),
    s(Vector3::One)
{
}

TQS::TQS(const Vector3& translation, const Quaternion& rotation, const Vector3& scale) noexcept :
    t(translation),
    q(rotation),
    s(scale)
{
}

TQS TQS::CreateIdentity() noexcept
{
    return TQS{};
}

TQS TQS::Create(const Vector3& translation, const Quaternion& rotation, const Vector3& scale) noexcept
{
    return TQS{translation, rotation, scale};
}

TQS TQS::CreateInverse(const Vector3& translation, const Quaternion& rotation, const Vector3& scale) noexcept
{
    using namespace rtm;
    qvvf qvvInv = qvv_inverse(qvv_set(rotation, translation, scale));
    return TQS{static_cast<Vector3>(qvvInv.translation),
               static_cast<Quaternion>(qvvInv.rotation),
               static_cast<Vector3>(qvvInv.scale)};
}

Matrix4x4 TQS::ToMatrix4x4() const noexcept
{
    return Matrix4x4::FromTRS(t, q, s);
}

Matrix4x4 TQS::ToMatrix4x4Inverse() const noexcept
{
    using namespace rtm;
    qvvf qvvInv = qvv_inverse(qvv_set(q, t, s));
    return CAST_FROM_SIMD_MATRIX_HELPER(Matrix4x4, matrix_from_qvv(qvvInv));
}

TQS TQS::Inverse() const noexcept
{
    using namespace rtm;
    qvvf qvvInv = qvv_inverse(qvv_set(q, t, s));
    return TQS{static_cast<Vector3>(qvvInv.translation),
               static_cast<Quaternion>(qvvInv.rotation),
               static_cast<Vector3>(qvvInv.scale)};
}

Vector3 TQS::MulPoint3(const Vector3& v, bool useScale) const noexcept
{
    using namespace rtm;
    auto vSIMD = static_cast<Vector3::SIMDValueType>(v);
    qvvf qvv   = qvv_set(q, t, s);

    return static_cast<Vector3>(useScale ?
                                    qvv_mul_point3(vSIMD, qvv) :
                                    qvv_mul_point3_no_scale(vSIMD, qvv));
}

Vector3 TQS::MulVector3(const Vector3& v, bool useScale) const noexcept
{
    using namespace rtm;
    return MulPoint3(v, useScale) - t;
}

Vector3 TQS::InvMulPoint3(const Vector3& v, bool useScale) const noexcept
{
    using namespace rtm;
    qvvf qvvInv = qvv_inverse(qvv_set(q, t, s));
    auto vSIMD  = static_cast<Vector3::SIMDValueType>(v);
    return static_cast<Vector3>(useScale ?
                                    qvv_mul_point3(vSIMD, qvvInv) :
                                    qvv_mul_point3_no_scale(vSIMD, qvvInv));
}

Vector3 TQS::InvMulVector3(const Vector3& v, bool useScale) const noexcept
{
    using namespace rtm;
    return InvMulPoint3(v, useScale) - t;
}


TQS TQS::Mul(const TQS& a, const TQS& b) noexcept
{
    using namespace rtm;
    qvvf qvvA = qvv_set(a.q, a.t, a.s);
    qvvf qvvB = qvv_set(b.q, b.t, b.s);
    qvvf qvv  = qvv_mul(qvvA, qvvB);

    return TQS{static_cast<Vector3>(qvv.translation),
               static_cast<Quaternion>(qvv.rotation),
               static_cast<Vector3>(qvv.scale)};
}

TQS TQS::Blend(const TQS& a, const TQS& b, float t) noexcept
{
    using namespace rtm;
    qvvf qvvA = qvv_set(a.q, a.t, a.s);
    qvvf qvvB = qvv_set(b.q, b.t, b.s);
    qvvf qvv  = qvv_slerp(qvvA, qvvB, t);

    return TQS{static_cast<Vector3>(qvv.translation),
               static_cast<Quaternion>(qvv.rotation),
               static_cast<Vector3>(qvv.scale)};
}

} // namespace gore