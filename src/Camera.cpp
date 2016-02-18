#include "Camera.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>

using namespace glm;

Camera::Camera(float fov, float aspect, float near, float far)
{
    setProjection(fov, aspect, near, far);
    position = vec3(0.0f, 0.0f, 0.0f);
    pitch = yaw = 0.0f;
}

void Camera::setPosition(vec3 _position)
{
    position = _position;
}

void Camera::setDirection(float _pitch, float _yaw)
{
    pitch = _pitch;
    yaw = _yaw;
}

void Camera::setProjection(float fov, float aspect, float near, float far)
{
    projection = perspective(fov, aspect, near, far);
}

void Camera::look(float dx, float dy)
{
    pitch += dy;
    yaw += dx;
    if (pitch > half_pi<float>()) pitch = half_pi<float>();
    if (pitch < -half_pi<float>()) pitch = -half_pi<float>();
    if (yaw > two_pi<float>()) yaw -= two_pi<float>();;
    if (yaw < -two_pi<float>()) yaw += two_pi<float>();
}

void Camera::move(float forward, float right, float up)
{
    position.x += forward*sin(yaw) + right*cos(yaw);
    position.y += up;
    position.z -= forward*cos(yaw) - right*sin(yaw);
}

mat4 Camera::getCameraMatrix()
{
    // TODO quaternion-based camera
    mat4 view(1.0f);
    view = glm::rotate(view, pitch, vec3(1.0f, 0.0f, 0.0f));
    view = glm::rotate(view, yaw, vec3(0.0f, 1.0f, 0.0f));
    view = glm::translate(view, -position);
    return projection * view;
}
