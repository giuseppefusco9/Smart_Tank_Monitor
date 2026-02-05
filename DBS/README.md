# Dashboard Subsystem (DBS)

**Smart Tank Monitoring System - Dashboard Subsystem**

The DBS provides a web-based interface for real-time monitoring and control of the Smart Tank Monitoring System.

## Features

- **Real-time Monitoring**: Displays current water level, valve opening percentage, and system mode.
- **Historical Data**: Visualizes water level history using a dynamic chart.
- **Remote Control**: Allows switching between AUTOMATIC and MANUAL modes and controlling the valve opening when in MANUAL mode.
- **Status Indicators**: Show connection status with the Control Unit Subsystem (CUS).

## Technologies

- **Frontend**: HTML5, Vanilla CSS, JavaScript.
- **Charts**: Chart.js for data visualization.
- **Communication**: REST API calls to the CUS.

## Project Structure

```
DBS/
├── index.html          # Main dashboard page
├── css/
│   └── styles.css      # Dashboard styling
└── js/
    ├── config.js      # API configuration
    ├── api.js         # API communication logic
    ├── chart.js       # Chart.js initialization and updates
    ├── controls.js    # UI interaction and control logic
    └── app.js         # Main application coordinator
```

## Website structure
![alt text](<Screenshot 2026-02-04 171458.png>)
![alt text](<Screenshot 2026-02-05 112743.png>)