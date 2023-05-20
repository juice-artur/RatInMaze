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
};

struct QueueNode {
  Point Point;
  int Dist;
};

const int RowNum[] = {-1, 0, 0, 1};
const int ColumnNum[] = {0, -1, 1, 0};

std::mutex WriteMutex;

bool IsValid(int Row, int Column, int Size);

int BFS(std::vector<std::vector<int>> Maze, Point Src, Point Dest,
        int const size);

bool SolveMaze(int Size, std::vector<std::vector<int>>  Maze);

void WriteToFile(std::ofstream &File, int Data);

void CalculateRejections(std::atomic_unsigned_lock_free &NumberOfLaunches,
                         const int Size,
                         std::ofstream &File);

int main() {
  std::cout << "Enter size: " << std::endl;

  int InputSize = 0;

  std::cin >> InputSize;

  const int Size = InputSize;

  std::atomic_unsigned_lock_free NumberOfLaunches{0};

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
  auto Start = std::chrono::high_resolution_clock::now();
  for (auto &t : Theards) {
    t.join();
  }
  auto Stop = std::chrono::high_resolution_clock::now();

  auto Duration = duration_cast<std::chrono::microseconds>(Stop - Start);

  std::cout << "Duration" << Duration.count() << std::endl;

  File.close();
}

bool IsValid(int Row, int Column, int Size) {
  return (Row >= 0) && (Row < Size) && (Column >= 0) && (Column < Size);
}

int BFS(std::vector<std::vector<int>> Maze, Point Src, Point Dest,
        int const size) {
  if (!Maze[Src.x][Src.y] || !Maze[Dest.x][Dest.y]) return -1;

  std::vector<std::vector<bool>> Visited;
  Visited.resize(size);
  for (int i = 0; i < size; i++) {
    Visited[i].resize(size);
  }
  Visited[Src.x][Src.y] = true;

  std::queue<QueueNode> Queue;

  QueueNode s = {Src, 0};
  Queue.push(s);

  while (!Queue.empty()) {
    QueueNode Current = Queue.front();
    Point pt = Current.Point;

    if (pt.x == Dest.x && pt.y == Dest.y ) return Current.Dist;

    Queue.pop();

    for (int i = 0; i < 4; i++) {
      int Row = pt.x + RowNum[i];
      int Column = pt.y + ColumnNum[i];
      if (IsValid(Row, Column, size) && Maze[Row][Column] &&
          !Visited[Row][Column]) {
        Visited[Row][Column] = true;
        QueueNode Adjcell = {{Row, Column}, Current.Dist + 1};
        Queue.push(Adjcell);
      }
    }
  }

  return -1;
}

bool SolveMaze(int Size, std::vector<std::vector<int>> Maze) {
  std::vector<Point> Sources;
  std::vector<Point> Dests;
  for (int i = 0; i < Size; i++) {
      Sources.push_back({0, i});
    Dests.push_back({Size - 1, i});
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

void CalculateRejections(std::atomic_unsigned_lock_free &NumberOfLaunches,
                         const int Size,
                         std::ofstream &File) {
  while (NumberOfLaunches.load() < 100) {
    int NumberAttempts = 0;

    std::vector<std::vector<int>>  Maze;
    Maze.resize(Size);

    for (int i = 0; i < Size; i++) {
      Maze[i].resize(Size);
    }

    for (size_t i = 0; i < Size; i++) {
      for (size_t j = 0; j < Size; j++) {
          Maze[i][j] = 1;
        
      }
    }

    while (SolveMaze(Size, Maze)) {
      std::random_device RandomDevice;
      std::mt19937 Gen(RandomDevice());
      std::uniform_int_distribution<> Distribution(0, Size - 1);

      int i = 0;
      int j = 0;

      do {
        i = Distribution(Gen);
        j = Distribution(Gen);
      } while (Maze[i][j] != 1);

      Maze[i][j] = 0;
      NumberAttempts++;
    }

    WriteToFile(File, NumberAttempts);
    NumberOfLaunches.fetch_add(1);
  }
}