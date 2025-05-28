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

        struct Builder {
            std::vector<Vertex> vertices{};
            std::vector<uint32_t> indices{};

            void loadModel(const std::string& filepath);

     
        };

        static std::shared_ptr<VkcModel> createModelFromFile(
            VkcDevice& device, std::string const& filepath);

        VkcModel(VkcDevice& device, Builder const& builder);
        ~VkcModel();

        VkcModel(VkcModel const&) = delete;
        void operator=(VkcModel const&) = delete;

        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);

    private:
        void createVertexBuffers(std::vector<Vertex> const& vertices);
        void createIndexBuffers(std::vector<uint32_t> const& indices);

        VkcDevice& vkcDevice;
        bool hasIndexBuffer{ false };

        std::unique_ptr<VkcBuffer> vertexBuffer;
        uint32_t vertexCount;

        std::unique_ptr<VkcBuffer> indexBuffer;
        uint32_t indexCount;
    };

} // namespace vkc