// vk_model.cpp

// Project headers
#include "vk_obj_model.h"
#include "Utils/vkc_utils.h"


// External
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>




#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// STD
#include <cassert>
#include <cstring>
#include <stdexcept>
#include <unordered_map>
#include <unordered_set>


namespace std
{
    template <>
    struct hash<vkc::VkcOBJmodel::Vertex> {
        size_t operator()(vkc::VkcOBJmodel::Vertex const& vertex) const
        {
            size_t seed = 0;
            vkc::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
            return seed;
        }
    };
}

namespace vkc
{
    VkcOBJmodel::VkcOBJmodel(VkcDevice& device, const Builder& builder)
        : vkcDevice{ device }, isSkyboxModel{ builder.isSkybox } {

        if (builder.isSkybox) {
            createVertexBuffers(builder.skyboxVertices); 
        }
        else {
            createVertexBuffers(builder.vertices);
            createIndexBuffers(builder.indices);
        }
      
    
    }

    VkcOBJmodel::~VkcOBJmodel() {}

    std::shared_ptr<VkcOBJmodel> VkcOBJmodel::createModelFromFile(VkcDevice& device, const std::string& filepath, bool isSkybox)
    {
        Builder builder{};
        builder.loadModel(filepath, isSkybox);
        return std::make_shared<VkcOBJmodel>(device, builder);
    }

    void VkcOBJmodel::createVertexBuffers(const std::vector<Vertex>& vertices)
    {
        vertexCount = static_cast<uint32_t>(vertices.size());
        assert(vertexCount >= 3 && "Vertex count must be at least 3");
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
        uint32_t vertexSize = sizeof(vertices[0]);

        VkcBuffer stagingBuffer{
            vkcDevice,
            vertexSize,
            vertexCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        };

        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void*)vertices.data());

        vertexBuffer = std::make_unique<VkcBuffer>(
            vkcDevice,
            vertexSize,
            vertexCount,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );

        vkcDevice.copyBuffer(stagingBuffer.getBuffer(), vertexBuffer->getBuffer(), bufferSize);
    }
    void VkcOBJmodel::createVertexBuffers(const std::vector<SkyboxVertex>& vertices) {
        vertexCount = static_cast<uint32_t>(vertices.size());
        assert(vertexCount >= 3 && "Skybox vertex count must be at least 3");
        VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
        uint32_t vertexSize = sizeof(vertices[0]);

        VkcBuffer stagingBuffer{
            vkcDevice,
            vertexSize,
            vertexCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        };

        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void*)vertices.data()); 
     
        vertexBuffer = std::make_unique<VkcBuffer>(
            vkcDevice,
            vertexSize,
            vertexCount,
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );

        vkcDevice.copyBuffer(stagingBuffer.getBuffer(), vertexBuffer->getBuffer(), bufferSize);
    }

    void VkcOBJmodel::createIndexBuffers(const std::vector<uint32_t>& indices)
    {
        indexCount = static_cast<uint32_t>(indices.size());
        hasIndexBuffer = indexCount > 0;
        if (!hasIndexBuffer) return;

        VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;
        uint32_t indexSize = sizeof(indices[0]);

        VkcBuffer stagingBuffer{
            vkcDevice,
            indexSize,
            indexCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        };

        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void*)indices.data());

        indexBuffer = std::make_unique<VkcBuffer>(
            vkcDevice,
            indexSize,
            indexCount,
            VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

        vkcDevice.copyBuffer(stagingBuffer.getBuffer(), indexBuffer->getBuffer(), bufferSize);
    }

    void VkcOBJmodel::draw(VkCommandBuffer commandBuffer)
    {
        if (hasIndexBuffer) {
            vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
        }
        else {
            vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
        }
    }


    void VkcOBJmodel::bind(VkCommandBuffer commandBuffer)
    {
        VkBuffer buffers[] = { vertexBuffer->getBuffer() };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

        if (hasIndexBuffer) {
            vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
        }
    }

    std::vector<VkVertexInputBindingDescription> VkcOBJmodel::Vertex::getBindingDescriptions()
    {
        return {
            VkVertexInputBindingDescription{
                0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX
            }
        };
    }

    std::vector<VkVertexInputAttributeDescription> VkcOBJmodel::Vertex::getAttributeDescriptions()
    {
        return {
            { 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position) },
            { 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color) },
            { 2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal) },
            { 3, 0, VK_FORMAT_R32G32_SFLOAT,    offsetof(Vertex, uv) }
        };
    }

    std::vector<VkVertexInputBindingDescription> VkcOBJmodel::SkyboxVertex::getBindingDescriptions() {
        return { {
                /*binding=*/0,
                /*stride=*/sizeof(SkyboxVertex),
                /*inputRate=*/VK_VERTEX_INPUT_RATE_VERTEX
              } };
    }

    std::vector<VkVertexInputAttributeDescription> VkcOBJmodel::SkyboxVertex::getAttributeDescriptions() {
        return { {
                /*location=*/0,
                /*binding=*/0,
                /*format=*/VK_FORMAT_R32G32B32_SFLOAT,
                /*offset=*/offsetof(SkyboxVertex, position)
              } };
    }


    void VkcOBJmodel::Builder::loadModel(const std::string& filepath, bool isSkybox) {
        this->isSkybox = isSkybox;
        vertices.clear();
        skyboxVertices.clear();
        indices.clear();

        // 1) Check extension
        auto lastDot = filepath.find_last_of('.');
        if (lastDot == std::string::npos) {
            throw std::runtime_error("Model file has no extension: " + filepath);
        }
        std::string ext = filepath.substr(lastDot + 1);
        for (auto& c : ext) c = static_cast<char>(::tolower(c));

        if (ext == "obj") {
            // tinyobj loader ---
            tinyobj::attrib_t attrib;
            std::vector<tinyobj::shape_t>    shapes;
            std::vector<tinyobj::material_t> materials;
            std::string warn, err;
            if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath.c_str())) {
                throw std::runtime_error("Failed to load OBJ: " + warn + err);
            }


            if (isSkybox) {
                for (auto const& shape : shapes) {
                    for (auto const& index : shape.mesh.indices) {
                        SkyboxVertex vertex{};
                        vertex.position = {
                            attrib.vertices[3 * index.vertex_index + 0],
                            attrib.vertices[3 * index.vertex_index + 1],
                            attrib.vertices[3 * index.vertex_index + 2],
                        };
                        skyboxVertices.push_back(vertex);
                        indices.push_back(static_cast<uint32_t>(skyboxVertices.size() - 1));
                    }
                }
            }
            else {
                std::unordered_map<Vertex, uint32_t> uniqueVertices{};
                for (auto const& shape : shapes) {
                    for (auto const& index : shape.mesh.indices) {
                        Vertex vertex{};
                        // copy position, color, normal, uv exactly as before
                        if (index.vertex_index >= 0) {
                            vertex.position = {
                                attrib.vertices[3 * index.vertex_index + 0],
                                attrib.vertices[3 * index.vertex_index + 1],
                                attrib.vertices[3 * index.vertex_index + 2],
                            };
                        }
                        if (!attrib.colors.empty()) {
                            vertex.color = {
                                attrib.colors[3 * index.vertex_index + 0],
                                attrib.colors[3 * index.vertex_index + 1],
                                attrib.colors[3 * index.vertex_index + 2],
                            };
                        }
                        if (index.normal_index >= 0) {
                            vertex.normal = {
                                attrib.normals[3 * index.normal_index + 0],
                                attrib.normals[3 * index.normal_index + 1],
                                attrib.normals[3 * index.normal_index + 2],
                            };
                        }
                        if (index.texcoord_index >= 0) {
                            vertex.uv = {
                                attrib.texcoords[2 * index.texcoord_index + 0],
                                1.0f - attrib.texcoords[2 * index.texcoord_index + 1],
                            };
                        }

                        if (uniqueVertices.count(vertex) == 0) {
                            uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
                            vertices.push_back(vertex);
                        }
                        indices.push_back(uniqueVertices[vertex]);
                    }
                }
            }
            return;
        }
        else {
            throw std::runtime_error("Unsupported model format: " + ext);
        }
    }



}// namespace vkc