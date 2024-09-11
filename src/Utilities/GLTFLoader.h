#pragma once
#include "Prefix.h"
#include "Export.h"

#define TINYGLTF_NO_STB_IMAGE
#define TINYGLTF_NO_STB_IMAGE_WRITE
#define TINYGLTF_NO_EXTERNAL_IMAGE
#include <tiny_gltf.h>

#include "Rendering/Utils/GeometryUtils.h"
#include "Rendering/Components/MeshRenderer.h"

#include <memory>

namespace gore::gfx
{
using namespace gore::renderer;

class RenderContext;

ENGINE_CLASS(GLTFLoader)
{
public:
    GLTFLoader(RenderContext & rtx);
    ~GLTFLoader();

    [[nodiscard]] bool LoadMesh(MeshRenderer & mesh, const std::string& path, int meshIndex = 0, ShaderChannel channels = ShaderChannel::Default);

private:
    [[nodiscard]] bool CreateMeshFromGLTF(MeshRenderer & mesh, const tinygltf::Model& model, int meshIndex, const std::string& name, ShaderChannel channels);

    RenderContext & m_RenderContext;
};
} // namespace gore::gfx