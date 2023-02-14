#include <array>
#include <fstream>
#include <iostream>
#include <mutex>
#include <queue>
#include <random>
#include <sstream>

struct Point {
  int x;
  int y;
  int z;
};

struct QueueNode {
  Point Point;
  int Dist;
};

const int RowNum[] = {-1, 0, 0, 1};
const int ColumnNum[] = {0, -1, 1, 0};
const int LayersNum[] = {1, 0, -1, 0};

std::mutex WriteMutex;

bool IsValid(int Row, int Column, int Layer, int Size);

int BFS(std::vector<std::vector<std::vector<int>>> Maze, Point Src, Point Dest,
        int const size);

bool SolveMaze(int Size, std::vector<std::vector<std::vector<int>>> Maze);

void WriteToFile(std::ofstream &File, int Data);

void CalculateRejections(int &NumberOfLaunches, const int Size,
                         std::ofstream &File);

int main() {
  std::cout << "Enter size: " << std::endl;

  int InputSize = 0;

  std::cin >> InputSize;

  const int Size = InputSize;

  int NumberOfLaunches = 0;

  std::stringstream FileName("Maze");

  FileName << "Maze" << InputSize << "x" << InputSize << "x" << InputSize
           << ".txt";

  std::ofstream File;

  // If we use half of the available threads
  // on the processor, we get the highest productivity
  const unsigned int NumberTheards = std::thread::hardware_concurrency() / 2;

  std::cout << "The program was launched in: " << NumberTheards << " streams"
            << std::endl;

  File.open(FileName.str(), std::fstream::app);
  std::vector<std::thread> Theards(NumberTheards);
  for (auto &t : Theards) {
    t = std::thread(&CalculateRejections, std::ref(NumberOfLaunches), Size,
                    std::ref(File));
  }

  for (auto &t : Theards) {
    t.join();
  }
  File.close();
}

bool IsValid(int Row, int Column, int Layer, int Size) {
  return (Row >= 0) && (Row < Size) && (Column >= 0) && (Column < Size) &&
         (Layer >= 0) && (Layer < Size);
}

int BFS(std::vector<std::vector<std::vector<int>>> Maze, Point Src, Point Dest,
        int const size) {
  if (!Maze[Src.x][Src.y][Src.z] || !Maze[Dest.x][Dest.y][Dest.z]) return -1;

  std::vector<std::vector<std::vector<bool>>> Visited;
  Visited.resize(size);
  for (int i = 0; i < size; i++) {
    Visited[i].resize(size);
    for (int j = 0; j < size; j++) Visited[i][j].resize(size);
  }
  Visited[Src.x][Src.y][Src.z] = true;

  std::queue<QueueNode> Queue;

  QueueNode s = {Src, 0};
  Queue.push(s);

  while (!Queue.empty()) {
    QueueNode Current = Queue.front();
    Point pt = Current.Point;

    if (pt.x == Dest.x && pt.y == Dest.y && pt.z == Dest.z) return Current.Dist;

    Queue.pop();

    for (int i = 0; i < 4; i++) {
      int Row = pt.x + RowNum[i];
      int Column = pt.y + ColumnNum[i];
      int Layer = pt.z + LayersNum[i];
      if (IsValid(Row, Column, Layer, size) && Maze[Row][Column][Layer] &&
          !Visited[Row][Column][Layer]) {
        Visited[Row][Column][Layer] = true;
        QueueNode Adjcell = {{Row, Column, Layer}, Current.Dist + 1};
        Queue.push(Adjcell);
      }
    }
  }

  return -1;
}

bool SolveMaze(int Size, std::vector<std::vector<std::vector<int>>> Maze) {
  std::vector<Point> Sources;
  std::vector<Point> Dests;
  for (int i = 0; i < Size; i++) {
    for (int j = 0; j < Size; j++) {
      Sources.push_back({0, i, j});
      Dests.push_back({Size - 1, i, j});
    }
  }

  for (auto i : Sources) {
    for (auto j : Dests) {
      int Dist = BFS(Maze, i, j, Size);
      if (Dist != -1) {
        return true;
      }
    }
  }

  return false;
};

void WriteToFile(std::ofstream &File, int Data) {
  WriteMutex.lock();
  File << Data << std::endl;
  WriteMutex.unlock();
}

void CalculateRejections(int &NumberOfLaunches, const int Size,
                         std::ofstream &File) {
  while (NumberOfLaunches < 1000) {
    int NumberAttempts = 0;

    std::vector<std::vector<std::vector<int>>> Maze;
    Maze.resize(Size);

    for (int i = 0; i < Size; i++) {
      Maze[i].resize(Size);
      for (int j = 0; j < Size; j++) Maze[i][j].resize(Size);
    }

    for (size_t i = 0; i < Size; i++) {
      for (size_t j = 0; j < Size; j++) {
        for (size_t k = 0; k < Size; k++) {
          Maze[i][j][k] = 1;
        }
      }
    }

    while (SolveMaze(Size, Maze)) {
      std::random_device RandomDevice;
      std::mt19937 Gen(RandomDevice());
      std::uniform_int_distribution<> Distribution(0, Size - 1);

      int i = 0;
      int j = 0;
      int k = 0;

      do {
        i = Distribution(Gen);
        j = Distribution(Gen);
        k = Distribution(Gen);
      } while (Maze[i][j][k] != 1);

      Maze[i][j][k] = 0;
      NumberAttempts++;
    }

    WriteToFile(File, NumberAttempts);
    WriteMutex.lock();
    NumberOfLaunches++;
    WriteMutex.unlock();
  }
}