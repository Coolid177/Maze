#pragma once

#include "MazeObject.h"
#include <vector>

class Detector
{
private:
	std::vector<MazeObject> m_objects;
public:
	void addMazeObject(MazeObject& mazeObject);

	void clearMazeObjects();

	std::vector<MazeObject> getMazeObjects();
};
