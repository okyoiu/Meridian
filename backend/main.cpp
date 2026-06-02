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
    std::cout << "\n--- Engine Ready. Waiting for Web Requests... ---" << std::endl;
        
    // Boot up the router with our loaded map
    Router router(road_network);
    crow::SimpleApp app;

    // This is our API Endpoint. It listens for dynamic physical coordinates!
    CROW_ROUTE(app, "/route")
    ([&router, &road_network](const crow::request& req){
        
        // 1. Extract physical coordinates from the URL instead of IDs
        auto start_lat_param = req.url_params.get("startLat");
        auto start_lon_param = req.url_params.get("startLon");
        auto end_lat_param = req.url_params.get("endLat");
        auto end_lon_param = req.url_params.get("endLon");

        if (!start_lat_param || !start_lon_param || !end_lat_param || !end_lon_param) {
            return crow::response(400, "Missing coordinate parameters.");
        }

        double start_lat = std::stod(start_lat_param);
        double start_lon = std::stod(start_lon_param);
        double end_lat = std::stod(end_lat_param);
        double end_lon = std::stod(end_lon_param);

        // 2. Find the closest actual roads to the mouse clicks!
        uint32_t start_index = router.find_nearest_node(start_lat, start_lon);
        uint32_t end_index = router.find_nearest_node(end_lat, end_lon);

        // 3. Execute Dijkstra!
        RouteResult route = router.find_shortest_path(start_index, end_index);

        if (!route.found) return crow::response(404, "No path exists.");

        // 4. Serialize to JSON
        json response_json;
        response_json["distance_meters"] = route.total_distance_meters;
        
        json geometry = json::array();
        for (uint32_t node_idx : route.path_indices) {
            const Node& n = road_network.nodes[node_idx];
            geometry.push_back({n.lat, n.lon});
        }
        response_json["geometry"] = geometry;

        // 5. Send HTTP Response with CORS headers
        crow::response res(response_json.dump());
        res.set_header("Content-Type", "application/json");
        res.set_header("Access-Control-Allow-Origin", "*"); 
        return res;
    });

    std::cout << "\n API LIVE" << std::endl;
    // Starting the server
    app.port(8080).multithreaded().run();

    return 0;
}