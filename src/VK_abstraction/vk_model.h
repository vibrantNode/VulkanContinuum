#pragma once
#include "vk_device.h"
#include "Renderer/vk_buffer.h"

// libs
#define GLM_FORCE_RADIANS	
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>


// std
#include <memory>
#include <vector>

namespace vkc {

    class VkcModel {
    public:
        struct Vertex {
            glm::vec3 position{};
            glm::vec3 color{};
            glm::vec3 normal{};
            glm::vec2 uv{};

            static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
            static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

            bool operator==(const Vertex& other) const { return position == other.position && normal == other.normal && uv == other.uv; }
        };

        struct Builder {
            std::vector<Vertex> vertices{};
            std::vector<uint32_t> indices{};

            void loadModel(const std::string& filepath);
        };




        static std::unique_ptr<VkcModel> createModelFromFile(VkcDevice& device, const std::string& filepath);


        VkcModel::VkcModel(VkcDevice& device, const VkcModel::Builder& builder);
        ~VkcModel();

        VkcModel(const VkcModel&) = delete;
        void operator=(const VkcModel&) = delete;

        void draw(VkCommandBuffer commandBuffer);
        void bind(VkCommandBuffer commandBuffer);

    private:
        void createVertexBuffers(const std::vector<Vertex>& vertices);
        void createIndexBuffers(const std::vector<uint32_t>& indices);

        bool hasIndexBuffer = false;
        VkcDevice& vkcDevice;


        std::unique_ptr<VkcBuffer> vertexBuffer;
        uint32_t vertexCount;

        std::unique_ptr<VkcBuffer> indexBuffer;
        uint32_t indexCount;
    };
}  // namespace vkc