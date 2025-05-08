#include "vk_camera.h"


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath> 
// std
#include <cassert>
#include <limits>

namespace vkc {


    VkcCamera::VkcCamera(glm::vec3 position, float yaw, float pitch)
        : m_Position(position), m_Yaw(yaw), m_Pitch(pitch),
        m_WorldUp(glm::vec3(0.0f, 1.0f, 0.0f)),
        m_Zoom(75.0f), m_MovementSpeed(20.0f) {
        UpdateCameraVectors();

    }

    // Mouse movement processing
    void VkcCamera::ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch) {
        const float sensitivity = 0.1f;  // Adjust this value to your liking
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        m_Yaw += xoffset;
        m_Pitch += yoffset;

        if (constrainPitch) {
            if (m_Pitch > 89.0f) m_Pitch = 89.0f;
            if (m_Pitch < -89.0f) m_Pitch = -89.0f;
        }

        UpdateCameraVectors();
    }
    void VkcCamera::ProcessKeyboardInput(int direction, float deltaTime) {
        float velocity = m_MovementSpeed * deltaTime;  // Use movement speed
        if (direction == FORWARD)
            m_Position += m_Front * velocity;
        if (direction == BACKWARD)
            m_Position -= m_Front * velocity;
        if (direction == LEFT)
            m_Position -= m_Right * velocity;
        if (direction == RIGHT)
            m_Position += m_Right * velocity;
    }

    void VkcCamera::setOrthographicProjection(
        float left, float right, float top, float bottom, float near, float far) {
        projectionMatrix = glm::mat4{ 1.0f };
        projectionMatrix[0][0] = 2.f / (right - left);
        projectionMatrix[1][1] = 2.f / (bottom - top);
        projectionMatrix[2][2] = 1.f / (far - near);
        projectionMatrix[3][0] = -(right + left) / (right - left);
        projectionMatrix[3][1] = -(bottom + top) / (bottom - top);
        projectionMatrix[3][2] = -near / (far - near);
    }
    // This is the method being used below as opposed to ortho ^
    void VkcCamera::setPerspectiveProjection(float fovy, float aspect, float near, float far) {
        assert(glm::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0f);
        const float tanHalfFovy = tan(fovy / 2.f);
        projectionMatrix = glm::mat4{ 0.0f };
        projectionMatrix[0][0] = 1.f / (aspect * tanHalfFovy);
        projectionMatrix[1][1] = 1.f / (tanHalfFovy);
        projectionMatrix[2][2] = far / (far - near);
        projectionMatrix[2][3] = 1.f;
        projectionMatrix[3][2] = -(far * near) / (far - near);
    }

    void VkcCamera::setViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up) {
        const glm::vec3 w{ glm::normalize(direction) };
        const glm::vec3 u{ glm::normalize(glm::cross(w, up)) };
        const glm::vec3 v{ glm::cross(w, u) };

        viewMatrix = glm::mat4{ 1.f };
        viewMatrix[0][0] = u.x;
        viewMatrix[1][0] = u.y;
        viewMatrix[2][0] = u.z;
        viewMatrix[0][1] = v.x;
        viewMatrix[1][1] = v.y;
        viewMatrix[2][1] = v.z;
        viewMatrix[0][2] = w.x;
        viewMatrix[1][2] = w.y;
        viewMatrix[2][2] = w.z;
        viewMatrix[3][0] = -glm::dot(u, position);
        viewMatrix[3][1] = -glm::dot(v, position);
        viewMatrix[3][2] = -glm::dot(w, position);
    }

    void VkcCamera::setViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up) {
        setViewDirection(position, target - position, up);
    }

    void VkcCamera::setViewYXZ(glm::vec3 position, glm::vec3 rotation) {
        const float c3 = glm::cos(rotation.z);
        const float s3 = glm::sin(rotation.z);
        const float c2 = glm::cos(rotation.x);
        const float s2 = glm::sin(rotation.x);
        const float c1 = glm::cos(rotation.y);
        const float s1 = glm::sin(rotation.y);
        const glm::vec3 u{ (c1 * c3 + s1 * s2 * s3), (c2 * s3), (c1 * s2 * s3 - c3 * s1) };
        const glm::vec3 v{ (c3 * s1 * s2 - c1 * s3), (c2 * c3), (c1 * c3 * s2 + s1 * s3) };
        const glm::vec3 w{ (c2 * s1), (-s2), (c1 * c2) };
        viewMatrix = glm::mat4{ 1.f };
        viewMatrix[0][0] = u.x;
        viewMatrix[1][0] = u.y;
        viewMatrix[2][0] = u.z;
        viewMatrix[0][1] = v.x;
        viewMatrix[1][1] = v.y;
        viewMatrix[2][1] = v.z;
        viewMatrix[0][2] = w.x;
        viewMatrix[1][2] = w.y;
        viewMatrix[2][2] = w.z;
        viewMatrix[3][0] = -glm::dot(u, position);
        viewMatrix[3][1] = -glm::dot(v, position);
        viewMatrix[3][2] = -glm::dot(w, position);
    }

    void VkcCamera::UpdateCameraVectors() {
        // Calculate the new front vector based on yaw and pitch
        glm::vec3 front;
        front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
        front.y = sin(glm::radians(m_Pitch));
        front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
        this->m_Front = glm::normalize(front);

        // Re-calculate the right and up vector
        this->m_Right = glm::normalize(glm::cross(this->m_Front, this->m_Up));
        this->m_Up = glm::normalize(glm::cross(this->m_Right, this->m_Front));
    }


    // Get the camera's zoom (field of view)
    float VkcCamera::GetZoom() const {
        return m_Zoom;
    }

    // Set the movement speed of the camera
    void VkcCamera::SetMovementSpeed(float speed) {
        m_MovementSpeed = speed;
    }
    // Get the view matrix
    glm::mat4 VkcCamera::GetViewMatrix() const {
        return glm::lookAt(m_Position, m_Position + m_Front, m_Up);
    }
    // Get the camera position
    glm::vec3 VkcCamera::GetPosition() const {
        return m_Position;
    }

}  // namespace vkc