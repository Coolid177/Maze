#include "Detector.h"

void Detector::addMazeObject(MazeObject& mazeObject) {
	m_objects.push_back(mazeObject);
}

void Detector::clearMazeObjects() {
	m_objects.clear();
}

std::vector<MazeObject> Detector::getMazeObjects() {
	return m_objects;
}