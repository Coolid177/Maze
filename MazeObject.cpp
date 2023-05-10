#include "MazeObject.h"

MazeObject::MazeObject(glm::vec3 position, glm::vec3 size) : m_centerPosition{ position }, m_size{ size }
{
}

glm::vec3 MazeObject::getSize() const
{
	return m_size;
}

glm::vec3 MazeObject::getCenterPosition() const
{
	return m_centerPosition;
}
