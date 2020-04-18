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
#include <array>

#define internal static
#define persistent static

constexpr int maxNodes = 256;
constexpr int populationSize = 1 << 12; //4096
constexpr int totalIterations = 10000;
int nodeCount = 0;




    struct Point {
        float x;
        float y;
    };

    struct Member {
        std::array<int, maxNodes> nodeOrder;
        float fitness;
    };

    internal void printArray(std::array<int, maxNodes> array) {
        for (int item : array) {
            printf("[ %i ]", item);
        }
    }

    internal void printNodesArray(std::array<Point, maxNodes> nodes) {
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

    internal std::array<Point, maxNodes> readDataFromText(const char path[]) {

        std::array<Point, maxNodes> locations{};

       
        std::string cwd(getCurrentWorkingDirectory());
        std::string dir(cwd + "\\"  + path) ;
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

                    locations[nodeIndex] = { x, y };
                    nodeIndex += 1 % maxNodes;     
                    nodeCount++;
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

        return locations;

    }

    std::array<int, maxNodes> nodeOrder{};
    std::array<Point, maxNodes> nodes{};

    std::array<Member, populationSize> population{};

    float shortestDistance = (std::numeric_limits<float>::max)();
    std::array<int, maxNodes> bestRoute{ };


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

    internal void shuffleArray(std::array<int, maxNodes>& arr) {
        // obtain a time-based seed:
        unsigned seed = static_cast<int>(std::chrono::system_clock::now().time_since_epoch().count());

        shuffle(arr.begin(), arr.begin()+nodeCount, std::default_random_engine(seed));
    }




    internal void initializeNodeOrder() {
        
        for (int i = 0; i < nodeCount; i++) {
            nodeOrder[i] = i;
        }
    }

    internal void generateNodesFromFile(std::array<Point, maxNodes>& nodes) {
        nodes = readDataFromText("test.txt");
    }


    // UNUSED
    internal void generateRandomNodes(std::array<Point, maxNodes>& nodes) {

        for (int nodeIndex = 0; nodeIndex < maxNodes; nodeIndex++) {
            Point node = {};
            node.x = randomFloat(1.0f, 800.0f);
            node.y = randomFloat(1.0f, 600.0f);

            nodes[nodeIndex] = node;
        }
    }

    // UNUSED
    internal void generateDefaultNodes(std::array<Point, maxNodes>& nodes) {

        int width = 600;
        for (int nodeIndex = 0; nodeIndex < maxNodes; nodeIndex++) {
            Point node = {};
            node.x = 10.f * (nodeIndex % width);
            node.y = 9.f * (nodeIndex % width);

            nodes[nodeIndex] = node;
        }
    }


    internal void generatePopulation() {

        for (int populationIndex = 0; populationIndex < populationSize; populationIndex++) {
            shuffleArray(nodeOrder);
            Member member = { nodeOrder, 1.0 };
            population[populationIndex] = member;
        }
    }


    internal float calculateRouteDistance(const std::array<Point, maxNodes>& nodes, std::array<int, maxNodes> order) {
        float totalDistance = 0.0f;

        for (int i = 0; i < nodeCount - 1; i++) {
            const int index_a = order[i];
            const int index_b = order[i + 1];

            const Point node_a = nodes[index_a];
            const Point node_b = nodes[index_b];

            float distance = getDistance(node_a, node_b);

            totalDistance += distance;
        }
        return totalDistance;
    }

    internal void calculateFitness(std::array<Member, populationSize>& population) {
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

    internal void normalizeFitness(std::array<Member, populationSize>& population) {
        float sum = 0.f;
        for (const Member& member : population) {
            sum += member.fitness;
        }

        for (Member& member : population) {
            member.fitness = member.fitness / sum;
        }
    }

    internal std::array<int, maxNodes> pickOrderFromPopulation(std::array<Member, populationSize>& population) {
        int index = 0;
        float r = randomFloat();

        while (r > 0.f && index < maxNodes) {
            r = r - population[index].fitness;
            index++;
        }

        index--;

        index = (index > (populationSize - 1)) ? populationSize - 1 : index;
        index = (index < 0) ? 0 : index;
        assert(index >= 0 && index < populationSize - 1);
        return population[index].nodeOrder;
    }

    internal void mutateOrder(std::array<int, maxNodes>& order) {
        //Simply swap two elements in the order
        const int index_a = min(rand(), (nodeCount - 1));
        const int index_b = min((index_a + 1) ,nodeCount);

        std::swap(order[index_a], order[index_b]);
    }

    internal void nextGeneration(std::array<Member, populationSize>& population) {

        int index = 0;
        for (const Member& member : population) {
            std::array<int, maxNodes> order = pickOrderFromPopulation(population);
            mutateOrder(order);

            float fitness = member.fitness;

            population[index].nodeOrder = order;
            population[index].fitness = fitness;
            index++;
        }
    }

    internal void outputJSON(std::array<int, maxNodes> route) {
        printf("[ ");

        const uint32_t routeSize = nodeCount - 1;
        int count = 0;
        for (const int index : route) {
            printf("%i", index);
            if (count < routeSize - 1) printf(", ");
            count++;
          
        }

        printf(" ]");
    }



int main( int argc, char* args[] )
{
    std::srand((uint32_t)std::time(0));

    
    generateNodesFromFile(nodes);
    initializeNodeOrder();
    generatePopulation();
   
    int iterations = totalIterations;
    while (iterations--) {
        calculateFitness(population);
        normalizeFitness(population);
        nextGeneration(population);
    }

    outputJSON(bestRoute);    
}