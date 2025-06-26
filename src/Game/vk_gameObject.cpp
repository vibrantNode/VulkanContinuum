#include "vk_gameObject.h"

namespace vkc 
{
	glm::mat4 TransformComponent::mat4() const
	{
		// 1) Translation
		glm::mat4 T = glm::translate(glm::mat4(1.0f), translation);

		// 2) Rotation: yaw (Y), pitch (X), roll (Z)
		glm::mat4 R = glm::rotate(glm::mat4(1.0f), rotation.y, glm::vec3(0, 1, 0));
		R = glm::rotate(R, rotation.x, glm::vec3(1, 0, 0));
		R = glm::rotate(R, rotation.z, glm::vec3(0, 0, 1));

		// 3) Scale
		glm::mat4 S = glm::scale(glm::mat4(1.0f), scale);

		// Compose: T * R * S
		return T * R * S;
	}
	glm::mat3 TransformComponent::normalMatrix() const
	{
		// Build RS (ignore translation)
		glm::mat4 R = glm::rotate(glm::mat4(1.0f), rotation.y, glm::vec3(0, 1, 0));
		R = glm::rotate(R, rotation.x, glm::vec3(1, 0, 0));
		R = glm::rotate(R, rotation.z, glm::vec3(0, 0, 1));

		glm::mat4 RS = R * glm::scale(glm::mat4(1.0f), scale);

		// Normal matrix = inverse-transpose of the 3x3 upper-left
		return glm::transpose(glm::inverse(glm::mat3(RS)));
	}
	VkcGameObject VkcGameObject::makePointLight(float intensity, float radius, glm::vec3 color)
	{
		VkcGameObject gameObj = VkcGameObject::createGameObject();
		gameObj.color = color;
		gameObj.transform.scale.x = radius;
		gameObj.pointLight = std::make_unique<PointLightComponent>();
		gameObj.pointLight->lightIntensity = intensity;
		return gameObj;
	}
}// namespace vkc