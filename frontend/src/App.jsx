import { useState, useEffect } from 'react'
import { MapContainer, TileLayer, Polyline, Marker, Popup } from 'react-leaflet'
import 'leaflet/dist/leaflet.css' // CRITICAL: This makes the map look like a map, not broken images

function App() {
  const [routeCoordinates, setRouteCoordinates] = useState([])
  const [distance, setDistance] = useState(0)

  // We want to fetch the data exactly once when the map first loads
  useEffect(() => {
    const fetchRoute = async () => {
      try {
        // Pinging your custom C++ Engine!
        const response = await fetch('http://localhost:8080/route?start=26546004&end=13894629882')
        if (!response.ok) throw new Error('Failed to fetch route')
        
        const data = await response.json()
        
        // Save the JSON arrays into React state so the map can draw them
        setRouteCoordinates(data.geometry)
        setDistance(data.distance_meters)
      } catch (error) {
        console.error("Error communicating with Meridian Engine:", error)
      }
    }

    fetchRoute()
  }, [])

  // Center the map roughly in Northwest Austin
  const mapCenter = [30.48, -97.76]

  return (
    <div style={{ position: 'relative', height: '100vh', width: '100vw' }}>
      
      {/* Floating UI overlay to show your C++ math */}
      <div style={{
        position: 'absolute', top: 20, left: 50, zIndex: 1000, 
        backgroundColor: 'white', padding: '15px', borderRadius: '8px', 
        boxShadow: '0px 4px 12px rgba(0,0,0,0.1)', fontFamily: 'sans-serif'
      }}>
        <h2 style={{ margin: '0 0 10px 0' }}>Meridian Engine</h2>
        <p style={{ margin: 0 }}><strong>Distance:</strong> {(distance / 1000).toFixed(2)} km</p>
        <p style={{ margin: 0, fontSize: '0.9em', color: 'gray' }}>Powered by C++ & Dijkstra</p>
      </div>

      {/* The Interactive Map */}
      <MapContainer center={mapCenter} zoom={13} style={{ height: '100%', width: '100%' }}>
        
        {/* The open-source map tiles (streets, buildings, etc.) */}
        <TileLayer
          attribution='&copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a> contributors'
          url="https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png"
        />

        {/* The blue line that represents your C++ route! */}
        {routeCoordinates.length > 0 && (
          <Polyline positions={routeCoordinates} color="blue" weight={5} opacity={0.7} />
        )}

      </MapContainer>
    </div>
  )
}

export default App