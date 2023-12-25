#pragma once

#include "Export.h"

#include <string>

namespace gore
{

ENGINE_CLASS(Object)
{
public:
    explicit Object(std::string name);
    virtual ~Object();

    Object(const Object&)            = delete;
    Object& operator=(const Object&) = delete;
    Object(Object&&)                 = delete;
    Object& operator=(Object&&)      = delete;

    [[nodiscard]] const std::string& GetName() const
    {
        return m_Name;
    }
    void SetName(std::string name)
    {
        m_Name = std::move(name);
    }

private:
    std::string m_Name;
};

} // namespace gore
