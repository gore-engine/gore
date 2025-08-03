#pragma once

#include "Scene/Scene.h"
#include "Object/Camera.h"
#include "Object/GameObject.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

namespace gore::renderer
{
void RenderRootGameObject(const GameObject* rootNode);
void RenderGameObjectComponents(const GameObject* gameObject);
} // namespace gore::renderer