#include "Prefix.h"

#include "GameObject.h"
#include "Scene/Scene.h"

#include <utility>
#include <iostream>

namespace gore
{

GameObject::GameObject(std::string name, Scene* scene) :
    Object(std::move(name)),
    m_Scene(scene)
{
}

GameObject::~GameObject()
{
}

void GameObject::Update()
{
}

}