// Source file for reading the OSM file
#include "parser.hpp"
#include <iostream>
#include <unordered_set>
#include <unordered_map> //added for ID-to_Index lookup
#include <cmath> // added for Haversine math

// libosmim core headers
// (errors may be highlighted, but you can ignore as makefile takes care of this)
#include <osmium/io/any_input.hpp>
#include <osmium/handler.hpp>
#include <osmium/visitor.hpp>
#include <osmium/osm/way.hpp>
#include <osmium/osm/node.hpp>

// MATH HELPER FUNCTIONS (for Haversine Formula)
constexpr double EARTH_RADIUS_METERS = 6371000.0;

double to_radians(double degrees) {
    return degrees * M_PI / 180.0;
}

double haversine_distance(double lat1, double lon1, double lat2, double lon2) {
    double dLat = to_radians(lat2 - lat1);
    double dLon = to_radians(lon2 - lon1);
    double a = std::sin(dLat/2) * std::sin(dLat/2) +
               std::cos(to_radians(lat1)) * std::cos(to_radians(lat2)) *
               std::sin(dLon/2) * std::sin(dLon/2);
    double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1-a));
    return EARTH_RADIUS_METERS * c; // Returns distance in meters
}

// Finding all the nodes that are part of a valid road network to location
class WayScannerHandler : public osmium::handler::Handler 
{
    public:
        // This hash-set will store the IDs of nodes that we actually care about
        std::unordered_set<int64_t> valid_node_ids;

        // this function is automatically called by libosmim everytime it reads a "Way"
        void way(const osmium::Way& current_way) {
            // This retrieves the value of the `highway` tag for the current way.
            const char* highway_tag = current_way.tags().get_value_by_key("highway");

            // If the highway is not `null`, then the way is valid
            if (highway_tag != nullptr) {
                // for each node, its unique ID is added to valid_node_ids set
                for (const auto& node_ref : current_way.nodes()) {
                    // Save the Node ID to the set
                    // note: `unordered_set` automatically prevents duplicates
                    valid_node_ids.insert(node_ref.ref());
                }
            }
        }
};

// Pass 2: extracting Coordinates and map Global IDs to tight array indices
class NodeMapperHandler : public osmium::handler::Handler 
{
    public:
        // references to the data we made in pass 1
        const std::unordered_set<int64_t>& valid_ids;
        std::vector<Node>& graph_nodes;

        // dictionary to map global ID -> array index
        std::unordered_map<int64_t, uint32_t>& id_to_index_map;

        // internal index counter
        uint32_t current_internal_index = 0;

        // constructor (initializing ids, nodes, AND the map)
        NodeMapperHandler(const std::unordered_set<int64_t>& ids, std::vector<Node>& nodes, std::unordered_map<int64_t, uint32_t>& map) 
        : valid_ids(ids), graph_nodes(nodes), id_to_index_map(map) {}

        // function is called every time libosmium sees a "Node"
        void node(const osmium::Node& current_node) {
            // check to see if node's ID is in our set of valid road nodes
            if (valid_ids.find(current_node.id()) != valid_ids.end()) {
                // If it passes then this is a valid node, so we create custom Node struct
                Node our_node;
                our_node.osm_id = current_node.id();
                our_node.lat = current_node.location().lat();
                our_node.lon = current_node.location().lon();
                our_node.index = current_internal_index;

                // once data is saved onto the node struct
                // we save it to our graph and increment the index to continue (saving data)
                graph_nodes.push_back(our_node);
                id_to_index_map[current_node.id()] = current_internal_index;
                current_internal_index++;
            }
        }
};

// Pass 3: edge builder
class EdgeBuilderHandler : public osmium::handler::Handler 
{
    public:
        const std::unordered_map<int64_t, uint32_t>& id_to_index_map;
        Graph& graph;

        EdgeBuilderHandler(const std::unordered_map<int64_t, uint32_t>& map, Graph& g) 
        : id_to_index_map(map), graph(g) {}
    
        void way(const osmium::Way& current_way) {
            const char* highway_tag = current_way.tags().get_value_by_key("highway");
            if (highway_tag == nullptr) { // ignores non-roads (so all unvalid roads)
                return;
            }
            if (current_way.nodes().size() < 2) { // a road must have at least 2 points
                return;
            }

            // checking if road is a one-way street
            bool is_oneway = false;
            const char* oneway_tag = current_way.tags().get_value_by_key("oneway");
            if (oneway_tag != nullptr && (std::string(oneway_tag) == "yes" || std::string(oneway_tag) == "1")) {
                is_oneway = true;
            }

            // loop thru road nodes in pairs (A->B, then B->C, like an adj list DFS)
            auto it = current_way.nodes().begin();
            auto prev = it++;

            for (; it != current_way.nodes().end(); ++it, ++prev) {
                auto prev_map_it = id_to_index_map.find(prev->ref());
                auto curr_map_it = id_to_index_map.find(it->ref());   
                
                // basically: if both Node A and B are valid in our system
                if (prev_map_it != id_to_index_map.end() && curr_map_it != id_to_index_map.end()) {
                    uint32_t index_A = prev_map_it->second;
                    uint32_t index_B = curr_map_it->second;

                    // calculate the distance in meters (m) using Haversine (formula)
                    double dist = haversine_distance(
                        graph.nodes[index_A].lat, graph.nodes[index_A].lon,
                        graph.nodes[index_B].lat, graph.nodes[index_B].lon
                    );
                    // Add the edge from A to B
                    graph.adjList[index_A].push_back({index_B, static_cast<float>(dist)});

                    // If it is NOT a one-way street, we must also add the reverse edge (B to A)
                    if (!is_oneway) {
                        graph.adjList[index_B].push_back({index_A, static_cast<float>(dist)});
                    }
                }
            }
        }
};

// Entry point for parsing the OSM data file
Graph parse_map_data(const std::string& file_path) 
{
    // Initializing the graph (will hold nodes and edges of the road network)
    Graph map_graph;

    // PASS 1: Finding all Valid Road Nodes
    std::cout << "Starting Pass 1: Scanning ways for valid roads..." << std::endl;
    WayScannerHandler pass1_handler;
    osmium::io::Reader reader1{file_path, osmium::osm_entity_bits::way}; // only reading "ways"
    osmium::apply(reader1, pass1_handler);
    reader1.close();

    std::cout << "Pass 1 Complete." << std::endl;
    std::cout << "Filtered out irrelevant data." << std::endl;
    std::cout << "Total valid road nodes found: " << pass1_handler.valid_node_ids.size() << std::endl;
        
    // PASS 2: Extracting Coordinates and Building Node Array
    std::cout << "\nStarting Pass 2: Extracting spatial coordinates..." << std::endl;
    std::unordered_map<int64_t, uint32_t> global_to_internal_map; // Created the map variable
    NodeMapperHandler pass2_handler(pass1_handler.valid_node_ids, map_graph.nodes, global_to_internal_map); // MODIFIED: Pass the new map as the third argument to Pass 2
    osmium::io::Reader reader2{file_path, osmium::osm_entity_bits::node}; // only reading "nodes"
    osmium::apply(reader2, pass2_handler);
    reader2.close();

    std::cout << "Pass 2 Complete. Successfully mapped spatial data." << std::endl;

    //! PREPARE FOR PASS 3 (CRITICAL OPTIMIZATION)
    // We must resize our adjacency list to match the number of nodes, 
    // otherwise pushing edges into it will crash the program.
    map_graph.adjList.resize(map_graph.nodes.size());

    // PASS 3
    std::cout << "\nStarting Pass 3: Building Edges & Calculating Distances..." << std::endl;
    EdgeBuilderHandler pass3_handler(global_to_internal_map, map_graph);
    osmium::io::Reader reader3{file_path, osmium::osm_entity_bits::way};
    osmium::apply(reader3, pass3_handler);
    reader3.close();
    std::cout << "Pass 3 Complete. Graph fully constructed!" << std::endl;
    return map_graph;
}