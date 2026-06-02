# Backend Engine for Project - Meridian 

The high-performance C++ backend for the Meridian pathfinding visualizer. This engine is responsible for parsing raw OpenStreetMap (OSM) `.pbf` data, constructing a memory-efficient graph structure, and executing routing algorithms (Dijkstra, A* Search) to calculate optimal paths.

##  Final Status: Map Parsing (3-Test system)
To hold 1.4 million nodes in RAM without crashing, the engine uses a highly optimized 3-Pass Parsing Strategy:- [x] **Pass 1 (Way Scanner):** Successfully filtering out irrelevant data (buildings, trees) and identifying all valid road nodes.
- [x] **Pass 2 (Node Mapper):** Successfully extracting spatial coordinates (Latitude/Longitude) and mapping 64-bit global OSM IDs to tight 32-bit sequential array indices.
- [x] **Pass 3 (Edge Connections):** *Next up.* Connecting the nodes to build the final Adjacency List and calculating edge weights using the Haversine formula.

## Routing & Algorithms
- **Nearest Neighbor Search:** An $O(N)$ spatial scan that translates physical Latitude/Longitude coordinate clicks from the web UI into the nearest valid road intersection in the graph.
- **Dijkstra's Algorithm:** An unoptimized implementation of Dijkstra's operating at $O(V + E \log V)$ using a C++ Priority Queue to guarantee the absolute shortest physical path between two nodes.

## API Documentation
The engine runs locally on `http://localhost:8080` and exposes the following endpoint:

### `GET /route`
Calculates the shortest path between two physical coordinates.
* **Query Parameters:**
  * `startLat` (double)
  * `startLon` (double)
  * `endLat` (double)
  * `endLon` (double)
* **Response:** Returns a JSON payload containing the total `distance_meters` and an array of `geometry` coordinates to draw the polyline.

## System Requirements & Dependencies

This project is configured for macOS (Apple Silicon) using Homebrew.

**Required Packages:**
```bash
brew install cmake
brew install libosmium
brew install protozero
```
*Note: The project also links against `zlib`, `expat`, and `bzip2`, which are typically included with the macOS Command Line Tools (more info [here](https://github.com/osmcode/osmium-tool)).*

## Build & Run Instructions

This project uses `CMake` for its build configurations.

To perform, run the following
```bash
# Ensure you are in the directory for backend
cd backend

# Remove any old previous cache (if any)
rm -rf build

# Configure CMake
cmake -B build

# Compile the C++ code and execute the server
cmake --build build && ./build/server
```

## Directory Structure
```
backend/
backend/
├── CMakeLists.txt      # Compiler instructions, automated dependency linking
├── main.cpp            # API Router (Crow), multithreaded server entry point
├── parser.hpp          # Graph data structures and Haversine math headers
├── parser.cpp          # libosmium Handlers for map parsing
└── router.cpp          # Algorithm implementations (Dijkstra, Nearest Neighbor)
```
