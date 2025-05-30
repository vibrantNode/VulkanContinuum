#pragma once
#include "vk_device.h"
#include "VK_abstraction/vk_buffer.h"
#include "VK_abstraction/vk_texture.h"
// libs
#define GLM_FORCE_RADIANS	
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>


// std
#include <memory>
#include <vector>
#include <unordered_map>



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

            bool operator==(Vertex const& o) const {
                return position == o.position &&
                    normal == o.normal &&
                    uv == o.uv;
            }
        };

        struct SkyboxVertex {
            glm::vec3 position;
            static std::vector<VkVertexInputBindingDescription>   getBindingDescriptions();
            static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
        };

        struct Builder {
            std::vector<Vertex> vertices{};
            std::vector<SkyboxVertex> skyboxVertices{};
            std::vector<uint32_t> indices{};

            void loadModel(const std::string& filepath, bool isSkybox);

            bool isSkybox{ false };
     
        };

        static std::shared_ptr<VkcModel> createModelFromFile(
            VkcDevice& device, std::string const& filepath, bool isSkybox = false);

        VkcModel(VkcDevice& device, Builder const& builder);
        ~VkcModel();

        VkcModel(VkcModel const&) = delete;
        void operator=(VkcModel const&) = delete;

        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);


        bool isSkybox() const { return isSkyboxModel; }
    private:
        void createVertexBuffers(std::vector<Vertex> const& vertices);
        void createIndexBuffers(std::vector<uint32_t> const& indices);

        void createVertexBuffers(const std::vector<SkyboxVertex>& vertices);

        VkcDevice& vkcDevice;
        bool hasIndexBuffer{ false };
        bool isSkyboxModel{ false };

        std::unique_ptr<VkcBuffer> vertexBuffer;
        uint32_t vertexCount;

        std::unique_ptr<VkcBuffer> indexBuffer;
        uint32_t indexCount;
    };


} // namespace vkc

