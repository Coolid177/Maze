#pragma once

#include "InteractionObject.h"
#include <vector>

struct PositionCode {
	int all = 0;
	int leftCode = 0;
	int rightCode = 0;
	int bottomCode = 0;
	int topCode = 0;
};

class InteractionDetector
{
private:
	float m_resolution;
	float m_reach;
	int m_interactableID = -1;

	std::vector<InteractionObject> m_objects;

	bool canReach(const MazeObject& mazeObject, glm::vec3 front, glm::vec3 minReach);
	bool checkPlane(std::pair<float, float> planeCenter, std::pair<float, float> planeSize, std::pair<float, float> planeMinReach, std::pair<float, float> planeMaxReach);
	PositionCode generatePositionCode(std::pair<float, float> max, std::pair<float, float> min, float d1, float d2);
public:
	InteractionDetector(float reach);
	bool checkInteractions(glm::vec3 minReach, glm::vec3 front);

	int getInteractedID();

	void addInteractionObject(InteractionObject& mazeObject);

	void clearMazeObjects();
};

