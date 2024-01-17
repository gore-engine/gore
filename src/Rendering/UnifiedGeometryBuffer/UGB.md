<a id="top"></a>
# Unified Geometry Buffer (UGB)

Like most modern game engines, the renderer in gore use a unified vertex buffer and index buffer to store all the geometry data. This is called the Unified Geometry Buffer (UGB). 

There are several important SSBO buffers in our design:
* Instance Buffer:\
  This buffer stores the instance data for each object. The instance data is defined as follows:
  ```hlsl
  struct InstanceData 
  {
    float4x4    modelMatrix;
    uint        meshIndex; // index to MeshBuffer
    uint3       padding;
  };
  ```
* Mesh Buffer:\
    This buffer stores the mesh data for each object. The mesh data is defined as follows:
    ```hlsl
    struct MeshData 
    {
        uint        vertexOffset; // index to VertexBuffer
        uint        indexOffset;  // index to IndexBuffer
        uint        vertexCount;
        uint        indexCount;
    };
    ```
* Index Buffer:\
    tmp index buffer, may be removed in the future.
* Vertex Buffer:\
    tmp vertex buffer, may be removed in the future.