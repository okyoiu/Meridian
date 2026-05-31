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
            const char* highway_tag = current_way.tags().get_value_by_key("highway");

            if (highway_tag != nullptr) {
                for (const auto& node_ref : current_way.nodes()) {
                    // Save the Node ID to the set
                    // note: `unordered_set` automatically prevents duplicates
                    valid_node_ids.insert(node_ref.ref());
                }
            }
        }
};

Graph parse_map_data(const std::string& file_path) 
{
    Graph map_graph;

    std::cout << "Starting Pass 1: Scanning ways for valid roads..." << std::endl;
    
    // 1) Initialize our custom handler
    WayScannerHandler pass1_handler;
    
    // 2) Open the file. 
    // Optimization: We tell libosmium to ONLY read "Ways" right now and skip parsing Nodes or Relations.
    osmium::io::Reader reader{file_path, osmium::osm_entity_bits::way};
    
    // 3) Apply the handler to the file stream
    osmium::apply(reader, pass1_handler);
    
    // 4) Always cleanly close the reader
    reader.close();
    
    std::cout << "Pass 1 Complete." << std::endl;
    std::cout << "Filtered out irrelevant data." << std::endl;
    std::cout << "Total valid road nodes found: " << pass1_handler.valid_node_ids.size() << std::endl;
        
    return map_graph;
}