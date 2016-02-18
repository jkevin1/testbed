#ifndef Models_HPP
#define Models_HPP

#include <glm/glm.hpp>

namespace Models
{
void initialize();

void update(const glm::mat4& vpMatrix, const glm::vec3& eyePos);

void draw();

void release();
}

#endif // Models_HPP
