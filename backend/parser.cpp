// Source file for reading the OSM file
#include "parser.hpp"
#include <iostream>
#include <unordered_set>

// libosmim core headers
// (errors may be highlighted, but you can ignore as makefile takes care of this)
#include <osmium/io/any_input.hpp>
#include <osmium/handler.hpp>
#include <osmium/visitor.hpp>
#include <osmium/osm/way.hpp>
#include <osmium/osm/node.hpp>

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

        // internal index counter
        uint32_t current_internal_index = 0;

        // constructor (initializing ids and nodes)
        NodeMapperHandler(const std::unordered_set<int64_t>& ids, std::vector<Node>& nodes) 
            : valid_ids(ids), graph_nodes(nodes) {}

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
                current_internal_index++;
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
    NodeMapperHandler pass2_handler(pass1_handler.valid_node_ids, map_graph.nodes);
    osmium::io::Reader reader2{file_path, osmium::osm_entity_bits::node}; // only reading "nodes"
    osmium::apply(reader2, pass2_handler);
    reader2.close();

    std::cout << "Pass 2 Complete. Successfully mapped spatial data." << std::endl;
  
    return map_graph;
}