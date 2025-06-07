#pragma once
#include "vk_device.h"
#include "VK_abstraction/vk_buffer.h"
#include "VK_abstraction/vk_texture.h"
// libs
#define GLM_FORCE_RADIANS	
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>


#include <tiny_gltf.h>

// std
#include <memory>
#include <vector>
#include <unordered_map>



namespace vkc {


    class VkcModel
    {
    public:
        virtual void bind(VkCommandBuffer commandBuffer)
        {

        }
        virtual void draw(VkCommandBuffer commandBuffer)
        {

        }
        virtual void  draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout)
        {

        }
        virtual ~VkcModel() = default;
    };

    class VkcOBJmodel : public VkcModel {
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
        struct PrimitiveInfo {
            uint32_t      firstIndex;
            uint32_t      indexCount;
            int           textureIndex; // index into textures[]
            glm::mat4     modelMatrix;
        };
        struct Builder {
            std::vector<Vertex> vertices{};
            std::vector<SkyboxVertex> skyboxVertices{};
            std::vector<uint32_t> indices{};

            void loadModel(const std::string& filepath, bool isSkybox);
   

            bool isSkybox{ false };
     
        };

        static std::shared_ptr<VkcOBJmodel> createModelFromFile(
            VkcDevice& device, std::string const& filepath, bool isSkybox = false);

        VkcOBJmodel(VkcDevice& device, Builder const& builder);
        ~VkcOBJmodel();

        VkcOBJmodel(VkcOBJmodel const&) = delete;
        void operator=(VkcOBJmodel const&) = delete;

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

        std::vector<std::shared_ptr<VkcTexture>> textures;
    };



class VkcGLTFmodel : public VkcModel
    {
    public:


        VkcDevice* m_pDevice;
        VkBuffer m_vkBuffer;
        VkQueue m_vkCopyQueue;

        struct Vertex {
            glm::vec3 pos;
            glm::vec3 normal;
            glm::vec3 uv;
            glm::vec3 color;

            static std::vector<VkVertexInputBindingDescription>   getBindingDescriptions();
            static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

        };

        // Single vertex buffer for all primitives
        struct {
            VkBuffer buffer;
            VkDeviceMemory memory;
        } vertices;

        // Single index buffer for all primitives
        struct {
            int count;
            VkBuffer buffer;
            VkDeviceMemory memory;
        } indices;
        // The following structures roughly represent the glTF scene structure
    // To keep things simple, they only contain those properties that are required for this sample
        struct Node;

        // A primitive contains the data for a single draw call
        struct Primitive {
            uint32_t firstIndex;
            uint32_t indexCount;
            int32_t materialIndex;
        };

        // Contains the node's (optional) geometry and can be made up of an arbitrary number of primitives
        struct Mesh {
            std::vector<Primitive> primitives;
        };

        // A node represents an object in the glTF scene graph
        struct Node {
            Node* parent;
            std::vector<Node*> children;
            Mesh mesh;
            glm::mat4 matrix;
            ~Node() {
                for (auto& child : children) {
                    delete child;
                }
            }
        };

        // A glTF material stores information in e.g. the texture that is attached to it and colors
        struct Material {
            glm::vec4 baseColorFactor = glm::vec4(1.0f);
            uint32_t baseColorTextureIndex;
        };

        // Contains the texture for a single glTF image
        // Images may be reused by texture objects and are as such separated
        struct Image {
            VkcTexture texture;
            // We also store (and create) a descriptor set that's used to access this texture from the fragment shader
            VkDescriptorSet descriptorSet;

           
        };

        // A glTF texture stores a reference to the image and a sampler
        // In this sample, we are only interested in the image
        struct Texture {
            int32_t imageIndex;
        };


        void loadImages(tinygltf::Model& input);
        void loadTextures(tinygltf::Model& input);
        void loadMaterials(tinygltf::Model& input);
        void loadNode(const tinygltf::Node& inputNode, const tinygltf::Model& input, VkcGLTFmodel::Node* parent, std::vector<uint32_t>& indexBuffer, std::vector<VkcGLTFmodel::Vertex>& vertexBuffer);
        void drawNode(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, VkcGLTFmodel::Node* node);
        void draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout)override;
       
  


        std::vector<Image> images;
        std::vector<Texture> textures;
        std::vector<Material> materials;
        std::vector<Node*> nodes;
 
    };

} // namespace vkc

