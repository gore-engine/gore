#pragma once
#include "Prefix.h"
#include "Export.h"

#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_EXTERNAL_IMAGE
#include <tiny_gltf.h>

#include "Rendering/Utils/GeometryUtils.h"

#include <memory>

using namespace gore::gfx::geometry;

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

    [[nodiscard]] std::unique_ptr<Mesh> LoadMesh(const std::string& path, int meshIndex = 0, ShaderChannel channels = ShaderChannel::Default);

private:
    [[nodiscard]] std::unique_ptr<Mesh> CreateMeshFromGLTF(const tinygltf::Model& model, int meshIndex, ShaderChannel channels);

    RenderContext & m_RenderContext;
};
} // namespace gore::gfx