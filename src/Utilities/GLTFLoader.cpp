#define TINYGLTF_IMPLEMENTATION
#include "GLTFLoader.h"

#include "Rendering/Components/MeshRenderer.h"
#include "Rendering/RenderContext.h"

namespace gore::gfx
{
GLTFLoader::GLTFLoader(RenderContext& rtx) :
    m_RenderContext(rtx)
{
}

GLTFLoader::~GLTFLoader()
{
}

bool GLTFLoader::LoadMesh(MeshRenderer& mesh, const std::string& path, int meshIndex, ShaderChannel channels)
{
    std::string error;
    std::string warning;

    tinygltf::Model model;

    tinygltf::TinyGLTF gltf;
    bool importResult = gltf.LoadASCIIFromFile(&model, &error, &warning, path);
    if (importResult == false)
    {
        LOG_STREAM(ERROR) << "Failed to load GLTF file: " << path << std::endl;
        return false;
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

    auto pos             = path.find_last_of('/');
    std::string meshName = path.substr(pos + 1, path.size() - pos - 1);

    return CreateMeshFromGLTF(mesh, model, meshIndex, meshName, channels);
}

static GraphicsFormat GetIndexDataFormat(const tinygltf::Accessor& accessor)
{
    switch (accessor.componentType)
    {
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
            return GraphicsFormat::R8_UINT;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
            return GraphicsFormat::R16_UINT;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
            return GraphicsFormat::R32_UINT;
        default:
            assert(false);
            return GraphicsFormat::Undefined;
    }
}

bool GLTFLoader::CreateMeshFromGLTF(MeshRenderer& mesh, const tinygltf::Model& model, int meshIndex, const std::string& name, ShaderChannel channels)
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
    mesh.SetVertexCount(vertexCount);

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

    std::string vertexBufferName = name + "_VertexBuffer";

    BufferHandle vertexBuffer = m_RenderContext.CreateBuffer({
        .debugName = vertexBufferName.c_str(),
        .byteSize  = (uint32_t)(vertexData.size() * sizeof(Vertex)),
        .usage     = BufferUsage::Vertex,
        .data      = vertexData.data(),
    });

    IndexType indexType = IndexType::None;
    std::vector<uint8_t> indexData;
    if (gltfPrimitive.indices >= 0)
    {
        const tinygltf::Accessor& accessor     = model.accessors[gltfPrimitive.indices];
        const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
        const tinygltf::Buffer& buffer         = model.buffers[bufferView.buffer];

        mesh.SetIndexCount(accessor.count);

        const void* data = &buffer.data[bufferView.byteOffset + accessor.byteOffset];
        size_t size      = accessor.count * accessor.ByteStride(bufferView);

        auto format = GetIndexDataFormat(accessor);
        // Convert UINT8 to UINT16
        if (format == GraphicsFormat::R8_UINT)
        {
            format = GraphicsFormat::R16_UINT;
            indexData.resize(size * 2);
            for (size_t i = 0; i < size; ++i)
            {
                indexData[i * 2]     = ((uint8_t*)data)[i];
                indexData[i * 2 + 1] = 0;
            }
        }
        else
        {
            indexData.resize(size);
            memcpy(indexData.data(), data, size);
        }

        indexType   = GetIndexTypeByGraphicsFormat(format);
    }

    BufferHandle indexBuffer = m_RenderContext.CreateBuffer({
        .debugName = (name + "_IndexBuffer").c_str(),
        .byteSize  = (uint32_t)(indexData.size() * GetIndexTypeSize(indexType)),
        .usage     = BufferUsage::Index,
        .data      = indexData.data(),
    });

    mesh.SetVertexBuffer(vertexBuffer);
    mesh.SetVertexCount(vertexCount);

    mesh.SetIndexBuffer(indexBuffer);
    mesh.SetIndexType(indexType);

    return true;
}
} // namespace gore::gfx