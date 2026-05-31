# Backend Engine for Project - Meridian 

The high-performance C++ backend for the Meridian pathfinding visualizer. This engine is responsible for parsing raw OpenStreetMap (OSM) `.pbf` data, constructing a memory-efficient graph structure, and executing routing algorithms (Dijkstra, A* Search) to calculate optimal paths.

##  Current Status: Map Parsing (Phase 1)
Currently implementing the **Two-Pass Parsing Strategy** to convert raw OSM database primitives into an in-memory graph.
- [x] **Pass 1 (Way Scanner):** Successfully filtering out irrelevant data (buildings, trees) and identifying all valid road nodes.
- [ ] **Pass 2 (Node Mapper):** Successfully extracting spatial coordinates (Latitude/Longitude) and mapping 64-bit global OSM IDs to tight 32-bit sequential array indices.
- [ ] **Pass 3 (Edge Connections):** *Next up.* Connecting the nodes to build the final Adjacency List and calculating edge weights using the Haversine formula.

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
├── CMakeLists.txt      # Compiler instructions, library linking, and find_path configs
├── main.cpp            # Application entry point
├── parser.hpp          # Graph data structures (Node, Edge, Graph)
└── parser.cpp          # libosmium Handlers (WayScannerHandler, NodeMapperHandler)
```
