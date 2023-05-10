#include <vector>
#include <iostream>
#include <random>
#include <fstream>

// prim's agorithm for creating mazes
// algorithm idea from
// https://stackoverflow.com/questions/29739751/implementing-a-randomly-generated-maze-using-prims-algorithm
// https://en.wikipedia.org/wiki/Maze_generation_algorithm

/** DESCRIPTION OF THE ALGORITHM
* 1) A Grid consists of a 2 dimensional array of cells.
* 2) A Cell has 2 states: Blocked or Passage.
* 3) Start with a Grid full of Cells in state Blocked.
* 4) Pick a random Cell, set it to state Passage and Compute its frontier cells. A frontier cell of a Cell is a cell with distance 2 in state Blocked and within the grid.
* 5) While the list of frontier cells is not empty:
* 	5.1) Pick a random frontier cell from the list of frontier cells.
*	5.2) Let neighbors(frontierCell) = All cells in distance 2 in state Passage.
*		 Pick a random neighbor and connect the frontier cell with the neighbor by setting the cell in-between to state Passage.
*		 Compute the frontier cells of the chosen frontier cell and add them to the frontier list. Remove the chosen frontier cell from the list of frontier cells.
*/

class coordinate {
public:
	int x;
	int y;
	coordinate(int x, int y) {
		this->x = x;
		this->y = y;
	}
};

enum class mazeState {
	BLOCKED, PASSAGE
};

void printGrid(std::vector<std::vector<mazeState>>* grid);
void createGrid(int width, int height, std::vector<std::vector<mazeState>>* grid);
int getRandomInt(int min, int max);
void findFrontierCells(std::vector<std::vector<mazeState>>* grid, int x, int y, std::vector<coordinate>* frontierCells);
void printFrontierList(std::vector<coordinate>* frontierCells);
void findNeighbours(std::vector<std::vector<mazeState>>* grid, int x, int y, std::vector<coordinate>* frontierCells);

void generateMaze(int width = 50, int height = 50) {
	std::vector<coordinate> frontierCells = std::vector<coordinate>();

	//initialize the dimensions
	std::vector<std::vector<mazeState>>* grid = new std::vector<std::vector<mazeState>>;
	createGrid(width, height, grid);
	//pick a startpoint
	coordinate startPoint = coordinate(getRandomInt(0, width - 1), getRandomInt(0, height - 1));

	//toggle the startposition
	grid->at(startPoint.x).at(startPoint.y) = mazeState::PASSAGE;
	findFrontierCells(grid, startPoint.x, startPoint.y, &frontierCells);

	while (frontierCells.size() != 0) {
		int pos = getRandomInt(0, frontierCells.size() - 1);
		coordinate chosenCell = frontierCells.at(pos);
		frontierCells.erase(frontierCells.begin() + pos);
		std::vector<coordinate> neighbourCells = std::vector<coordinate>();
		findNeighbours(grid, chosenCell.x, chosenCell.y, &neighbourCells);

		int neighbourPos = getRandomInt(0, neighbourCells.size() - 1);
		coordinate neighbourCell = neighbourCells.at(neighbourPos);
		grid->at(chosenCell.x).at(chosenCell.y) = mazeState::PASSAGE;
		grid->at((neighbourCell.x + chosenCell.x) / 2).at((neighbourCell.y + chosenCell.y) / 2) = mazeState::PASSAGE;
		findFrontierCells(grid, chosenCell.x, chosenCell.y, &frontierCells);
		for (int i = 0; i < frontierCells.size(); i++) {
			if (grid->at(frontierCells.at(i).x).at(frontierCells.at(i).y) == mazeState::PASSAGE) {
				frontierCells.erase(frontierCells.begin() + i);
			}
		}
	}
	printGrid(grid);
}

void findNeighbours(std::vector<std::vector<mazeState>>* grid, int x, int y, std::vector<coordinate>* frontierCells) {
	if (y + 2 < grid->at(x).size() && grid->at(x).at(y + 2) == mazeState::PASSAGE) {
		frontierCells->push_back(coordinate(x, y + 2));
	}
	if (y - 2 >= 0 && grid->at(x).at(y - 2) == mazeState::PASSAGE) {
		frontierCells->push_back(coordinate(x, y - 2));
	}
	if (x + 2 < grid->size() && grid->at(x + 2).at(y) == mazeState::PASSAGE) {
		frontierCells->push_back(coordinate(x + 2, y));
	}
	if (x - 2 >= 0 && grid->at(x - 2).at(y) == mazeState::PASSAGE) {
		frontierCells->push_back(coordinate(x - 2, y));
	}
}

void findFrontierCells(std::vector<std::vector<mazeState>>* grid, int x, int y, std::vector<coordinate>* frontierCells) {
	if (y + 2 < grid->at(x).size() - 1 && grid->at(x).at(y + 2) == mazeState::BLOCKED) {
		frontierCells->push_back(coordinate(x, y + 2));
	}
	if (y - 2 >= 0 + 1 && grid->at(x).at(y - 2) == mazeState::BLOCKED) {
		frontierCells->push_back(coordinate(x, y - 2));
	}
	if (x + 2 < grid->size() - 1 && grid->at(x + 2).at(y) == mazeState::BLOCKED) {
		frontierCells->push_back(coordinate(x + 2, y));
	}
	if (x - 2 >= 0 + 1 && grid->at(x - 2).at(y) == mazeState::BLOCKED) {
		frontierCells->push_back(coordinate(x - 2, y));
	}
}

void printFrontierList(std::vector<coordinate>* frontierCells) {
	for (int i = 0; i < frontierCells->size(); i++) {
		std::cout << frontierCells->at(i).x << " " << frontierCells->at(i).y << std::endl;
	}
}

void printGrid(std::vector<std::vector<mazeState>>* grid) {
	std::ofstream outfile("maze.txt");
	for (int i = 0; i < grid->at(0).size(); i++) {
		for (int j = 0; j < grid->size(); j++) {
			if (grid->at(j).at(i) == mazeState::BLOCKED) {
				outfile << '#';
			}
			else {
				outfile << ' ';
			}
		}
		outfile << std::endl;
	}
	outfile.close();
}

/**
* Create a grid
*/
void createGrid(int width, int height, std::vector<std::vector<mazeState>>* grid) {
	for (int i = 0; i < width; i++) {
		std::vector<mazeState> mazeRow = std::vector<mazeState>();
		for (int j = 0; j < height; j++) {
			mazeRow.push_back(mazeState::BLOCKED);
		}
		grid->push_back(mazeRow);
	}
}

/**
* Generate a random integer
*/
int getRandomInt(int min, int max) {
	//initialize generator
	std::random_device rd;
	std::default_random_engine eng(rd());

	// set range
	std::uniform_int_distribution<int> distr(min, max);

	// generate integer
	return distr(eng);
}