#include "Prefix.h"

#include "System.h"

#include "Core/App.h"

namespace gore
{

std::map<std::string, System*> System::s_RegisteredSystems;

System::System(App* app) :
    m_App(app)
{
}

System::~System()
{
}

void System::Register(const std::string& name)
{
    s_RegisteredSystems[name] = this;
}

System* System::Get(const std::string& name)
{
    auto it = s_RegisteredSystems.find(name);
    if (it != s_RegisteredSystems.end())
    {
        return it->second;
    }
    return nullptr;
}

std::vector<System*> System::GetAll()
{
    std::vector<System*> systems;
    systems.reserve(s_RegisteredSystems.size());
    for (auto& it : s_RegisteredSystems)
    {
        systems.push_back(it.second);
    }
    return systems;
}

} // namespace gore
