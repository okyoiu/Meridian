import { useState, useEffect, useRef } from 'react'
import { MapContainer, TileLayer, Polyline, Marker, useMapEvents } from 'react-leaflet'
import 'leaflet/dist/leaflet.css'
import L from 'leaflet'
import './App.css';

// ── Leaflet icon fix ──────────────────────────────────────────────────────────
delete L.Icon.Default.prototype._getIconUrl
L.Icon.Default.mergeOptions({
  iconRetinaUrl: 'https://unpkg.com/leaflet@1.9.4/dist/images/marker-icon-2x.png',
  iconUrl:       'https://unpkg.com/leaflet@1.9.4/dist/images/marker-icon.png',
  shadowUrl:     'https://unpkg.com/leaflet@1.9.4/dist/images/marker-shadow.png',
})

// Custom pin icons
const makeIcon = (color) => L.divIcon({
  className: '',
  html: `<svg width="28" height="38" viewBox="0 0 28 38" fill="none" xmlns="http://www.w3.org/2000/svg">
    <path d="M14 0C6.268 0 0 6.268 0 14c0 10.5 14 24 14 24S28 24.5 28 14C28 6.268 21.732 0 14 0z" fill="${color}"/>
    <circle cx="14" cy="14" r="5" fill="white"/>
  </svg>`,
  iconSize: [28, 38],
  iconAnchor: [14, 38],
})

const START_ICON = makeIcon('#00d4ff')
const END_ICON   = makeIcon('#ff4d6d')

// ── Constants ─────────────────────────────────────────────────────────────────
const AUSTIN_BOUNDS = [[30.1, -98.0], [30.6, -97.5]]
const AUSTIN_CENTER = [30.2672, -97.7431]

const ALGORITHMS = [
  { id: 'dijkstra', label: "Dijkstra's",    desc: 'Explores all directions uniformly. Guaranteed shortest path.' },
  { id: 'astar',    label: 'A* Search',     desc: 'Heuristic-guided. Explores far fewer nodes than Dijkstra.' },
  { id: 'bidir',    label: 'Bidirectional', desc: 'Dual-frontier from both ends. Fastest convergence.' },
]


// ── MapClickHandler ───────────────────────────────────────────────────────────
function MapClickHandler({ onMapClick }) {
  useMapEvents({ click(e) { onMapClick([e.latlng.lat, e.latlng.lng]) } })
  return null
}

// ── Main App ──────────────────────────────────────────────────────────────────
export default function App() {
  const [tab, setTab]                     = useState('route')
  const [algorithm, setAlgorithm]         = useState('astar')
  const [startPoint, setStartPoint]       = useState(null)
  const [endPoint, setEndPoint]           = useState(null)
  const [routeCoords, setRouteCoords]     = useState([])
  const [stats, setStats]                 = useState(null)   // { distance_meters, nodes_visited, duration_ms }
  const [isLoading, setIsLoading]         = useState(false)
  const [error, setError]                 = useState(null)
  const errorTimer                        = useRef(null)

  const showError = (msg) => {
    setError(msg)
    clearTimeout(errorTimer.current)
    errorTimer.current = setTimeout(() => setError(null), 3500)
  }

  const handleMapClick = (coords) => {
    if (!startPoint || (startPoint && endPoint)) {
      setStartPoint(coords)
      setEndPoint(null)
      setRouteCoords([])
      setStats(null)
    } else {
      setEndPoint(coords)
      fetchRoute(startPoint, coords)
    }
  }

  const fetchRoute = async (start, end) => {
    setIsLoading(true)
    setRouteCoords([])
    setStats(null)
    try {
      const url = `http://localhost:8080/route?startLat=${start[0]}&startLon=${start[1]}&endLat=${end[0]}&endLon=${end[1]}&algorithm=${algorithm}`
      const res = await fetch(url)
      if (!res.ok) throw new Error(`Server error ${res.status}`)
      const data = await res.json()
      setRouteCoords(data.geometry)
      setStats({
        distance_meters: data.distance_meters,
        nodes_visited:   data.nodes_visited   ?? '—',
        duration_ms:     data.duration_ms     ?? '—',
      })
    } catch (e) {
      showError('Route calculation failed — is the backend running?')
      console.error(e)
    }
    setIsLoading(false)
  }

  const handleReset = () => {
    setStartPoint(null)
    setEndPoint(null)
    setRouteCoords([])
    setStats(null)
    setError(null)
  }

  const step = !startPoint ? 0 : !endPoint ? 1 : 2

  const fmtCoords = (pt) => pt ? `${pt[0].toFixed(4)}, ${pt[1].toFixed(4)}` : null

  return (
    <>
      <div className="app">

        {/* ── TOPBAR ─────────────────────────────────────────── */}
        <header className="topbar">
          <div className="topbar-logo">
            <svg width="22" height="22" viewBox="0 0 22 22" fill="none">
              <circle cx="11" cy="11" r="10" stroke="#00d4ff" strokeWidth="1.5"/>
              <circle cx="11" cy="11" r="4"  fill="#00d4ff"/>
              <line x1="11" y1="1"  x2="11" y2="5"  stroke="#00d4ff" strokeWidth="1.5"/>
              <line x1="11" y1="17" x2="11" y2="21" stroke="#00d4ff" strokeWidth="1.5"/>
              <line x1="1"  y1="11" x2="5"  y2="11" stroke="#00d4ff" strokeWidth="1.5"/>
              <line x1="17" y1="11" x2="21" y2="11" stroke="#00d4ff" strokeWidth="1.5"/>
            </svg>
            <span className="topbar-wordmark">Meridian<span>Engine</span></span>
          </div>
          <div className="topbar-divider"/>
          <span className="topbar-tag">Austin, TX · OSM Graph · v0.1</span>
          <div className="topbar-spacer"/>
          <div className="status-dot"/>
          <span className="status-label">backend online</span>
        </header>

        {/* ── LEFT PANEL ─────────────────────────────────────── */}
        <aside className="panel">
          <nav className="tabs">
            {[['route','Route'],['stats','Stats'],['about','About']].map(([id, label]) => (
              <button key={id} className={`tab ${tab === id ? 'active' : ''}`} onClick={() => setTab(id)}>
                {label}
              </button>
            ))}
          </nav>

          {/* ROUTE TAB */}
          {tab === 'route' && (
            <div className="panel-body">

              {/* Algorithm */}
              <div>
                <div className="section-label">Algorithm</div>
                <div className="algo-grid">
                  {ALGORITHMS.map(a => (
                    <button key={a.id} className={`algo-btn ${algorithm === a.id ? 'selected' : ''}`}
                      onClick={() => setAlgorithm(a.id)}>
                      <div className="algo-btn-header">
                        <span className="algo-name">{a.label}</span>
                        <div className="algo-check"/>
                      </div>
                      <div className="algo-desc">{a.desc}</div>
                    </button>
                  ))}
                </div>
              </div>

              {/* Waypoints */}
              <div>
                <div className="section-label">Waypoints</div>
                <div className="waypoint-grid">
                  <div className="waypoint-row">
                    <div className={`wp-dot ${startPoint ? 'start' : 'empty'}`}/>
                    <div>
                      <div className="wp-label">Origin</div>
                      <div className={`wp-coords ${startPoint ? '' : 'empty'}`}>
                        {fmtCoords(startPoint) ?? 'Click map to set'}
                      </div>
                    </div>
                  </div>
                  <div className="waypoint-row">
                    <div className={`wp-dot ${endPoint ? 'end' : 'empty'}`}/>
                    <div>
                      <div className="wp-label">Destination</div>
                      <div className={`wp-coords ${endPoint ? '' : 'empty'}`}>
                        {fmtCoords(endPoint) ?? 'Click map to set'}
                      </div>
                    </div>
                  </div>
                </div>
              </div>

              {/* Steps */}
              <div>
                <div className="section-label">Instructions</div>
                <div className="instruction-steps">
                  {[
                    'Select a routing algorithm above',
                    'Click the map to drop an origin pin',
                    'Click again to drop a destination',
                  ].map((text, i) => (
                    <div key={i} className="step-row">
                      <div className={`step-num ${step > i ? 'done' : ''}`}>{i + 1}</div>
                      <div className={`step-text ${step > i ? 'done' : ''}`}>{text}</div>
                    </div>
                  ))}
                </div>
              </div>

              {isLoading && (
                <div>
                  <div style={{ fontSize: 11, fontFamily: 'var(--font-mono)', color: 'var(--muted)', marginBottom: 4 }}>
                    Computing route…
                  </div>
                  <div className="loading-bar-wrap">
                    <div className="loading-bar"/>
                  </div>
                </div>
              )}

              <button className="btn-ghost" onClick={handleReset}>↺ Reset Route</button>
            </div>
          )}

          {/* STATS TAB */}
          {tab === 'stats' && (
            <div className="panel-body">
              <div>
                <div className="section-label">Last Route</div>
                <div className="stats-grid">
                  <div className={`stat-card ${stats ? 'highlight' : ''}`}>
                    <div className={`stat-value ${stats ? '' : 'empty'}`}>
                      {stats ? (stats.distance_meters / 1000).toFixed(2) : '—'}
                    </div>
                    <div className="stat-unit">km distance</div>
                  </div>
                  <div className={`stat-card ${stats ? 'highlight' : ''}`}>
                    <div className={`stat-value gold ${stats ? '' : 'empty'}`}>
                      {stats ? stats.duration_ms : '—'}
                    </div>
                    <div className="stat-unit">ms compute</div>
                  </div>
                  <div className={`stat-card ${stats ? 'highlight' : ''}`} style={{gridColumn:'1/-1'}}>
                    <div className={`stat-value red ${stats ? '' : 'empty'}`}>
                      {stats ? stats.nodes_visited.toLocaleString() : '—'}
                    </div>
                    <div className="stat-unit">nodes visited</div>
                  </div>
                </div>
              </div>

              <div>
                <div className="section-label">Graph Metadata</div>
                <div className="metrics-list">
                  {[
                    ['Region',       'Austin, TX'],
                    ['Source',       'OpenStreetMap PBF'],
                    ['Graph type',   'Directed adjacency list'],
                    ['Index',        'k-d tree (nanoflann)'],
                    ['Snap latency', '< 1 ms'],
                    ['RAM usage',    '~25–30 MB'],
                  ].map(([k, v]) => (
                    <div className="metric-row" key={k}>
                      <span className="metric-key">{k}</span>
                      <span className="metric-val">{v}</span>
                    </div>
                  ))}
                </div>
              </div>
            </div>
          )}

          {/* ABOUT TAB */}
          {tab === 'about' && (
            <div className="panel-body">
              <div className="about-section">
                <div className="about-heading">
                  Applied graph theory<br/>on <span>real-world data</span>
                </div>
                <div className="about-body">
                  Meridian Engine parses a real OpenStreetMap PBF extract of Austin into a
                  compact in-memory adjacency list (~25 MB). A C++ server processes routing
                  requests in milliseconds and streams algorithm execution steps over WebSocket
                  for live visualization.
                </div>

                <div>
                  <div className="section-label" style={{marginBottom:8}}>Tech stack</div>
                  <div className="tech-pills">
                    {['C++17','libosmium','Crow HTTP','nanoflann','CMake'].map(t => (
                      <span key={t} className="pill accent">{t}</span>
                    ))}
                    {['React','TypeScript','Vite','Leaflet.js','Fly.io','Vercel'].map(t => (
                      <span key={t} className="pill">{t}</span>
                    ))}
                  </div>
                </div>

                <div>
                  <div className="section-label" style={{marginBottom:8}}>Architecture</div>
                  <div className="metrics-list">
                    {[
                      ['Route API',  'REST  POST /route'],
                      ['Viz stream', 'WebSocket /visualize'],
                      ['Startup',    'PBF parse → binary cache'],
                      ['Algo order', 'Dijkstra → A* → BiDir'],
                    ].map(([k, v]) => (
                      <div className="metric-row" key={k}>
                        <span className="metric-key">{k}</span>
                        <span className="metric-val" style={{fontFamily:'var(--font-mono)',fontSize:10}}>{v}</span>
                      </div>
                    ))}
                  </div>
                </div>

                <a href="https://github.com" target="_blank" rel="noopener noreferrer"
                  style={{textDecoration:'none'}}>
                  <button className="btn-ghost" style={{marginTop:4}}>
                    ↗ View source on GitHub
                  </button>
                </a>
              </div>
            </div>
          )}
        </aside>

        {/* ── MAP ────────────────────────────────────────────── */}
        <main className="map-area">
          <MapContainer
            center={AUSTIN_CENTER}
            zoom={12}
            minZoom={10}
            maxBounds={AUSTIN_BOUNDS}
            maxBoundsViscosity={1.0}
            zoomControl={false}
            style={{ height: '100%', width: '100%' }}
          >
            <TileLayer
              attribution='&copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a>'
              url="https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png"
            />
            <MapClickHandler onMapClick={handleMapClick}/>
            {startPoint && <Marker position={startPoint} icon={START_ICON}/>}
            {endPoint   && <Marker position={endPoint}   icon={END_ICON}/>}
            {routeCoords.length > 0 && (
              <Polyline positions={routeCoords} color="#00d4ff" weight={4} opacity={0.9}/>
            )}
          </MapContainer>

          {/* Hint banner */}
          <div className={`map-hint ${step === 2 ? 'hidden' : ''}`}>
            {step === 0 && '↖ Select an algorithm, then click the map to begin'}
            {step === 1 && 'Origin set — click the map to drop a destination'}
          </div>

          {/* HUD bottom-right */}
          <div className="map-hud">
            {stats && (
              <div className="hud-badge">
                <strong>{(stats.distance_meters / 1000).toFixed(2)} km</strong>
                {' · '}
                {ALGORITHMS.find(a => a.id === algorithm)?.label}
                {stats.duration_ms !== '—' && ` · ${stats.duration_ms} ms`}
              </div>
            )}
            <div className="hud-badge">Austin, TX · OSM</div>
          </div>

          {/* Error toast */}
          {error && <div className="toast">{error}</div>}
        </main>

      </div>
    </>
  )
}