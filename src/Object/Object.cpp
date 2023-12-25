#include "Prefix.h"

#include "Object.h"

#include <utility>

namespace gore
{

Object::Object(std::string name) :
    m_Name(std::move(name))
{
}

Object::~Object() = default;

} // namespace gore
