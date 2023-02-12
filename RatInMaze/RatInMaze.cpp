#include <iostream>
#include<queue>

#include <iostream>
#include <random>
#include <fstream>
#include <sstream>
using namespace std;

struct Point
{
	int x;
	int y;
};

struct queueNode
{
	Point pt;
	int dist;
};



bool isValid(int row, int col, int size)
{
	return (row >= 0) && (row < size) &&
		(col >= 0) && (col < size);
}

int rowNum[] = { -1, 0, 0, 1 };
int colNum[] = { 0, -1, 1, 0 };

int BFS(vector<vector<int>> mat, Point src, Point dest, int const size)
{
	if (!mat[src.x][src.y] || !mat[dest.x][dest.y])
		return -1;

	vector<vector<bool>> visited(size, vector<bool>(size));


	visited[src.x][src.y] = true;

	queue<queueNode> q;

	queueNode s = { src, 0 };
	q.push(s);

	while (!q.empty())
	{
		queueNode curr = q.front();
		Point pt = curr.pt;

		if (pt.x == dest.x && pt.y == dest.y)
			return curr.dist;

		q.pop();

		for (int i = 0; i < 4; i++)
		{
			int row = pt.x + rowNum[i];
			int col = pt.y + colNum[i];

			if (isValid(row, col, size) && mat[row][col] &&
				!visited[row][col])
			{
				visited[row][col] = true;
				queueNode Adjcell = { {row, col},
									  curr.dist + 1 };
				q.push(Adjcell);
			}
		}
	}

	return -1;
}


bool solveMaze(int size, vector<vector<int>> mat) {

	std::vector<Point> sources;
	std::vector<Point> dests;
	for (int i = 0; i < size; i++)
	{
		sources.push_back({ 0, i });
		dests.push_back({ size - 1, i });
	}


	for (auto i : sources)
	{
		for (auto j : dests)
		{
			int dist = BFS(mat, i, j, size);

			if (dist != -1)
			{
				return true;
			}
		}
	}

	return false;
};

int main()
{

	cout << "Enter size: " << endl;

	auto a = 0;

	cin >> a;

	const int size = a;

	int NumberOfLaunches = 0;

	std::stringstream fileName("Maze");

	fileName << "Maze" << a << "x" << a << ".txt";

	ofstream myFile;

	myFile.open(fileName.str(), fstream::app);


	while (NumberOfLaunches < 100)
	{
		int numberAttempts = 0;

		vector<vector<int>> mat(size, vector<int>(size));

		for (size_t i = 0; i < a; i++)
		{
			for (size_t j = 0; j < a; j++)
			{
				mat[i][j] = 1;
			}
		}


		while (solveMaze(size, mat))
		{

			random_device rd;
			mt19937 gen(rd());
			uniform_int_distribution<> distr(0, a - 1);

			int i = 0;
			int j = 0;

			do
			{
				i = distr(gen);
				j = distr(gen);
			} while (mat[i][j] != 1);

			mat[i][j] = 0;

			numberAttempts++;
		}
		myFile << numberAttempts << '\n';
		NumberOfLaunches++;
	}

	myFile.close();
}

