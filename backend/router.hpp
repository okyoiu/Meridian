#ifndef ROUTER_HPP
#define ROUTER_HPP

#include "parser.hpp"
#include <vector>

// A struct for holding the final calculated given path
struct RouteResult {
    double total_distance_meters;
    std::vector<uint32_t> path_indices; // the sequence of internal node IDs to drive thru
    bool found; // `True` if a path exists, else false if unreachable
};

// The routing engine class (where the magic happens)
class Router {
    public: 
        // passing in the graph in by reference (bc why make a big copy of million nodes in memory lol)
        Router(const Graph& graph);

        // Dijkstra !! function
        RouteResult find_shortest_path(uint32_t start_index, uint32_t end_index);

        uint32_t find_nearest_node(double lat, double lon); // This will be O(n)
    private:
        const Graph& map_graph;
};

#endif