// vk_model.cpp

// Project headers
#include "vk_model.h"
#include "Utils/vkc_utils.h"


// External
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#define STB_IMAGE_IMPLEMENTATION       // <-- add this
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NOEXCEPTION
#include <tiny_gltf.h>

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


    void VkcGLTFmodel::loadImages(tinygltf::Model& input)
    {
        // Images can be stored inside the glTF (which is the case for the sample model), so instead of directly
        images.clear();
        images.reserve(input.images.size());

        for (size_t i = 0; i < input.images.size(); i++) {
            tinygltf::Image& glTFImage = input.images[i];

            unsigned char* buffer = nullptr;
            VkDeviceSize bufferSize = 0;
            bool deleteBuffer = false;

            if (glTFImage.component == 3) {
                bufferSize = glTFImage.width * glTFImage.height * 4;
                buffer = new unsigned char[bufferSize];
                unsigned char* rgba = buffer;
                unsigned char* rgb = &glTFImage.image[0];
                for (size_t i = 0; i < glTFImage.width * glTFImage.height; ++i) {
                    memcpy(rgba, rgb, sizeof(unsigned char) * 3);
                    rgba += 4;
                    rgb += 3;
                }
                deleteBuffer = true;
            }
            else {
                buffer = &glTFImage.image[0];
                bufferSize = glTFImage.image.size();
            }

            // Construct and upload texture
            Image image;
            image.texture = VkcTexture(m_pDevice); // ← construct explicitly
            image.texture.fromBuffer(buffer, bufferSize, VK_FORMAT_R8G8B8A8_UNORM, glTFImage.width, glTFImage.height, m_pDevice, m_vkCopyQueue);

            images.emplace_back(std::move(image));

            if (deleteBuffer) {
                delete[] buffer;
            }
        }
    }

    void VkcGLTFmodel::loadTextures(tinygltf::Model& input)
    {
        textures.resize(input.textures.size());
        for (size_t i = 0; i < input.textures.size(); i++) {
            textures[i].imageIndex = input.textures[i].source;
        }
    }
  void VkcGLTFmodel::loadMaterials(tinygltf::Model& input)
  {
      materials.resize(input.materials.size());
      for (size_t i = 0; i < input.materials.size(); i++) {
          // We only read the most basic properties required for our sample
          tinygltf::Material glTFMaterial = input.materials[i];
          // Get the base color factor
          if (glTFMaterial.values.find("baseColorFactor") != glTFMaterial.values.end()) {
              materials[i].baseColorFactor = glm::make_vec4(glTFMaterial.values["baseColorFactor"].ColorFactor().data());
          }
          // Get base color texture index
          if (glTFMaterial.values.find("baseColorTexture") != glTFMaterial.values.end()) {
              materials[i].baseColorTextureIndex = glTFMaterial.values["baseColorTexture"].TextureIndex();
          }
      }
  }

  void VkcGLTFmodel::loadNode(const tinygltf::Node& inputNode, const tinygltf::Model& input, VkcGLTFmodel::Node* parent, std::vector<uint32_t>& indexBuffer, std::vector<VkcGLTFmodel::Vertex>& vertexBuffer)
  {
      VkcGLTFmodel::Node* node = new VkcGLTFmodel::Node{};
      node->matrix = glm::mat4(1.0f);
      node->parent = parent;

      // Get the local node matrix
      // It's either made up from translation, rotation, scale or a 4x4 matrix
      if (inputNode.translation.size() == 3) {
          node->matrix = glm::translate(node->matrix, glm::vec3(glm::make_vec3(inputNode.translation.data())));
      }
      if (inputNode.rotation.size() == 4) {
          glm::quat q = glm::make_quat(inputNode.rotation.data());
          node->matrix *= glm::mat4(q);
      }
      if (inputNode.scale.size() == 3) {
          node->matrix = glm::scale(node->matrix, glm::vec3(glm::make_vec3(inputNode.scale.data())));
      }
      if (inputNode.matrix.size() == 16) {
          node->matrix = glm::make_mat4x4(inputNode.matrix.data());
      };

      // Load node's children
      if (inputNode.children.size() > 0) {
          for (size_t i = 0; i < inputNode.children.size(); i++) {
              loadNode(input.nodes[inputNode.children[i]], input, node, indexBuffer, vertexBuffer);
          }
      }

      // If the node contains mesh data, we load vertices and indices from the buffers
      // In glTF this is done via accessors and buffer views
      if (inputNode.mesh > -1) {
          const tinygltf::Mesh mesh = input.meshes[inputNode.mesh];
          // Iterate through all primitives of this node's mesh
          for (size_t i = 0; i < mesh.primitives.size(); i++) {
              const tinygltf::Primitive& glTFPrimitive = mesh.primitives[i];
              uint32_t firstIndex = static_cast<uint32_t>(indexBuffer.size());
              uint32_t vertexStart = static_cast<uint32_t>(vertexBuffer.size());
              uint32_t indexCount = 0;
              // Vertices
              {
                  const float* positionBuffer = nullptr;
                  const float* normalsBuffer = nullptr;
                  const float* texCoordsBuffer = nullptr;
                  size_t vertexCount = 0;

                  // Get buffer data for vertex positions
                  if (glTFPrimitive.attributes.find("POSITION") != glTFPrimitive.attributes.end()) {
                      const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.attributes.find("POSITION")->second];
                      const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
                      positionBuffer = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                      vertexCount = accessor.count;
                  }
                  // Get buffer data for vertex normals
                  if (glTFPrimitive.attributes.find("NORMAL") != glTFPrimitive.attributes.end()) {
                      const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.attributes.find("NORMAL")->second];
                      const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
                      normalsBuffer = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                  }
                  // Get buffer data for vertex texture coordinates
                  // glTF supports multiple sets, we only load the first one
                  if (glTFPrimitive.attributes.find("TEXCOORD_0") != glTFPrimitive.attributes.end()) {
                      const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.attributes.find("TEXCOORD_0")->second];
                      const tinygltf::BufferView& view = input.bufferViews[accessor.bufferView];
                      texCoordsBuffer = reinterpret_cast<const float*>(&(input.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
                  }

                  // Append data to model's vertex buffer
                  for (size_t v = 0; v < vertexCount; v++) {
                      Vertex vert{};
                      vert.pos = glm::vec4(glm::make_vec3(&positionBuffer[v * 3]), 1.0f);
                      vert.normal = glm::normalize(glm::vec3(normalsBuffer ? glm::make_vec3(&normalsBuffer[v * 3]) : glm::vec3(0.0f)));
                      vert.uv = texCoordsBuffer
                          ? glm::vec3(glm::make_vec2(&texCoordsBuffer[v * 2]), 0.0f)
                          : glm::vec3(0.0f);

                      vert.color = glm::vec3(1.0f);
                      vertexBuffer.push_back(vert);
                  }
              }
              // Indices
              {
                  const tinygltf::Accessor& accessor = input.accessors[glTFPrimitive.indices];
                  const tinygltf::BufferView& bufferView = input.bufferViews[accessor.bufferView];
                  const tinygltf::Buffer& buffer = input.buffers[bufferView.buffer];

                  indexCount += static_cast<uint32_t>(accessor.count);

                  // glTF supports different component types of indices
                  switch (accessor.componentType) {
                  case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT: {
                      const uint32_t* buf = reinterpret_cast<const uint32_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
                      for (size_t index = 0; index < accessor.count; index++) {
                          indexBuffer.push_back(buf[index] + vertexStart);
                      }
                      break;
                  }
                  case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT: {
                      const uint16_t* buf = reinterpret_cast<const uint16_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
                      for (size_t index = 0; index < accessor.count; index++) {
                          indexBuffer.push_back(buf[index] + vertexStart);
                      }
                      break;
                  }
                  case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE: {
                      const uint8_t* buf = reinterpret_cast<const uint8_t*>(&buffer.data[accessor.byteOffset + bufferView.byteOffset]);
                      for (size_t index = 0; index < accessor.count; index++) {
                          indexBuffer.push_back(buf[index] + vertexStart);
                      }
                      break;
                  }
                  default:
                      std::cerr << "Index component type " << accessor.componentType << " not supported!" << std::endl;
                      return;
                  }
              }
              Primitive primitive{};
              primitive.firstIndex = firstIndex;
              primitive.indexCount = indexCount;
              primitive.materialIndex = glTFPrimitive.material;
              node->mesh.primitives.push_back(primitive);
          }
      }

      if (parent) {
          parent->children.push_back(node);
      }
      else {
          nodes.push_back(node);
      }
  }

  void VkcGLTFmodel::drawNode(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout, VkcGLTFmodel::Node* node)
  {
      if (node->mesh.primitives.size() > 0) {
          // Pass the node's matrix via push constants
          // Traverse the node hierarchy to the top-most parent to get the final matrix of the current node
          glm::mat4 nodeMatrix = node->matrix;
          VkcGLTFmodel::Node* currentParent = node->parent;
          while (currentParent) {
              nodeMatrix = currentParent->matrix * nodeMatrix;
              currentParent = currentParent->parent;
          }
          // Pass the final matrix to the vertex shader using push constants
          vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &nodeMatrix);
          for (VkcGLTFmodel::Primitive& primitive : node->mesh.primitives) {
              if (primitive.indexCount > 0) {
                  // Get the texture index for this primitive
                  VkcGLTFmodel::Texture texture = textures[materials[primitive.materialIndex].baseColorTextureIndex];
                  // Bind the descriptor for the current primitive's texture
                  vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 1, 1, &images[texture.imageIndex].descriptorSet, 0, nullptr);
                  vkCmdDrawIndexed(commandBuffer, primitive.indexCount, 1, primitive.firstIndex, 0, 0);
              }
          }
      }
      for (auto& child : node->children) {
          drawNode(commandBuffer, pipelineLayout, child);
      }
  }

  void VkcGLTFmodel::draw(VkCommandBuffer commandBuffer, VkPipelineLayout pipelineLayout)
  {
      // All vertices and indices are stored in single buffers, so we only need to bind once
      VkDeviceSize offsets[1] = { 0 };
      vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertices.buffer, offsets);
      vkCmdBindIndexBuffer(commandBuffer, indices.buffer, 0, VK_INDEX_TYPE_UINT32);
      // Render all nodes at top-level
      for (auto& node : nodes) {
          drawNode(commandBuffer, pipelineLayout, node);
      }
  }



}// namespace vkc