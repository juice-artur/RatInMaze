#include <fstream>
#include <iostream>
#include <mutex>
#include <random>
#include <sstream>
#include <stack>
#include <vector>

std::mutex WriteMutex;
// Структура, що представляє вершину графу
struct Vertex {
  // Індекси сусідніх вершин
  std::vector<int> neighbors;
  // Індекс вершини, яка веде до поточної вершини
  int previous;
};

// Функція для перетворення лабіринту в граф
std::vector<Vertex> createGraphFromMaze(
    const std::vector<std::vector<int>>& maze) {
  // Розміри лабіринту
  int numRows = maze.size();
  int numCols = maze[0].size();

  // Створення графу з відповідною кількістю вершин
  std::vector<Vertex> graph(numRows * numCols);

  // Заповнення сусідів для кожної вершини
  for (int i = 0; i < numRows; i++) {
    for (int j = 0; j < numCols; j++) {
      int vertexIndex = i * numCols + j;

      // Перевірка можливості руху вгору
      if (i > 0 && maze[i][j] != 1) {
        int neighborIndex = (i - 1) * numCols + j;
        graph[vertexIndex].neighbors.push_back(neighborIndex);
      }

      // Перевірка можливості руху вниз
      if (i < numRows - 1 && maze[i][j] != 1) {
        int neighborIndex = (i + 1) * numCols + j;
        graph[vertexIndex].neighbors.push_back(neighborIndex);
      }

      // Перевірка можливості руху вліво
      if (j > 0 && maze[i][j] != 1) {
        int neighborIndex = i * numCols + (j - 1);
        graph[vertexIndex].neighbors.push_back(neighborIndex);
      }

      // Перевірка можливості руху вправо
      if (j < numCols - 1 && maze[i][j] != 1) {
        int neighborIndex = i * numCols + (j + 1);
        graph[vertexIndex].neighbors.push_back(neighborIndex);
      }
    }
  }

  return graph;
}

void removeRandomEdge(std::vector<Vertex>& graph, int size) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<int> dist(size, graph.size() - size);

  int randomVertex = dist(gen);
  if (!graph[randomVertex].neighbors.empty()) {
    std::uniform_int_distribution<int> distNeighbor(
        0, graph[randomVertex].neighbors.size() - 1);
    int randomNeighborIndex = distNeighbor(gen);
    int randomNeighbor = graph[randomVertex].neighbors[randomNeighborIndex];

    auto& neighbors = graph[randomVertex].neighbors;
    neighbors.erase(neighbors.begin() + randomNeighborIndex);

    auto& neighborNeighbors = graph[randomNeighbor].neighbors;
    auto it = std::find(neighborNeighbors.begin(), neighborNeighbors.end(),
                        randomVertex);
    if (it != neighborNeighbors.end()) {
      neighborNeighbors.erase(it);
    }
  }
}

std::vector<int> mazeSolver(std::vector<Vertex>& graph, int startVertex,
                            int exitVertex) {
  std::vector<bool> visited(graph.size(),
                            false);  // Масив для відвіданих вершин
  std::vector<int> path;  // Масив для зберігання шляху

  std::stack<int> stack;  // Стек для реалізації правила лівої руки
  stack.push(startVertex);
  visited[startVertex] = true;

  while (!stack.empty()) {
    int currentVertex = stack.top();

    if (currentVertex == exitVertex) {
      // Досягнуто вихідну вершину, формуємо шлях
      while (currentVertex != startVertex) {
        path.push_back(currentVertex);
        currentVertex = graph[currentVertex].previous;
      }
      path.push_back(startVertex);
      std::reverse(path.begin(),
                   path.end());  // Інвертуємо шлях, щоб отримати від початкової
                                 // до кінцевої вершини
      return path;
    }

    bool foundNext = false;
    for (int neighbor : graph[currentVertex].neighbors) {
      if (!visited[neighbor]) {
        stack.push(neighbor);
        graph[neighbor].previous = currentVertex;
        visited[neighbor] = true;
        foundNext = true;
        break;
      }
    }

    if (!foundNext) {
      stack.pop();
    }
  }

  return path;
}

void CalculateRejections(std::atomic_unsigned_lock_free& NumberOfLaunches,
                         const int Size, std::ofstream& File);
void WriteToFile(std::ofstream& File, int Data);

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

  const unsigned int NumberTheards = std::thread::hardware_concurrency() / 2;

  std::cout << "The program was launched in: " << NumberTheards << " streams"
            << std::endl;

  File.open(FileName.str(), std::fstream::app);
  std::vector<std::thread> Theards(NumberTheards);
  for (auto& t : Theards) {
    t = std::thread(&CalculateRejections, std::ref(NumberOfLaunches), Size,
                    std::ref(File));
  }
  auto Start = std::chrono::high_resolution_clock::now();
  for (auto& t : Theards) {
    t.join();
  }
  auto Stop = std::chrono::high_resolution_clock::now();

  auto Duration = duration_cast<std::chrono::microseconds>(Stop - Start);

  std::cout << "Duration" << Duration.count() << std::endl;

  return 0;
}

void WriteToFile(std::ofstream& File, int Data) {
  WriteMutex.lock();
  File << Data << std::endl;
  WriteMutex.unlock();
}

void CalculateRejections(std::atomic_unsigned_lock_free& NumberOfLaunches,
                         const int Size, std::ofstream& File) {
  while (NumberOfLaunches.load() < 100) {
    int NumberAttempts = 0;

    std::vector<std::vector<int>> Maze;
    Maze.resize(Size + 2);

    for (int i = 0; i < Size +2; i++) {
      Maze[i].resize(Size);
    }

    for (size_t i = 0; i < Size + 2; i++) {
      for (size_t j = 0; j < Size; j++) {
        Maze[i][j] = 0;
      }
    }

    std::vector<Vertex> graph = createGraphFromMaze(Maze);
    int startVertex = 0;
    int exitVertex = Maze.size() * Maze[0].size() - 1;

    while (!mazeSolver(graph, startVertex, exitVertex).empty()) {
      removeRandomEdge(graph, Size);
      NumberAttempts++;
    }

    WriteToFile(File, NumberAttempts);
    NumberOfLaunches.fetch_add(1);
  }
}