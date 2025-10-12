#include "GPUScene.h"

#include "Object/GameObject.h"
#include "Object/Transform.h"

#include "Rendering/Components/MeshRenderer.h"

namespace gore::gfx
{
void ConstructGPUSceneData(
    const std::vector<class gore::GameObject*>& gameObjects,
    GPUScene& outScene)
{
    for (const auto& gameObject : gameObjects)
    {
        // Process each game object and extract relevant data
        // For example:
        // - Get material data
        // - Get geometry data
        // - Get instance data

        Transform* transform = gameObject->GetComponent<Transform>();
        renderer::MeshRenderer* meshRenderer = gameObject->GetComponent<renderer::MeshRenderer>();
        if (transform == nullptr || meshRenderer == nullptr)
            continue;

        

        
        // Add the extracted data to the outScene
    }
}
} // namespace gore::gfx