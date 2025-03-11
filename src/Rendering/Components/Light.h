#pragma once

#include "Prefix.h"
#include "Export.h"

#include "Object/Component.h"
#include "Math/Vector3.h"
#include "Math/Vector4.h"

namespace gore
{
enum LightType
{
    Directional = 0,
    Point       = 1,
    Spot        = 2
};

struct LightData
{
    Vector3 direction{0.0f, 0.0f, -1.0f};
    float range{0.0f};
    Vector4 color{0.0f, 0.0f, 0.0f, 0.0f};
    float intensity{0.0f};
};

ENGINE_CLASS(Light) final : public Component
{
public:
    NON_COPYABLE(Light)

    explicit Light(GameObject * GameObject) noexcept;
    ~Light() override;

    void Start() override;
    void Update() override;

    GETTER_SETTER(LightType, Type)
    GETTER_SETTER(LightData, Data)

private:
    LightType m_Type;
    LightData m_Data;
};
} // namespace gore