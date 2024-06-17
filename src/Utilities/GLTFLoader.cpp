#include "GLTFLoader.h"

#include "Rendering/Components/Mesh.h"

namespace gore::gfx
{
GLTFLoader::GLTFLoader(RenderContext& rtx) :
    m_RenderContext(rtx)
{
}

GLTFLoader::~GLTFLoader()
{
}

std::unique_ptr<Mesh> LoadMesh(const std::string& path, int meshIndex)
{
    std::string error;
    std::string warning;

    tinygltf::Model model;

    tinygltf::TinyGLTF gltf;
    // std::string gltfImporter = gltf.LoadASCIIFromFile()

    return std::unique_ptr<Mesh>();
}
} // namespace gore::gfx