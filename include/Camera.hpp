#ifndef Camera_HPP
#define Camera_HPP

#include <glm/glm.hpp>

class Camera
{
public:
    Camera(float fov, float aspect, float near, float far);

    void setPosition(glm::vec3 _position);
    void setDirection(float _pitch, float _yaw);
    void setProjection(float fov, float aspect, float near, float far);

    void look(float dx, float dy);
    void move(float forward, float right, float up);

    glm::mat4 getCameraMatrix();
    glm::vec3 getPosition() { return position; }
private:
    glm::vec3 position;
    float pitch, yaw;
    glm::mat4 projection;
};

#endif // Camera_HPP
