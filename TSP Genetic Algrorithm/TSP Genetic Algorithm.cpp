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

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;


constexpr u32 populationSize = 1 << 13; // 8192


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

internal float randomFloat() {
    float random = static_cast<float>(std::rand()) / RAND_MAX;
    return random;
}

internal float randomFloat(float min, float max) {

    float random = min + static_cast<float> (std::rand()) / static_cast<float>(RAND_MAX / (max - min));
    return random;
}



internal void generateRandomNodes(std::vector<Point>& nodes) {
    u16 maxNodes = 16;
    for (u16 nodeIndex = 0; nodeIndex < maxNodes; nodeIndex++) {
        Point node = {};
        node.x = randomFloat(1.0f, 800.0f);
        node.y = randomFloat(1.0f, 600.0f);

        nodes.emplace_back(node);
    }
}

internal void generateDefaultNodes(std::vector<Point>& nodes) {
    u16 maxNodes = 16;
    u32 width = 600;
    for (u16 nodeIndex = 0; nodeIndex < maxNodes; nodeIndex++) {
        Point node = {};
        node.x = 10.f * (nodeIndex % width);
        node.y = 9.f * (nodeIndex % width);

        nodes.emplace_back(node);
    }
}



internal char* getCurrentWorkingDirectory() {
    char cwd[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, cwd);
    return cwd;
}

internal std::string getExePath() {
    char buffer[MAX_PATH];
    GetModuleFileNameA(NULL, buffer, MAX_PATH);
    std::string::size_type pos = std::string(buffer).find_last_of("\\/");
    return std::string(buffer).substr(0, pos);
}


internal void generateNodesFromFile(std::vector<Point>& nodes, const char path[]) {

    std::string exePath(getExePath());
    std::string dir(exePath + "\\" + path);
    try {

        std::ifstream in_file(dir);
        u16 nodeIndex = 0;

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
            std::cout << "ERROR: File not found:" << dir << ". Falling back to default nodes. \n";
            generateDefaultNodes(nodes);
        }
    }
    catch (std::ifstream::failure e) {
        std::cerr << "Exception opening/reading/closing file\n";
    }


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
    u16 nodesInUse = static_cast<u16> (nodes.size());
    for (u16 i = 0; i < nodesInUse; i++) {
        nodeOrder.emplace_back(i);
    }

}




internal void fillPopulation(std::vector<Member>& population, std::vector<int>& nodeOrder) {

    for (u32 populationIndex = 0; populationIndex < populationSize; populationIndex++) {
        shuffleArray(nodeOrder);
        Member member = { nodeOrder, 1.0 };
        population.emplace_back(member);
    }
}


internal float calculateRouteDistance(const std::vector<Point>& nodes, std::vector<int> order) {
    float totalDistance = 0.0f;

    for (u32 i = 0; i < static_cast<int>(order.size()) - 1; i++) {
        const u32 i_second = i + 1;
        const u32 index_a = order[i];
        const u16 index_b = order[i_second];

        const Point node_a = nodes[index_a];
        const Point node_b = nodes[index_b];

        float distance = getDistanceSquared(node_a, node_b);

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

internal std::vector<int> pickBestOrderFromPopulation(std::vector<Member>& population) {
    Member *best = &population[0];
    float highestFitness = -1.0f;
    for (Member& member : population) {
        if (member.fitness > highestFitness) {
            best = &member;
            highestFitness = member.fitness;
        }
    }

    return best->nodeOrder;
}

internal std::vector<int> pickOrderFromPopulation(std::vector<Member>& population) {
    u32 index = 0;
    float r = randomFloat();

    // TODO: nodes.size() as argument
    while (r > 0.f && index < population.size()) {
        r = r - population[index].fitness;
        index++;
    }

    index--;

    index = (index > (populationSize - 1)) ? populationSize - 1 : index;
    assert(index >= 0 && index < populationSize - 1);
    return population[index].nodeOrder;
}

internal void mutateOrder(std::vector<int>& order) {
    //Simply swap two elements in the order
    const u16 index_a = rand() % (static_cast<u16> (order.size()) - 1);
    const u16 index_b = (index_a + 1) % order.size();

    std::swap(order[index_a], order[index_b]);
}

internal void nextGeneration(std::vector<Member>& population) {

    for (Member& member : population) {
        member.nodeOrder = pickBestOrderFromPopulation(population);
        mutateOrder(member.nodeOrder);
    }
}

internal void outputJSON(std::vector<int> route) {
    printf("[ ");

    u16 count{ 0 };
    u16 routeSize = static_cast<u16> ( route.size() );
    for (const u16 nodeIndex : route) {
        printf("%i", nodeIndex);
        if (count < routeSize - 1) printf(", ");
        count++;
    }

    printf(" ]");
}

struct Options {
    u32 iterations = 1000;
    u32 numBuckets = 128;
    u16 threads = 1;
};


internal Options parseArguments(char* args[], int argc) {
    Options options;
    

    if (argc > 1) {
        for (u16 argIndex = 1; argIndex < argc; argIndex++) {
            std::string arg(args[argIndex]);
            std::string::size_type pos = arg.find(":");
            std::string command = arg.substr(0, pos);
            std::string value = arg.substr(pos + 1);
            
            if (command == "iterations") {
                u16 iterations = std::stoi(value);
                options.iterations = iterations;
            }
            else if (command == "threads") {
                u16 threads = std::stoi(value);
                options.threads = threads;
            }

            //options.threads = args[2];
        }
    }

    return options;
}

struct Bucket {
    std::vector<Member> populationChunk;
    int bucketSize;
};

int main(int argc, char* args[])
{
    Options options = parseArguments(args, argc);

    std::srand((u32)std::time(0));

    std::vector<Point> nodes{};
    std::vector<int> nodeOrder{};

    std::vector<Member> population{};

    float shortestDistance = (std::numeric_limits<float>::max)();
    std::vector<int> bestRoute{};

    generateNodesFromFile(nodes, "coords.txt");
    initializeNodeOrder(nodeOrder, nodes);
    fillPopulation(population, nodeOrder);
    

    // Split into multiple buckets
    std::vector<Bucket> buckets;
    u16 numBuckets = options.numBuckets;
    u32 populationIndex = 0;
    for (u16 bucketIndex = 0; bucketIndex <  numBuckets; bucketIndex++) {
        u16 bucketSize = population.size() / numBuckets;
        
        Bucket bucket;
        bucket.bucketSize = bucketSize;

        // TODO: not just copy all the data
        for (; populationIndex < bucketSize; populationIndex++) {
            Member member = population[populationIndex];
            bucket.populationChunk.push_back(member);
        }

        buckets.push_back(bucket);
    }



    for (Bucket& bucket : buckets) {
        u32 iterations = options.iterations;
        while (iterations--) {
            calculateFitness(bucket.populationChunk, nodes, shortestDistance, bestRoute);
            normalizeFitness(bucket.populationChunk);
            nextGeneration(bucket.populationChunk);
        }

    }



    outputJSON(bestRoute);

    return 0;
}