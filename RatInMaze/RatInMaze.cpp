#include <fstream>
#include <iostream>
#include <mutex>
#include <queue>
#include <random>
#include <sstream>
using namespace std;

struct Point {
  int x;
  int y;
  int z;
};

struct queueNode {
  Point pt;
  int dist;
};

bool isValid(int row, int col, int z, int size) {
  return (row >= 0) && (row < size) && (col >= 0) && (col < size) && (z >= 0) &&
         (z < size);
}

int rowNum[] = {-1, 0, 0, 1};
int colNum[] = {0, -1, 1, 0};
int zNum[] = {1, 0, -1, 0};
int BFS(vector<vector<vector<int>>> mat, Point src, Point dest,
        int const size) {
  if (!mat[src.x][src.y][src.z] || !mat[dest.x][dest.y][dest.z]) return -1;

  vector<vector<vector<bool>>> visited;
  visited.resize(size);
  for (int i = 0; i < size; i++) {
    visited[i].resize(size);
    for (int j = 0; j < size; j++) visited[i][j].resize(size);
  }
  visited[src.x][src.y][src.z] = true;

  queue<queueNode> q;

  queueNode s = {src, 0};
  q.push(s);

  while (!q.empty()) {
    queueNode curr = q.front();
    Point pt = curr.pt;

    if (pt.x == dest.x && pt.y == dest.y && pt.z == dest.z) return curr.dist;

    q.pop();

    for (int i = 0; i < 4; i++) {
      int row = pt.x + rowNum[i];
      int col = pt.y + colNum[i];
      int z = pt.z + zNum[i];
      if (isValid(row, col, z, size) && mat[row][col][z] &&
          !visited[row][col][z]) {
        visited[row][col][z] = true;
        queueNode Adjcell = {{row, col, z}, curr.dist + 1};
        q.push(Adjcell);
      }
    }
  }

  return -1;
}

mutex mtx;
bool solveMaze(int size, vector<vector<vector<int>>> mat) {
  std::vector<Point> sources;
  std::vector<Point> dests;
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      sources.push_back({0, i, j});
      dests.push_back({size - 1, i, j});
    }
  }

  for (auto i : sources) {
    for (auto j : dests) {
      int dist = BFS(mat, i, j, size);

      if (dist != -1) {
        return true;
      }
    }
  }

  return false;
};

void writeToFile(std::ofstream &myFile, int data) {
  mtx.lock();

  myFile << data << endl;

  mtx.unlock();
}


void Launch(int &NumberOfLaunches, const int size, int a,
            std::ofstream &myFile) {
  while (NumberOfLaunches < 1000) {
    int numberAttempts = 0;

    vector<vector<vector<int>>> mat;
    mat.resize(size);

    for (int i = 0; i < size; i++) {
      mat[i].resize(size);
      for (int j = 0; j < size; j++) mat[i][j].resize(size);
    }

    for (size_t i = 0; i < size; i++) {
      for (size_t j = 0; j < size; j++) {
        for (size_t k = 0; k < size; k++) {
          mat[i][j][k] = 1;
        }
      }
    }

    while (solveMaze(size, mat)) {
      random_device rd;
      mt19937 gen(rd());
      uniform_int_distribution<> distr(0, size - 1);

      int i = 0;
      int j = 0;
      int k = 0;

      do {
        i = distr(gen);
        j = distr(gen);
        k = distr(gen);
      } while (mat[i][j][k] != 1);

      mat[i][j][k] = 0;

      numberAttempts++;
    }

    writeToFile(myFile, numberAttempts);
    mtx.lock();

    NumberOfLaunches++;

    mtx.unlock();

  }
}




int main() {
  cout << "Enter size: " << endl;

  auto a = 0;

  cin >> a;

  const int size = a;

  int NumberOfLaunches = 0;

  std::stringstream fileName("Maze");

  fileName << "Maze" << a << "x" << a << ".txt";

  ofstream myFile;

  myFile.open(fileName.str(), fstream::app);

  for (int i = 0; i < 100; i++) {
    thread t1(&Launch, ref(NumberOfLaunches), size, i, ref(myFile));
    t1.join();
  }

  myFile.close();
}


