# Dashboard Subsystem (DBS)

**Smart Tank Monitoring System - Dashboard Subsystem**

Front-end for remote operators to visualise data and interact with the system.

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

## How to run the Dashboard

1. **Start the CUS first:**
   ```bash
   cd ../CUS
   uv run cus_main.py
   ```

2. **Serve the dashboard:**
   ```bash
   cd ../DBS
   python -m http.server 8080
   ```

3. **Open in browser:**
   - Go to http://localhost:8080
   - Dashboard will connect to CUS automatically

## The dashboard will:

- Auto-connect to CUS at http://localhost:5000
- Update every 2 seconds
- Display real-time water levels
- Show valve status
- Allow mode switching and manual control

## Customization

Edit `js/config.js` if CUS runs on a different port:
```javascript
API_BASE_URL: 'http://localhost:5000'  // Change here
```

## Dashboard Features

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

## File Overview

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