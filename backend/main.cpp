// PURPOSE: main entry point for the program
#include <iostream>
#include <string>
#include "parser.hpp"

// gateway entry for program
int main() 
{
    std::string data_path = "../data/austin.osm.pbf";

    std::cout << "Starting Meridian Routing Engine" << std::endl;
    std::cout << "Loading map data from: " << data_path << std::endl;

    try {
        Graph road_network = parse_map_data(data_path);

        std::cout << "\n--- MAP LOADING COMPLETE ---" << std::endl;
        std::cout << "Total Nodes Loaded: " << road_network.nodes.size() << std::endl;
        std::cout << "Total Adjacency Lists: " << road_network.adjList.size() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error Loading map data: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}