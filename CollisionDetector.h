#pragma once

#include "MazeObject.h"
#include <vector>

class CollisionDetector
{
private:
	std::vector<MazeObject*> m_objects;

	bool checkCameraCollisionX(MazeObject* mazeObject, glm::vec3 position, glm::vec3 size);
	bool checkCameraCollisionY(MazeObject* mazeObject, glm::vec3 position, glm::vec3 size);
	bool checkCameraCollisionZ(MazeObject* mazeObject, glm::vec3 position, glm::vec3 size);
public:
	CollisionDetector();

	bool checkCameraCollision(MazeObject* mazeObject, glm::vec3 position, glm::vec3 size);
	bool checkCameraCollisions(glm::vec3 position, glm::vec3 size);

	void addMazeObject(MazeObject* mazeObject);

	void clearMazeObjects();
};

