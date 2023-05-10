#include "CollisionDetector.h"
#include <glm/glm/glm.hpp>

CollisionDetector::CollisionDetector()
{
}

bool CollisionDetector::checkCameraCollisionX(MazeObject* mazeObject, glm::vec3 position, glm::vec3 size)
{
	glm::vec3 objectPosition = mazeObject->getCenterPosition();
	glm::vec3 objectSize = mazeObject->getSize();
	bool xCollision = ((objectPosition.x + (objectSize.x / 2)) >= (position.x - (size.x / 2)) && (objectPosition.x - (objectSize.x / 2)) <= (position.x - (size.x / 2))) || ((objectPosition.x - (objectSize.x / 2)) <= (position.x + (size.x / 2)) && (objectPosition.x + (objectSize.x / 2)) >= (position.x + (size.x / 2)));
	return xCollision;
}

bool CollisionDetector::checkCameraCollisionY(MazeObject* mazeObject, glm::vec3 position, glm::vec3 size)
{
	glm::vec3 objectPosition = mazeObject->getCenterPosition();
	glm::vec3 objectSize = mazeObject->getSize();
	bool yCollision = ((objectPosition.y + (objectSize.y / 2)) >= (position.y - size.y) && (objectPosition.y - (objectSize.y / 2)) <= (position.y - size.y)) || ((objectPosition.y - (objectSize.y / 2)) <= position.y && (objectPosition.y + (objectSize.y / 2)) >= position.y);
	return yCollision;
}

bool CollisionDetector::checkCameraCollisionZ(MazeObject* mazeObject, glm::vec3 position, glm::vec3 size)
{
	glm::vec3 objectPosition = mazeObject->getCenterPosition();
	glm::vec3 objectSize = mazeObject->getSize();
	bool zCollision = ((objectPosition.z + (objectSize.z / 2)) >= (position.z - (size.z / 2)) && (objectPosition.z - (objectSize.z / 2)) <= (position.z - (size.z / 2))) || ((objectPosition.z - (objectSize.z / 2)) <= (position.z + (size.z / 2)) && (objectPosition.z + (objectSize.z / 2)) >= (position.z + (size.z / 2)));
	return zCollision;
}

bool CollisionDetector::checkCameraCollisions(glm::vec3 position, glm::vec3 size)
{
	bool collision = false;
	for (MazeObject* obj : m_objects) {
		collision = checkCameraCollision(obj, position, size);
		if (collision) {
			return collision;
		}
	}
	return collision;
}

bool CollisionDetector::checkCameraCollision(MazeObject* mazeObject, glm::vec3 position, glm::vec3 size) {
	return checkCameraCollisionX(mazeObject, position, size) && checkCameraCollisionY(mazeObject, position, size) && checkCameraCollisionZ(mazeObject, position, size);
}

void CollisionDetector::addMazeObject(MazeObject* mazeObject) {
	m_objects.push_back(mazeObject);
}

void CollisionDetector::clearMazeObjects() {
	m_objects.clear();
}