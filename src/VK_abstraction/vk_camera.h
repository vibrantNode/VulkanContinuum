#pragma once 
// libs
#define GLM_FORCE_RADIANS	
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>


namespace vkc {
	enum CameraMovement {
		FORWARD,
		BACKWARD,
		LEFT,
		RIGHT
	};

	class VkcCamera {
	public:
		VkcCamera() {};
		VkcCamera(glm::vec3 position, float yaw, float pitch);

		void setOrthographicProjection(float left, float right, float top, float bottom, float near, float far);
		void setViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up = glm::vec3{ 0.f, -1.f, 0.f });
		void setViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up = glm::vec3{ 0.f, -1.f, 0.f });

		void setViewYXZ(glm::vec3 position, glm::vec3 rotation);

		void setPerspectiveProjection(float fovy, float aspect, float near, float far);
		const glm::mat4& getProjection() const { return projectionMatrix; }
		const glm::mat4& getView() const { return viewMatrix; }

		// Get zoom (field of view)
		float GetZoom() const;
		// Get the view matrix
		glm::mat4 GetViewMatrix() const;


		glm::vec3 GetPosition() const;
		// Set movement speed
		void SetMovementSpeed(float speed);
		// new shit
		void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);
		void ProcessKeyboardInput(int direction, float deltaTime);

		//void calculateMouseOffset(float xoffset, float yoffset);
	private:
		glm::mat4 projectionMatrix{ 1.f };
		glm::mat4 viewMatrix{ 1.f };

		float m_LastX; // Last mouse x position
		float m_LastY; // Last mouse y position
		float m_MouseSensitivity = 0.1f; // Sensitivity factor for mouse input
		// Deadzone
		float m_MouseDeadZone;
		// Inertia
		glm::vec3 m_Velocity;
		// Camera attributes
		glm::vec3 m_Position;
		glm::vec3 m_Front{ 0.0f, 0.0f, -1.0f };  // Camera direction (forward vector)
		glm::vec3 m_Up{ 0.0f, 1.0f, 0.0f };     // Up vector
		glm::vec3 m_Right{ 1.0f, 0.0f, 0.0f };   // Right vector (cross product)
		glm::vec3 m_WorldUp;

		// Euler angles
		float m_Yaw = 0.0f;
		float m_Pitch = -90.0f;

		// Camera options
		float m_Zoom;
		float m_MovementSpeed;  // Added movement speed

		// Update camera vectors
		void UpdateCameraVectors();
	};
}