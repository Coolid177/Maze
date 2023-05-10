#include "InteractionObject.h"

InteractionObject::InteractionObject(glm::vec3 positionCenter, glm::vec3 size, int ID) : m_ID{ ID }, MazeObject(positionCenter, size) {

}

int InteractionObject::getID() const {
	return m_ID;
}