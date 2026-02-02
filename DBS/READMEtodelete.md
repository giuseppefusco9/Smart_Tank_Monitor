# Dashboard Subsystem (DBS)

**Smart Tank Monitoring System - Dashboard Subsystem**

The DBS is the web-based frontend for the Tank Monitoring System, providing real-time visualization and control capabilities.

## Features

- 📊 **Real-time Water Level Graph** - Live chart with L1/L2 threshold lines
- 🎛️ **Valve Control** - Visual indicator and manual control slider
- 🔄 **Mode Switching** - Toggle between AUTOMATIC and MANUAL modes
- 📈 **System Status** - Current water level, valve opening, and system state
- 🎨 **Modern UI** - Dark theme with smooth animations and premium design
- 📱 **Responsive Design** - Works on desktop, tablet, and mobile
- 🔔 **Toast Notifications** - User-friendly feedback for all actions
- ⚡ **Auto-refresh** - Updates every 2 seconds

## Project Structure

```
DBS/
├── index.html              # Main HTML page
├── css/
│   └── styles.css         # Complete styling (dark theme, animations)
├── js/
│   ├── config.js          # Configuration (API URL, update interval)
│   ├── api.js             # API communication with CUS
│   ├── chart.js           # Chart.js integration
│   ├── controls.js        # User interaction handlers
│   └── app.js             # Main application logic
└── README.md              # This file
```

## Installation

### 1. No Build Required!

This is a pure HTML/CSS/JavaScript application with no build step needed.

### 2. Serve the Files

You can serve the dashboard using any web server. Here are some options:

**Option A: Python HTTP Server (Recommended)**
```bash
cd DBS
python -m http.server 8080
```
Then open http://localhost:8080

**Option B: Node.js HTTP Server**
```bash
npm install -g http-server
cd DBS
http-server -p 8080
```

**Option C: VS Code Live Server**
- Install "Live Server" extension
- Right-click `index.html`
- Select "Open with Live Server"

## Configuration

### API Endpoint

Edit `js/config.js` to change the CUS API URL:

```javascript
const CONFIG = {
    API_BASE_URL: 'http://localhost:5000',  // Change if CUS runs on different port
    UPDATE_INTERVAL: 2000,  // Update frequency in milliseconds
    // ...
};
```

## Usage

### 1. Start the CUS

Make sure the Control Unit Subsystem is running:

```bash
cd ../CUS
python cus_main.py
```

The CUS should be accessible at http://localhost:5000

### 2. Open the Dashboard

Open http://localhost:8080 (or your chosen port) in a web browser.

### 3. Dashboard Features

#### Connection Status
- **Green dot**: Connected to CUS
- **Gray dot**: Connecting...
- **Red dot**: Connection failed

#### System Status Card
- Displays current water level
- Shows valve opening percentage with visual bar
- Displays L1 and L2 threshold values
- Mode badge shows current system state

#### Water Level Chart
- Real-time graph of water level history
- Shows last 50 measurements
- L1 threshold line (dashed yellow)
- L2 threshold line (dashed red)
- Hover to see exact values

#### Control Panel

**Mode Switching:**
1. Click "Automatic" for automatic valve control
2. Click "Manual" for manual control

**Manual Valve Control:**
1. Switch to MANUAL mode
2. Use slider to adjust valve opening (0-100%)
3. Click "Apply Valve Setting" to send command

#### System Information
- Last update timestamp
- CUS API endpoint
- Update interval
- Auto-refresh status

## Communication Protocol

### HTTP API Calls

The dashboard communicates with the CUS via HTTP REST API:

**GET /api/status** - Every 2 seconds
```javascript
{
  mode: "AUTOMATIC",
  valve_opening: 50,
  latest_level: 35.5,
  ...
}
```

**GET /api/rainwater** - Every 2 seconds
```javascript
{
  measurements: [{level: 35.5, timestamp: 1234567890}, ...],
  count: 100
}
```

**POST /api/mode** - On mode button click
```javascript
{mode: "MANUAL"}
```

**POST /api/valve** - On apply button click
```javascript
{opening: 75}
```

## Browser Compatibility

Tested and working on:
- ✅ Chrome/Edge 90+
- ✅ Firefox 88+
- ✅ Safari 14+

Requires modern browser with ES6 support and Chart.js compatibility.

## Customization

### Change Colors

Edit `css/styles.css` CSS variables:

```css
:root {
    --primary: #3b82f6;  /* Change primary color */
    --success: #10b981;  /* Change success color */
    /* ... */
}
```

### Change Update Frequency

Edit `js/config.js`:

```javascript
UPDATE_INTERVAL: 2000,  // Change to desired milliseconds
```

### Change Chart Settings

Edit `js/config.js`:

```javascript
CHART_MAX_POINTS: 50,  // Number of points to display
```

## Troubleshooting

### Dashboard Shows "Connecting..." or "Disconnected"

**Problem:** Cannot connect to CUS
**Solutions:**
1. Verify CUS is running: `python cus_main.py`
2. Check if CUS is accessible: `curl http://localhost:5000/health`
3. Verify API_BASE_URL in `js/config.js` matches CUS address
4. Check browser console for CORS errors
5. Ensure CUS has CORS enabled (already configured in `http_server.py`)

### Chart Not Displaying

**Problem:** Chart shows but no data
**Solutions:**
1. Verify TMS is sending data to CUS
2. Check browser console for errors
3. Verify Chart.js CDN is accessible (requires internet)

### Mode Switching Doesn't Work

**Problem:** Clicking mode buttons has no effect
**Solutions:**
1. Check if CUS is in UNCONNECTED state
2. Verify browser console for API errors
3. Check CUS logs for mode change requests

### CORS Errors

**Problem:** Browser blocks API requests
**Solutions:**
1. CUS already has CORS enabled via Flask-CORS
2. If using custom setup, add CORS headers to CUS
3. Serve DBS from same origin as CUS (not recommended for development)

## Development

### File Overview

**index.html**
- Main structure
- Card-based layout
- SVG icons
- Chart canvas

**css/styles.css**
- CSS variables for theming
- Responsive grid layout
- Smooth animations
- Dark mode design

**js/config.js**
- Configuration constants
- API endpoints
- Chart colors

**js/api.js**
- HTTP request wrapper
- All CUS API methods
- Error handling

**js/chart.js**
- Chart.js initialization
- Data updates
- Threshold lines

**js/controls.js**
- Mode switching
- Valve slider
- Toast notifications

**js/app.js**
- Main coordinator
- Periodic updates
- Dashboard initialization

## For Assignment Submission

This DBS implementation fulfills the assignment requirements:
- ✅ Runs on PC (web browser)
- ✅ Uses HTTP to communicate with CUS
- ✅ Visualizes rainwater level data (graph with N measurements)
- ✅ Shows current valve opening percentage
- ✅ Shows system state (AUTOMATIC/MANUAL/UNCONNECTED)
- ✅ Provides mode switching button
- ✅ Provides manual valve control widget
- ✅ No specific technology constraints - uses vanilla HTML/CSS/JS
- ✅ Modern, professional UI design

## Screenshot

The dashboard features:
- Dark theme with gradient accents
- Card-based layout
- Real-time updating chart
- Interactive controls
- Responsive design

## License

Smart Tank Monitoring System - ISI LT a.y. 2025/2026 Assignment #03
