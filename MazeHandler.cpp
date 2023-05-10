#include "MazeHandler.h"

#include <fstream>
#include <iostream>
#include <time.h>

MazeHandler::MazeHandler(string file) : m_mazeFile{ file }, m_maze{new vector<vector<char>>()} {
	ifstream maze(file); 
	if (maze.is_open()) {
		vector<char> row = vector<char>();
		while (maze) { //not end of file
			char type = maze.get();
			if (type == '#') { //if wall, add to matrix
				row.push_back('#');
			}
			else if (type == '\n') {//end of row reached
				m_maze->push_back(row); //push row to maze
				row.clear(); //clear the row
			} else { //no wall, add blank space
				row.push_back(' '); 
			}
		}
		m_maze->push_back(row);
	}
}

vector<glm::vec3> MazeHandler::getBuildingPositionos() {
	vector<glm::vec3> positions = vector<glm::vec3>();
	float startX = 0;
	const float Y = 11.2f; //Y position is always the same. Objects should not float
	float startZ = 0; //we start far and go closer
	for (int row = 0; row < m_maze->size(); row++) {
		for (int column = 0; column < m_maze->at(row).size(); column++) {
			if (m_maze->at(row).at(column) == '#') {
				positions.push_back(glm::vec3(startX, Y, -startZ));
			}
			startX += 16.25f;
		}
		startX = 0;
		startZ += 14.5f;
	}
	return positions;
}

vector<glm::vec3> MazeHandler::getLightPositions() {
	//seed for rand
	time_t t;
	srand((unsigned)time(&t));

	//positions
	vector<glm::vec3> positions = vector<glm::vec3>();
	float startX = 10;
	const float Y = 25.0f; //Y position is always the same. Objects should not float
	float startZ = 10; //we start far and go closer
	for (int row = 0; row < 30; row++) {
		positions.push_back(glm::vec3(rand() % m_maze->at(0).size() * 16.25, Y + (rand() % 5), -(rand() % m_maze->size() * 14.25)));
	}
	return positions;
}

vector<glm::vec3> MazeHandler::getTrashPositions() {
	time_t t;
	srand((unsigned)time(&t));
	vector<glm::vec3> positions = vector<glm::vec3>();
	for (int i = 0; i < m_maze->size(); i++) {
		for (int j = 0; j < m_maze->at(i).size(); j++) {
			if (m_maze->at(i).at(j) == ' ') {
				int factor = -1;
				if (rand() % 2 == 0) {
					factor = 1;
				}
				if (rand() % 3 == 0) {
					positions.push_back(glm::vec3(i * 16.25 + ((rand() % 8) * factor), -0.78, -(j * 14.5f + ((rand() % 6) * factor))));
				}
			}
		}
	}
	return positions;
}

float MazeHandler::getMazeWidth() const {
	return m_maze->size() * 2;
}
float MazeHandler::getMazeHeight() const {
	return m_maze->at(0).size() * 2;
}

glm::vec3 MazeHandler::spawnLocation() {
	for (int i = 3; i < m_maze->size(); i++) {
		for (int j = 3; j < m_maze->at(i).size(); j++) {
			if (m_maze->at(i).at(j) == ' ') {
				return glm::vec3(j*16.25f, 7.0f, -i*14.5);
			}
		}
	}
}