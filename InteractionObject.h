#pragma once
#include "MazeObject.h"

class InteractionObject : public MazeObject
{
private:
	int m_ID;
public:
	InteractionObject(glm::vec3 positionCenter, glm::vec3 size, int ID);
	int getID() const;
};

