#ifndef MAZEHANDLER_H
#define MAZEHANDLER_H

#include <string>
#include <vector>
#include <fstream>
#include <glm/glm/vec3.hpp>

using namespace std;

class MazeHandler {
private:
	string m_mazeFile;
	vector<vector<char>> *m_maze;

public:
	MazeHandler(string file);
	vector<glm::vec3> getBuildingPositionos();
	vector<glm::vec3> getLightPositions();
	float getMazeWidth() const;
	float getMazeHeight() const;
	vector<glm::vec3> getTrashPositions();
	glm::vec3 spawnLocation();
};

#endif // !MAZEHANDLER_H
