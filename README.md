# OSM Pathfinding Visualizer

An interactive, web-based map browser (similar to Google Maps) that visualize real-world pathfinding algorithms on OpenStreetMap data.

---

## Goal

Click two points on the map and watch for the routing algorithm to find a path in real time. The visualizer streams each step of the alogirhtm so that you can see where the search had expanded to. From there, the nodes being explored will find a final route, which would be displayed.

Supports switching between algorithms mid-session to compare how each one behaves on the same graph.

---

## Current Stack

This is a current idea of what I'm planning to use, as such, things may be subject to change during development.

| Layer | Technology |
|---|---|
| Frontend | React + TypeScript + Vite |
| Map rendering | Mapbox GL / Leaflet |
| Backend | C++ (Crow HTTP server) |
| Routing algorithms | Dijkstra's, A*, Bidirectional A* |
| Map data | OpenStreetMap (PBF via libosmium) |
| Deployment | Vercel (frontend) · Fly.io (backend) |

---

## Algorithms Planned

- **Dijkstra's** — explores all directions equally; guaranteed shortest path
- **A\*** — uses straight-line distance as a heuristic to guide the search; faster than Dijkstra's
- **Bidirectional A\*** — runs two simultaneous searches from both ends; meets in the middle
The visualization makes the difference between these tangible — you can see A* explore significantly fewer nodes than Dijkstra's on the same route.

## Running locally
 
**Prerequisites:** CMake, a C++17 compiler, Node.js 18+
 
```bash
# 1. Download an OSM extract (example: Austin, TX ~60 MB)
#    https://download.geofabrik.de/north-america/us/texas.html
 
# 2. Build and start the backend
cd backend
cmake -B build && cmake --build build
./build/server --data ../data/austin.osm.pbf
 
# 3. Start the frontend (in a separate terminal)
cd frontend
npm install && npm run dev
```
 
Open `http://localhost:5173`.
 
---
 
*More information will be added throughout development*