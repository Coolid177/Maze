#include "InteractionDetector.h"

InteractionDetector::InteractionDetector(float reach) : m_reach{ reach } {}

bool InteractionDetector::checkInteractions(glm::vec3 minReach, glm::vec3 front) {
	bool interaction = false;
	for (const InteractionObject& obj : m_objects) {
		if (canReach(obj, minReach, minReach + m_reach * front)) {
			m_interactableID = obj.getID();
			return true;
		}
	}
	return false;
}

bool InteractionDetector::canReach(const MazeObject& mazeObject, glm::vec3 minReach, glm::vec3 maxReach) {
	glm::vec3 center = mazeObject.getCenterPosition();
	glm::vec3 size = mazeObject.getSize();
	bool XYIntersection = checkPlane(std::pair<float, float>(center.x, center.y), std::pair<float, float>(size.x, size.y), std::pair<float, float>(minReach.x, minReach.y), std::pair<float, float>(maxReach.x, maxReach.y));
	bool XZIntersection = checkPlane(std::pair<float, float>(center.x, center.z), std::pair<float, float>(size.x, size.z), std::pair<float, float>(minReach.x, minReach.z), std::pair<float, float>(maxReach.x, maxReach.z));
	bool ZYIntersection = checkPlane(std::pair<float, float>(center.z, center.y), std::pair<float, float>(size.z, size.y), std::pair<float, float>(minReach.z, minReach.y), std::pair<float, float>(maxReach.z, maxReach.y));
	return XYIntersection && XZIntersection && ZYIntersection;
}

bool InteractionDetector::checkPlane(std::pair<float, float> planeCenter, std::pair<float, float> planeSize, std::pair<float, float> planeMinReach, std::pair<float, float> planeMaxReach) {
	std::pair<float, float> maxCords = std::pair<float, float>(planeCenter.first + (0.5 * planeSize.first), planeCenter.second + (0.5 * planeSize.second));
	std::pair<float, float> minCords = std::pair<float, float>(planeCenter.first - (0.5 * planeSize.first), planeCenter.second - (0.5 * planeSize.second));
	PositionCode minCode = generatePositionCode(maxCords, minCords, planeMinReach.first, planeMinReach.second);
	PositionCode maxCode = generatePositionCode(maxCords, minCords, planeMaxReach.first, planeMaxReach.second);
	if ((minCode.all | maxCode.all) == 0) {
		return true;
	}
	else if ((minCode.all & maxCode.all) != 0) {
		return false;
	}
	return true;
}

PositionCode InteractionDetector::generatePositionCode(std::pair<float, float> max, std::pair<float, float> min, float d1, float d2)
{
	PositionCode code;

	if (d2 > max.second) { //above
		code.topCode = 1;
		code.all += 8;
	}
	else if (d2 < min.second) { // below
		code.bottomCode = 1;
		code.all += 4;
	}

	if (d1 > max.first) { // left
		code.rightCode = 1;
		code.all += 2;
	}
	else if (d1 < min.first) { // right
		code.leftCode = 1;
		code.all += 1;
	}

	return code;
}

int InteractionDetector::getInteractedID() {
	return m_interactableID;
}

void InteractionDetector::addInteractionObject(InteractionObject& mazeObject) {
	m_objects.push_back(mazeObject);
}

void InteractionDetector::clearMazeObjects() {
	m_objects.clear();
}