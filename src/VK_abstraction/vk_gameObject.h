#pragma once

#include "vk_model.h"


// libs
#include <glm/gtc/matrix_transform.hpp>

// std
#include <memory>
#include <unordered_map>


namespace vkc {
	struct TransformComponent {
		glm::vec3 translation{};
		glm::vec3 scale{ 1.f, 1.f, 1.f };
		glm::vec3 rotation{};

		// Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
		// Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)

		glm::mat4 mat4();
		glm::mat3 normalMatrix();

	};

	struct PointLightComponent {
		float lightIntensity = 1.0f;
	};

	class VkcGameObject {
	public:
		using id_t = unsigned int;
		using Map = std::unordered_map<id_t, VkcGameObject>;
		static VkcGameObject createGameObject() {
			static id_t currentId = 0;
			return VkcGameObject{ currentId++ };
		}

		static VkcGameObject makePointLight(float intensity = 10.f, float radius = 0.1f, glm::vec3 color = glm::vec3(1.f));

		VkcGameObject(const VkcGameObject&) = delete;
		VkcGameObject& operator=(const VkcGameObject&) = delete;
		VkcGameObject(VkcGameObject&&) = default;
		VkcGameObject& operator=(VkcGameObject&&) = default;

		id_t getId() { return id; }

		TransformComponent transform;
		glm::vec3 color{};

		std::shared_ptr<VkcModel> model{};
		
		std::unique_ptr<PointLightComponent> pointLight = nullptr;
	private:
		VkcGameObject(id_t objId) : id{ objId } {}

		id_t id;
	};
}