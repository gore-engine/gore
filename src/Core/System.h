#pragma once

#include "Export.h"

#include <string>
#include <map>
#include <vector>

namespace gore
{

class App;

ENGINE_CLASS(System)
{
public:
    explicit System(App* app);
    virtual ~System();

    System(const System&)            = delete;
    System& operator=(const System&) = delete;
    System(System&&)                 = delete;
    System& operator=(System&&)      = delete;

    virtual void Initialize() = 0;
    virtual void Update()     = 0;
    virtual void Shutdown()   = 0;

    void Register(const std::string& name);
    static System* Get(const std::string& name);

private:
    friend class App;
    static std::map<std::string, System*> s_RegisteredSystems; // TODO: lock?
    static std::vector<System*> GetAll();

protected:
    App* m_App;
};

} // namespace gore
