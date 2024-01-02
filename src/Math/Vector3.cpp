#include "Vector3.h"

#include "rtm/vector4f.h"

namespace gore
{

Vector3::operator ValueType() const noexcept
{
    return rtm::vector_load3((reinterpret_cast<const float*>(this)));
}


}