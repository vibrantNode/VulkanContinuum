#pragma once 
// libs
#define GLM_FORCE_RADIANS	
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>


namespace vkc 
{
	enum CameraMovement
	{
		FORWARD,
		BACKWARD,
		LEFT,
		RIGHT
	};

	struct CameraInfo {
		glm::vec3 position;
		glm::vec3 target;
		float yawOffset;
		float pitchOffset;
		float fov;
		float movementSpeed;
	};

	class VkcCamera
	{
	public:
		VkcCamera() {};
		VkcCamera(glm::vec3 position, float yaw, float pitch, float zoom);

		void setOrthographicProjection(float left, float right, float top, float bottom, float near, float far);
		void setViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up = glm::vec3{ 0.f, -1.f, 0.f });
		void setViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up = glm::vec3{ 0.f, -1.f, 0.f });
		void SetTarget(const glm::vec3& target);

		void setViewYXZ(glm::vec3 position, glm::vec3 rotation);

		void setPerspectiveProjection(float aspect, float near, float far);
		const glm::mat4& getProjection() const { return projectionMatrix; }
		const glm::mat4& getView() const { return viewMatrix; }
		const glm::mat4& getInverseView() const{ return inverseViewMatrix; }
		// Get zoom (field of view)
		float GetZoom() const;
		// Get the view matrix
		glm::mat4 GetViewMatrix() const;
		glm::vec3 getPosition() const { return m_Position; }
		glm::vec3 getTarget() const { return m_Target; }
		float getFov() const { return m_Zoom; }



		glm::vec3 GetPosition() const;
		
	private:
		glm::mat4 projectionMatrix{ 1.f };
		glm::mat4 viewMatrix{ 1.f };
		glm::mat4 inverseViewMatrix{ 1.f };


		// Camera attributes
		glm::vec3 m_Position;
		glm::vec3 m_Front{ 0.0f, 0.0f, -1.0f };  // Camera direction (forward vector)
		glm::vec3 m_Up{ 0.0f, 1.0f, 0.0f };     // Up vector
		glm::vec3 m_Right{ 1.0f, 0.0f, 0.0f };   // Right vector (cross product)
		glm::vec3 m_WorldUp;


		// Camera options
		float m_Zoom;

		glm::vec3 m_Target;
		// Update camera vectors
		void UpdateCameraVectors();

	};
}// namespace vkc