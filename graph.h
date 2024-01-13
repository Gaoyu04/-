#pragma once

#include <iostream>
#include <vector>
#include <cstdlib>
#include <fstream>
#include <random>
#include <algorithm>


using namespace std;


class Graph {
public:
    int numVertices;
    std::vector<std::vector<char>> adjList;

public:
    Graph(int vertices) : numVertices(vertices) {
        adjList.resize(numVertices);
        for(int i=0; i<numVertices; i++){
            adjList[i].resize(numVertices,'0');
        }
    }

    void addEdge(int src, int dest) {
        adjList[src][dest] = '1';
        adjList[dest][src] = '1';
    }

    void generateConnectedGraph(int numEdges) {

        vector<int> visited;
        vector<int> nv;

        visited.push_back(0);
        for (int i = 1; i < numVertices; i++) {
            nv.push_back(i);
        }

        while (!nv.empty()) {
            //从已连通和未连通的节点中随机选取连接
            std::random_shuffle(visited.begin(), visited.end());
            std::random_shuffle(nv.begin(), nv.end());
            addEdge(visited.back(), nv.back());

            visited.push_back(nv.back());
            nv.pop_back();
        }

        // 添加边
        while (numEdges > 0&&(!nv.empty())) {
            int v1 = rand() % numVertices;
            int v2 = rand() % numVertices;
            if (v1 == v2) continue;
            if (count(adjList[v1].begin(), adjList[v1].end(), v2) == 0) {
                addEdge(v1, v2);
                numEdges--;
            }
        }

    }

    void printGraph() {
        for (int i = 0; i < numVertices; ++i) {
            std::cout << "Vertex " << i << ": ";
            for (int j : adjList[i]) {
                std::cout << j << " ";
            }
            std::cout << std::endl;
        }
    }

    void saveGraphToFile(const std::string& filename) {
        std::ofstream file(filename);
        if (file.is_open()) {
            for (int i = 0; i < numVertices; ++i) {
                for (int j : adjList[i]) {
                    file << char(j) << " ";
                }
                file << std::endl;
            }
            file.close();
            std::cout << "Graph saved to " << filename << std::endl;
        }
        else {
            std::cerr << "Unable to open file" << std::endl;
        }
    }
};

Graph getrandomgraph(int numVertices, int numEdges) {
    std::string filename = "graph.txt";

    Graph graph(numVertices);
    graph.generateConnectedGraph(numEdges);

    //graph.printGraph();
    graph.saveGraphToFile(filename);

    return graph;
}