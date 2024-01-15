#pragma once

#include "Prefix.h"


#include "Graphics/Vulkan/VulkanIncludes.h"
#include "Graphics/Vulkan/VulkanExtensions.h"

#include <glm/glm.hpp>
#include <vector>

struct Vertex
{
    glm::vec3 position;
    // glm::vec3 color;
    // glm::vec2 uv;

    static vk::VertexInputBindingDescription getBindingDescription()
    {
        vk::VertexInputBindingDescription binding;
        binding.binding = 0;
        binding.stride = sizeof(Vertex);
        binding.inputRate = vk::VertexInputRate::eVertex;
        return binding;
    }

    static std::array<vk::VertexInputAttributeDescription, 1> getAttributeDescriptions()
    {
        std::array<vk::VertexInputAttributeDescription, 1> attributes;
        attributes[0].binding = 0;
        attributes[0].location = 0;
        attributes[0].format = vk::Format::eR32G32B32Sfloat;
        attributes[0].offset = offsetof(Vertex, position);

        return attributes;
    }
};