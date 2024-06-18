#pragma once
#include "Prefix.h"
#include "Export.h"
#include "tiny_gltf.h"

#include <memory>

namespace gore::gfx
{
class RenderContext;
class Mesh;
class Material;

ENGINE_CLASS(GLTFLoader)
{
public:
    GLTFLoader(RenderContext & rtx);
    ~GLTFLoader();

    [[nodiscard]] std::unique_ptr<Mesh> LoadMesh(const std::string& path, int meshIndex = 0);

private:
    [[nodiscard]] std::unique_ptr<Mesh> CreateMeshFromGLTF(const tinygltf::Model& model, int meshIndex);

    RenderContext & m_RenderContext;
};
} // namespace gore::gfx