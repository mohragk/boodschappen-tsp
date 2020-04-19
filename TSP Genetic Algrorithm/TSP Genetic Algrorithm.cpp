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

constexpr uint16_t maxNodes = 128;
constexpr uint32_t populationSize = 1 << 12; //4096
constexpr uint32_t totalIterations = 1000;

uint16_t nodesInUse = 0;


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
        for (uint16_t i = 0; i < nodesInUse; i++){
            printf(" { x: %f, y: %f } \n", nodes[i].x, nodes[i].y);
        }

    }

    internal char* getCurrentWorkingDirectory() {
        char cwd[MAX_PATH];
        GetCurrentDirectoryA(MAX_PATH, cwd);
        return cwd;
    }

    internal void generateNodesFromFile(std::array<Point, maxNodes>& nodes, const char path[]) {


       
        std::string cwd(getCurrentWorkingDirectory());
        std::string dir(cwd + "\\"  + path) ;
        try {

            std::ifstream in_file(dir);
            uint16_t nodeIndex = 0;

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

                    nodes[nodeIndex] = { x, y };
                    nodeIndex += 1 % maxNodes;     
                    nodesInUse = nodeIndex;
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

        shuffle(arr.begin(), arr.begin()+nodesInUse, std::default_random_engine(seed));
    }




    internal void initializeNodeOrder(std::array<int, maxNodes>& nodeOrder) {
        for (uint16_t i = 0; i < nodesInUse; i++) {
            nodeOrder[i] = i;
        }
    }


    internal void generateRandomNodes(std::array<Point, maxNodes>& nodes) {

        for (int nodeIndex = 0; nodeIndex < maxNodes; nodeIndex++) {
            Point node = {};
            node.x = randomFloat(1.0f, 800.0f);
            node.y = randomFloat(1.0f, 600.0f);

            nodes[nodeIndex] = node;
        }
    }

    internal void generateDefaultNodes(std::array<Point, maxNodes>& nodes) {

        int width = 600;
        for (uint16_t nodeIndex = 0; nodeIndex < maxNodes; nodeIndex++) {
            Point node = {};
            node.x = 10.f * (nodeIndex % width);
            node.y = 9.f * (nodeIndex % width);

            nodes[nodeIndex] = node;
        }
    }


    internal void fillPopulation(std::array<Member, populationSize>& population) {

        for (uint32_t populationIndex = 0; populationIndex < populationSize; populationIndex++) {
            shuffleArray(nodeOrder);
            Member member = { nodeOrder, 1.0 };
            population[populationIndex] = member;
        }
    }


    internal float calculateRouteDistance(const std::array<Point, maxNodes>& nodes, std::array<int, maxNodes> order) {
        float totalDistance = 0.0f;

        for (uint16_t i = 0; i < nodesInUse - 1 ; i++) {
            const uint16_t i_next = i + 1;
            const uint16_t index_a = order[i];
            const uint16_t index_b = order[i_next];

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
        uint32_t index = 0;
        float r = randomFloat();

        while (r > 0.f && index < populationSize) {
            r = r - population[index].fitness;
            index++;
        }

        index--;

        index = (index > (populationSize - 1)) ? populationSize - 1 : index;
       
        assert(index >= 0 && index < populationSize - 1);
        return population[index].nodeOrder;
    }

    internal void mutateOrder(std::array<int, maxNodes>& order) {
        //Simply swap two elements in the order
        const uint16_t index_a = rand() % (nodesInUse - 1);
        const uint16_t index_b = (index_a + 1) % nodesInUse;

        std::swap(order[index_a], order[index_b]);
    }

    internal void nextGeneration(std::array<Member, populationSize>& population) {

        
        for (Member& member : population) {
            std::array<int, maxNodes> order = pickOrderFromPopulation(population);
            mutateOrder(order);

            member.nodeOrder = order;    
        }
    }

    internal void outputJSON(std::array<int, maxNodes> route) {
        printf("[ ");

        for (uint16_t i = 0; i < nodesInUse; i++ ){
            printf("%i", route[i]);
            if (i < nodesInUse - 1) printf(", ");
        }

        printf(" ]");
    }



int main( int argc, char* args[] )
{
    std::srand((uint32_t)std::time(0));

    
    generateNodesFromFile(nodes, "test.txt");
    initializeNodeOrder(nodeOrder);
    fillPopulation(population);
    printNodesArray(nodes);
   
    int iterations = totalIterations;
    while (iterations--) {
        calculateFitness(population);
        normalizeFitness(population);
        nextGeneration(population);
    }


    outputJSON(bestRoute);    
}