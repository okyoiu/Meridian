#include "router.hpp"
#include <iostream>
#include <queue>
#include <limits>
#include <algorithm>


// HELPER STRUCT: The min-heap element
struct QueueElement 
{
    double distance;
    uint32_t node_index;

    // Overload the > operator so the priority queue acts as a Min-Heap
    // (putting the smallest distance at the absolute top)
    bool operator>(const QueueElement& other) const {
        return distance > other.distance;
    }
};

// Construtor simply stores a reference to the massive map im sending in through here
Router::Router(const Graph& graph) : map_graph(graph){}

RouteResult Router::find_shortest_path(uint32_t start_index, uint32_t end_index) 
{
    // initializing given struct items
    RouteResult result;
    result.found = false;
    result.total_distance_meters = 0.0;

    uint32_t num_nodes = map_graph.nodes.size();
    if (start_index >= num_nodes || end_index >= num_nodes) { // checking if graph is invalid
        std::cerr << "Error: Invalid start or end index" << std::endl;
        return result;
    }

    std::cout << "Calculating route from Node " << start_index << " to Node " << end_index << "..." << std::endl;

    // implementing Dijkstra here
    // Tracking the shortest distance from the start to every other node
    const double INF = std::numeric_limits<double>::infinity();
    std::vector<double> distances(num_nodes, INF); // initialized a size of nodes to be INF

    // keeping track the predecessors of each node in the SPT (nodes w/ NO_PREV means unvisited or starting node)
    // - we use the max possible 4-byte int to represent "No Predecessor"
    const uint32_t NO_PREV = std::numeric_limits<uint32_t>::max();
    std::vector<uint32_t> previous(num_nodes, NO_PREV); // initialized a size of nodes to what the max of predecessors given

    // establishing the min-heap (Priority queue)
    std::priority_queue<QueueElement, std::vector<QueueElement>, std::greater<QueueElement>> min_heap;
    
    // initializing the start node
    distances[start_index] = 0.0;
    min_heap.push({0.0, start_index});

    // the algorithm start here
    while (!min_heap.empty()) {
        QueueElement current = min_heap.top();
        min_heap.pop();

        // for easier syntax (just to keep it simple)
        uint32_t u = current.node_index;

        if (u == end_index) {
            break; // leaves if we found shortest path already
        }

        // if we pull an old, longer distance from the heap, then ignore it
        if (current.distance > distances[u]) { // reminder: distances[u] is the shortest distance to node `u` that has been seen so far
            continue; // so we continue as long as we can find the shortest path node is found
        }

        // check all neighborint intersections
        for (const Edge& edge : map_graph.adjList[u]) {
            uint32_t v = edge.to;
            double new_dist = distances[u] + static_cast<double>(edge.weight);

            // relaxation: if we found a faster way to neighbor 'v', so we update it
            if (new_dist < distances[v]) {
                distances[v] = new_dist;
                previous[v] = u; // dropping and pointing back to 'u'
                min_heap.push({new_dist, v});
            }
        }
    }

    // reconstructing the path here
    if (distances[end_index] == INF) {
        std::cout << "No route exists between these points." << std::endl;
        return result;
    }
    result.found = true;
    result.total_distance_meters = distances[end_index];

    // backtrack from the destination, picking up breadcrumbs(my analogy) to the start
    uint32_t curr = end_index;
    while (curr != NO_PREV) {
        result.path_indices.push_back(curr);
        curr = previous[curr]; // moving backwards
    }

    // since the path was built from end to start, we reverse it
    std::reverse(result.path_indices.begin(), result.path_indices.end());

    std::cout << "Route calculated successfully!" << std::endl;
    return result;
}

uint32_t Router::find_nearest_node(double lat, double lon) {
    uint32_t best_index = 0;
    double min_distance = std::numeric_limits<double>::max();

    // we are going to scan 1.4 million nodes to find the closest one to the mouse click on Frontend
    for (uint32_t i = 0; i < map_graph.nodes.size(); ++i) {
        double dist = haversine_distance(lat, lon, map_graph.nodes[i].lat, map_graph.nodes[i].lon);

        if (dist < min_distance) {
            min_distance = dist;
            best_index = i;
        }
    }

    return best_index;
}