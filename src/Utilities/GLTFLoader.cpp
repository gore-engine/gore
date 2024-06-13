#include "GLTFLoader.h"

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
    return std::unique_ptr<Mesh>();
}
} // namespace gore::gfx