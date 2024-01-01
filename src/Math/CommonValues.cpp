#pragma once

#include "Math/Vector2.h"
#include "Math/Vector3.h"
#include "Math/Vector4.h"
#include "Math/Quaternion.h"
#include "Math/Matrix4x4.h"

namespace gore
{

const Vector2 Vector2::Zero  = Vector2(0.f, 0.f);
const Vector2 Vector2::One   = Vector2(1.f, 1.f);
const Vector2 Vector2::UnitX = Vector2(1.f, 0.f);
const Vector2 Vector2::UnitY = Vector2(0.f, 1.f);

const Vector3 Vector3::Zero     = Vector3(0.f, 0.f, 0.f);
const Vector3 Vector3::One      = Vector3(1.f, 1.f, 1.f);
const Vector3 Vector3::UnitX    = Vector3(1.f, 0.f, 0.f);
const Vector3 Vector3::UnitY    = Vector3(0.f, 1.f, 0.f);
const Vector3 Vector3::UnitZ    = Vector3(0.f, 0.f, 1.f);
const Vector3 Vector3::Up       = Vector3(0.f, 1.f, 0.f);
const Vector3 Vector3::Down     = Vector3(0.f, -1.f, 0.f);
const Vector3 Vector3::Right    = Vector3(1.f, 0.f, 0.f);
const Vector3 Vector3::Left     = Vector3(-1.f, 0.f, 0.f);
const Vector3 Vector3::Forward  = Vector3(0.f, 0.f, -1.f);
const Vector3 Vector3::Backward = Vector3(0.f, 0.f, 1.f);

const Vector4 Vector4::Zero  = Vector4(0.f, 0.f, 0.f, 0.f);
const Vector4 Vector4::One   = Vector4(1.f, 1.f, 1.f, 1.f);
const Vector4 Vector4::UnitX = Vector4(1.f, 0.f, 0.f, 0.f);
const Vector4 Vector4::UnitY = Vector4(0.f, 1.f, 0.f, 0.f);
const Vector4 Vector4::UnitZ = Vector4(0.f, 0.f, 1.f, 0.f);
const Vector4 Vector4::UnitW = Vector4(0.f, 0.f, 0.f, 1.f);

const Matrix4x4 Matrix4x4::Identity = Matrix4x4(1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f, 0.f, 0.f, 0.f, 0.f, 1.f);

const Quaternion Quaternion::Identity = {0.f, 0.f, 0.f, 1.f};

} // namespace gore