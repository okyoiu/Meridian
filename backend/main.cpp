// PURPOSE: main entry point for the program
#include <iostream>
#include <string>
#include <unordered_map>
#include "parser.hpp"
#include "router.hpp"

#include <crow.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// bringing in the global map from parser.cpp
extern std::unordered_map<int64_t, uint32_t> global_to_internal_map;

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

    // --- ROUTING ENGINE TEST ---
    std::cout << "\n--- Executing Dijkstra's Algorithm ---" << std::endl;
        
    // Boot up the router with our loaded map
    Router router(road_network);
    crow::SimpleApp app;
    // This is our API Endpoint. It listens for URLs like: /route?start=123&end=456
    // The [&] captures our router and graph by reference so the web server can use them.
    CROW_ROUTE(app, "/route") // so if someone visits "/route", then it executes the block of code
    ([&router, &road_network](const crow::request& req){ 
        
        // 1. Extract parameters from the URL
        // Crow automatically parses everything after the `?` into a dictionary called `url_params`
        auto start_param = req.url_params.get("start");
        auto end_param = req.url_params.get("end");


        if (!start_param || !end_param) return crow::response(400, "Missing parameters.");

        // Convert strings to 64-bit ints
        // because URLs are made up of text, `start_param` is a string. So you can use
        // `std::stoll` to convert that text into a 64-bit integer (`int` variables are too small to hold OSM IDs)
        int64_t start_osm_id = std::stoll(start_param);
        int64_t end_osm_id = std::stoll(end_param);

        // 2. Validate against our dictionary
        if (global_to_internal_map.find(start_osm_id) == global_to_internal_map.end() ||
            global_to_internal_map.find(end_osm_id) == global_to_internal_map.end()) {
            return crow::response(404, "Node ID not found.");
        }

        // 3. Execute Dijkstra!
        // The React app only knows the global OSM IDs but Dijkstra only knowns tiny array indices
        // so we use the dictionary in Pass 2 to translate the public IDs into the secrete array index
        uint32_t start_index = global_to_internal_map[start_osm_id];
        uint32_t end_index = global_to_internal_map[end_osm_id];
        RouteResult route = router.find_shortest_path(start_index, end_index);

        if (!route.found) return crow::response(404, "No path exists.");

        // 4. Serialize to JSON
        // we do this to send data over the internet since React doesn't get C++, so we serialize it
        json response_json;
        response_json["distance_meters"] = route.total_distance_meters;
        
        json geometry = json::array();
        // `for` loop looks @ array of indices Dijkstra returned and looks up their Latitude/Longitude on the graph and
        // packs them into a JSON array so React can draw them on a map
        for (uint32_t node_idx : route.path_indices) {
            const Node& n = road_network.nodes[node_idx];
            geometry.push_back({n.lat, n.lon}); // Send Lat/Lon pairs for React to draw
        }
        response_json["geometry"] = geometry;

        // 5. Send HTTP Response with CORS headers (critical for React)
        crow::response res(response_json.dump());
        res.set_header("Content-Type", "application/json");
        res.set_header("Access-Control-Allow-Origin", "*");// a security feature called CORS (Cross-Origin Resource Sharing) to prevent hacking (`*` means that it is a public API)
        return res;
    });

    std::cout << "\n API LIVE" << std::endl;
    // Starting the server
    app.port(8080).multithreaded().run(); // multithread so that Crow can assign different threads to calculate each route simultaneously rather than waiting

    return 0;
}