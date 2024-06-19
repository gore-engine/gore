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

std::unique_ptr<Mesh> GLTFLoader::LoadMesh(const std::string& path, int meshIndex, ShaderChannel channels)
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

std::unique_ptr<Mesh> GLTFLoader::CreateMeshFromGLTF(const tinygltf::Model& model, int meshIndex, ShaderChannel channels)
{
    // FIXME: only support default vertex now
    assert(channels == ShaderChannel::Default);

    assert(meshIndex >= 0 && meshIndex < model.meshes.size());

    auto& gltfPrimitive = model.meshes[meshIndex].primitives[0];

    const float* pos     = nullptr;
    const float* normals = nullptr;
    const float* uvs     = nullptr;

    bool needPosition = HasFlag(channels, ShaderChannel::Position);
    bool hasPosition  = gltfPrimitive.attributes.find("POSITION") != gltfPrimitive.attributes.end();
    assert(needPosition && hasPosition);

    const tinygltf::Accessor& accessor = model.accessors[gltfPrimitive.attributes.find("POSITION")->second];

    int vertexCount = accessor.count;

    const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
    const tinygltf::Buffer& buffer         = model.buffers[bufferView.buffer];

    pos = reinterpret_cast<const float*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);

    bool hasNormals = gltfPrimitive.attributes.find("NORMAL") != gltfPrimitive.attributes.end();
    if (hasNormals)
    {
        const tinygltf::Accessor& accessor     = model.accessors[gltfPrimitive.attributes.find("NORMAL")->second];
        const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
        const tinygltf::Buffer& buffer         = model.buffers[bufferView.buffer];

        normals = reinterpret_cast<const float*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
    }

    bool hasUVs = gltfPrimitive.attributes.find("TEXCOORD_0") != gltfPrimitive.attributes.end();
    if (hasUVs)
    {
        const tinygltf::Accessor& accessor     = model.accessors[gltfPrimitive.attributes.find("TEXCOORD_0")->second];
        const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
        const tinygltf::Buffer& buffer         = model.buffers[bufferView.buffer];

        uvs = reinterpret_cast<const float*>(&buffer.data[bufferView.byteOffset + accessor.byteOffset]);
    }

    std::vector<Vertex> vertexData;
    vertexData.reserve(vertexCount);

    for (int i = 0; i < vertexCount; ++i)
    {
        Vertex vertex;
        vertex.position = Vector3(pos[0], pos[1], pos[2]);
        pos += 3;

        if (hasNormals)
        {
            vertex.normal = Vector3(normals[0], normals[1], normals[2]);
            normals += 3;
        }
        else
        {
            vertex.normal = k_DefaultNormal;
        }

        if (hasUVs)
        {
            vertex.uv = Vector2(uvs[0], uvs[1]);
            uvs += 2;
        }
        else
        {
            vertex.uv = k_DefaultUV;
        }

        vertexData.push_back(vertex);
    }


    return std::unique_ptr<Mesh>();
}
} // namespace gore::gfx