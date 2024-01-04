#pragma once

#include "Export.h"

#include <type_traits>
#include <concepts>

namespace gore
{

ENGINE_CLASS(Component);

template <typename T>
concept IsComponentOrDerivedType = std::derived_from<T, Component>;

}