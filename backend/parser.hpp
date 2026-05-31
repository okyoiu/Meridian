// Header file for data structures
#ifndef PARSER_HPP
#define PARSER_HPP

#include <vector>
#include <cstdint> // note: provides fixed-withd int
#include <string>

// A single point that is on the Earth
struct Node 
{
    int64_t osm_id; // massible global ID from OpenStreetMap
    double lat; // latitude
    double lon; // longitude
    uint32_t index; // tight internal index for our array
};

// A road connection between the nodes
struct Edge 
{
    uint32_t to; // internal index of the destination node (route)
    float weight; // distance or travel time (graph theory def.)
};

struct Graph
{
    std::vector<Node> nodes;
    std::vector<std::vector<Edge>> adjList; // adjacent List holds outdegree edges for node `i`
};

// Function declaration to parse the file for information
Graph parse_map_data(const std::string& file_path);

#endif // END_OF PARSER_HPP