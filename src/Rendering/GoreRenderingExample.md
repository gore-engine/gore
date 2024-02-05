# Gore Rendering Loop
Easy and efficient rendering loop using gore.

## Introduction
This is a simple example of how to use gore to write efficient rendering code under the ideal conditions. This example is not meant to be a complete rendering loop, but rather a starting point for your own rendering loop.

### OnInit
The `OnInit` function is called once when the application starts. This is where you should initialize your rendering resources, such as shaders, textures, and buffers

```cpp
void OnInit()
{
    // RenderPass
    m_RenderPass = m_RenderContext->CreateRenderPass("GBuffer");

    // Pipeline = Shader Pass in Unity
    m_Pipeline0 = m_RenderContext->CreatePipeline(...);
    m_Pipeline1 = m_RenderContext->CreatePipeline(...);

    // Mesh
    // auto generate mesh buffers from file, maybe upload to GPU later
    m_Mesh = m_RenderContext->ImportMesh("path/to/mesh.obj");
}
```

### OnRender
The `OnRender` function is called every frame and whole pipeline runs here. This is where you should issue draw calls to the GPU.

```cpp
void OnRender()
{
    // Bind RenderPass
    m_RenderContext->BeginRenderPass(m_RenderPass);

    // This Part is DrawCall recording
    {
        // Draw
        m_RenderContext->DrawMesh(m_Mesh, m_Pipeline0);

        // Indirect Draw
        m_RenderContext->DrawMeshIndirect(m_Mesh, m_Pipeline1, m_IndirectBuffer, m_IndirectOffset);

        // Procedural Draw
        m_RenderContext->DrawMeshProcedural(m_Pipeline0, m_ProceduralMesh, m_ProceduralInstanceCount, m_ProceduralInstanceDataBuffer, m_ProceduralInstanceDataOffset);
    }

    // End RenderPass, sort draw calls in the other thread
    m_RenderContext->EndRenderPass();

    // Compute RenderPass
    m_RenderContext->BeginRenderPass(m_ComputePass);

    m_RenderContext->DispatchCompute(m_ComputePipeline, m_DispatchSize);
    
    m_RenderContext->EndRenderPass();
}
```

### OnShutdown
The `OnShutdown` function is called once when the application exits. This is where you should release your rendering resources.

```cpp
void OnShutdown()
{
    m_RenderContext->ReleaseRenderPass(m_RenderPass);
    m_RenderContext->ReleasePipeline(m_Pipeline0);
    m_RenderContext->ReleasePipeline(m_Pipeline1);
    m_RenderContext->ReleaseMesh(m_Mesh);
}
```