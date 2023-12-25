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

    NON_COPYABLE(Object);

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
