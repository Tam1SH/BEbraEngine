﻿module;
#include <vulkan/vulkan.h>
#include <array>;
#include <vector>;
export module Vertex;
import Vector2;
import Vector4;


namespace BEbraEngine {
    
    //Импортишь вектор 3? Компайл ерор. А мб это всё из-за CVector3, кто его знает
    export struct UnbelivableVector3FromMSVC {
        int x, y, z;
        UnbelivableVector3FromMSVC(int x, int y, int z) : x(x), y(y), z(z) { }
        UnbelivableVector3FromMSVC() {}
    };

    export struct Vertex {

        UnbelivableVector3FromMSVC position;
        Vector4 color;
        Vector2 texCoord;
        UnbelivableVector3FromMSVC Normal;
        //Vector3 Tangent;
        //Vector3 Bitangent;
        static VkVertexInputBindingDescription getBindingDescription() {
            VkVertexInputBindingDescription bindingDescription{};
            bindingDescription.binding = 0;
            bindingDescription.stride = sizeof(Vertex);
            bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            return bindingDescription;
        }

        static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions() {
            std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
            attributeDescriptions.resize(4);

            attributeDescriptions[0].binding = 0;
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[0].offset = offsetof(Vertex, position);

            attributeDescriptions[1].binding = 0;
            attributeDescriptions[1].location = 1;
            attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
            attributeDescriptions[1].offset = offsetof(Vertex, color);

            attributeDescriptions[2].binding = 0;
            attributeDescriptions[2].location = 2;
            attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
            attributeDescriptions[2].offset = offsetof(Vertex, texCoord);
            
            attributeDescriptions[3].binding = 0;
            attributeDescriptions[3].location = 3;
            attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[3].offset = offsetof(Vertex, Normal);
            /*
            attributeDescriptions[4].binding = 0;
            attributeDescriptions[4].location = 4;
            attributeDescriptions[4].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[4].offset = offsetof(Vertex, Tangent);

            attributeDescriptions[5].binding = 0;
            attributeDescriptions[5].location = 5;
            attributeDescriptions[5].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[5].offset = offsetof(Vertex, Bitangent);
            */
            return attributeDescriptions;
        }
    };
    
}