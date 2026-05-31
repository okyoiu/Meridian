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

    // declaring the graph outside so that it runs for the whole program
    Graph road_network;

    try {
        road_network = parse_map_data(data_path);

        std::cout << "\n--- MAP LOADING COMPLETE ---" << std::endl;
        std::cout << "Total Nodes Loaded: " << road_network.nodes.size() << std::endl;
        std::cout << "Total Adjacency Lists: " << road_network.adjList.size() << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error Loading map data: " << e.what() << std::endl;
        return 1;
    }

    // --- ADD THIS AFTER PRINTING TOTALS ---
    if (!road_network.nodes.empty()) {
        std::cout << "\n--- Data Sanity Check ---" << std::endl;
        
        // Check the very first node in memory
        Node first = road_network.nodes.front();
        std::cout << "First Node [0]: Global ID " << first.osm_id 
                  << " | Lat: " << first.lat 
                  << " | Lon: " << first.lon << std::endl;

        // Check the very last node in memory
        Node last = road_network.nodes.back();
        std::cout << "Last Node [" << road_network.nodes.size() - 1 << "]: Global ID " 
                  << last.osm_id 
                  << " | Lat: " << last.lat 
                  << " | Lon: " << last.lon << std::endl;
    }

    return 0;
}