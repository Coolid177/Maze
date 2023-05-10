#pragma once

#include <glm/glm/glm.hpp>

class MazeObject
{
private:
	glm::vec3 m_centerPosition;
	glm::vec3 m_size;

public:
	MazeObject(glm::vec3 positionCenter, glm::vec3 size);
	glm::vec3 getSize() const;
	glm::vec3 getCenterPosition() const;
};
