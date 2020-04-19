// TSP Genetic Algrorithm.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <Windows.h>

#include <cassert>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <limits>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <random>
#include <iterator>
#include <algorithm>
#include <vector>

#define internal static
#define persistent static

constexpr int populationSize = 1 << 12; //4096


struct Point {
    float x;
    float y;
};

struct Member {
    std::vector<int> nodeOrder;
    float fitness;
};

internal void printArray(std::vector<int> array) {
    for (int item : array) {
        printf("[ %i ]", item);
    }
}

internal void printNodesArray(std::vector<Point> nodes) {
    printf("Nodes: \n");
    for (const Point& node : nodes) {
        printf(" { x: %f, y: %f } \n", node.x, node.y);
    }

}

internal char* getCurrentWorkingDirectory() {
    char cwd[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, cwd);
    return cwd;
}

internal void generateNodesFromFile(std::vector<Point>& nodes, const char path[]) {

    std::string cwd(getCurrentWorkingDirectory());
    std::string dir(cwd + "\\" + path);
    try {

        std::ifstream in_file(dir);
        int nodeIndex = 0;

        if (in_file) {
            std::string line;
            while (std::getline(in_file, line)) {
                std::istringstream  is_line(line);
                std::string x_pos;
                std::getline(is_line, x_pos, ' ');
                float x = std::stof(x_pos);

                std::string y_pos;
                std::getline(is_line, y_pos);
                float y = std::stof(y_pos);

                nodes.emplace_back(Point({ x, y }));
            }

            in_file.close();

        }
        else {
            std::cout << "ERROR: File not found:" << dir << "\n";
        }
    }
    catch (std::ifstream::failure e) {
        std::cerr << "Exception opening/reading/closing file\n";
    }


}




internal float randomFloat() {
    float random = static_cast<float>(std::rand()) / RAND_MAX;
    return random;
}

internal float randomFloat(float min, float max) {

    float random = min + static_cast<float> (std::rand()) / static_cast<float>(RAND_MAX / (max - min));
    return random;
}


internal float getDistanceSquared(Point A, Point B) {
    float x2 = std::abs(B.x - A.x) * std::abs(B.x - A.x);
    float y2 = std::abs(B.y - A.y) * std::abs(B.y - A.y);

    return (x2 + y2);
}

internal float getDistance(Point a, Point b) {
    const float x = std::abs(b.x - a.x);
    const float y = std::abs(b.y - a.y);

    return std::hypot(x, y);
}

internal void shuffleArray(std::vector<int>& arr) {
    // obtain a time-based seed:
    unsigned seed = static_cast<int>(std::chrono::system_clock::now().time_since_epoch().count());

    shuffle(arr.begin(), arr.end(), std::default_random_engine(seed));
}




internal void initializeNodeOrder(std::vector<int>& nodeOrder, std::vector<Point>& nodes) {
    uint16_t nodesInUse = static_cast<uint16_t> (nodes.size());
    for (uint16_t i = 0; i < nodesInUse; i++) {
        nodeOrder.emplace_back(i);
    }

}


internal void generateRandomNodes(std::vector<Point>& nodes) {
    int maxNodes = 16;
    for (int nodeIndex = 0; nodeIndex < maxNodes; nodeIndex++) {
        Point node = {};
        node.x = randomFloat(1.0f, 800.0f);
        node.y = randomFloat(1.0f, 600.0f);

        nodes.emplace_back(node);
    }
}

internal void generateDefaultNodes(std::vector<Point>& nodes) {
    int maxNodes = 16;
    int width = 600;
    for (int nodeIndex = 0; nodeIndex < maxNodes; nodeIndex++) {
        Point node = {};
        node.x = 10.f * (nodeIndex % width);
        node.y = 9.f * (nodeIndex % width);

        nodes.emplace_back(node);
    }
}


internal void fillPopulation(std::vector<Member>& population, std::vector<int>& nodeOrder) {

    for (int populationIndex = 0; populationIndex < populationSize; populationIndex++) {
        shuffleArray(nodeOrder);
        Member member = { nodeOrder, 1.0 };
        population.emplace_back(member);
    }
}


internal float calculateRouteDistance(const std::vector<Point>& nodes, std::vector<int> order) {
    float totalDistance = 0.0f;

    for (uint16_t i = 0; i < static_cast<int>(order.size()) - 1; i++) {
        const uint16_t i_second = i + 1;
        const uint16_t index_a = order[i];
        const uint16_t index_b = order[i_second];

        const Point node_a = nodes[index_a];
        const Point node_b = nodes[index_b];

        float distance = getDistance(node_a, node_b);

        totalDistance += distance;
    }
    return totalDistance;
}

internal void calculateFitness(std::vector<Member>& population, std::vector<Point>& nodes, float& shortestDistance, std::vector<int>& bestRoute) {
    for (Member& member : population) {
        float distance = calculateRouteDistance(nodes, member.nodeOrder);
        if (distance < shortestDistance) {
            shortestDistance = distance;
            bestRoute = member.nodeOrder;
        }

        float normalized = 1.f / (distance + 1.f);
        member.fitness = normalized;
    }
}

internal void normalizeFitness(std::vector<Member>& population) {
    float sum = 0.f;
    for (const Member& member : population) {
        sum += member.fitness;
    }

    for (Member& member : population) {
        member.fitness = member.fitness / sum;
    }
}

internal std::vector<int> pickOrderFromPopulation(std::vector<Member>& population) {
    int index = 0;
    float r = randomFloat();

    // TODO: nodes.size() as argument
    while (r > 0.f && index < population.size()) {
        r = r - population[index].fitness;
        index++;
    }

    index--;

    index = (index > (populationSize - 1)) ? populationSize - 1 : index;
    index = (index < 0) ? 0 : index;
    assert(index >= 0 && index < populationSize - 1);
    return population[index].nodeOrder;
}

internal void mutateOrder(std::vector<int>& order) {
    //Simply swap two elements in the order
    const uint16_t index_a = rand() % (static_cast<uint16_t> (order.size()) - 1);
    const uint16_t index_b = (index_a + 1) % order.size();

    std::swap(order[index_a], order[index_b]);
}

internal void nextGeneration(std::vector<Member>& population) {


    for (Member& member : population) {
        std::vector<int> order = pickOrderFromPopulation(population);
        mutateOrder(order);
        member.nodeOrder = order;
    }
}

internal void outputJSON(std::vector<int> route) {
    printf("[ ");

    uint16_t count{ 0 };
    for (const int index : route) {
        printf("%i", index);
        if (count < route.size() - 1) printf(", ");
        count++;
    }

    printf(" ]");
}



int main(int argc, char* args[])
{
    std::srand((uint32_t)std::time(0));

    std::vector<Point> nodes{};
    std::vector<int> nodeOrder{};

    std::vector<Member> population{};

    float shortestDistance = (std::numeric_limits<float>::max)();
    std::vector<int> bestRoute{};

    generateNodesFromFile(nodes, "test.txt");
    initializeNodeOrder(nodeOrder, nodes);
    fillPopulation(population, nodeOrder);



    int iterations = args[1] ? std::stoi(args[1]) : 1000;
    while (iterations--) {
        calculateFitness(population, nodes, shortestDistance, bestRoute);
        normalizeFitness(population);
        nextGeneration(population);
    }

    outputJSON(bestRoute);

}