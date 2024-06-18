#define TINYGLTF_IMPLEMENTATION
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

std::unique_ptr<Mesh> GLTFLoader::LoadMesh(const std::string& path, int meshIndex, MeshChannel channels)
{
    std::string error;
    std::string warning;

    tinygltf::Model model;

    tinygltf::TinyGLTF gltf;
    bool importResult = gltf.LoadASCIIFromFile(&model, &error, &warning, path);
    if (importResult == false)
    {
        LOG_STREAM(ERROR) << "Failed to load GLTF file: " << path << std::endl;
        return nullptr;
    }

    if (error.empty() == false)
    {
        LOG_STREAM(ERROR) << "Error loading GLTF file: " << path << std::endl;
        LOG_STREAM(ERROR) << error << std::endl;
    }

    if (warning.empty() == false)
    {
        LOG_STREAM(WARNING) << "Warning loading GLTF file: " << path << std::endl;
        LOG_STREAM(WARNING) << warning << std::endl;
    }

    return std::move(CreateMeshFromGLTF(model, meshIndex, channels));
}

std::unique_ptr<Mesh> GLTFLoader::CreateMeshFromGLTF(const tinygltf::Model& model, int meshIndex, MeshChannel channels)
{
    return std::unique_ptr<Mesh>();
}
} // namespace gore::gfx