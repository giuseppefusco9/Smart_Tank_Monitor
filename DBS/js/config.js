/**
 * Configuration for Dashboard Subsystem (DBS)
 */

const CONFIG = {
    // CUS API Configuration
    API_BASE_URL: 'http://localhost:5000',
    
    // Update intervals (in milliseconds)
    UPDATE_INTERVAL: 2000,  // 2 seconds
    CHART_MAX_POINTS: 50,   // Maximum number of points to show on chart
    
    // API Endpoints
    ENDPOINTS: {
        STATUS: '/api/status',
        RAINWATER: '/api/rainwater',
        MODE: '/api/mode',
        VALVE: '/api/valve',
        CONFIG: '/api/config',
        HEALTH: '/health'
    },
    
    // Toast notification duration
    TOAST_DURATION: 3000,  // 3 seconds
    
    // Chart colors
    CHART_COLORS: {
        line: '#60a5fa',
        fill: 'rgba(96, 165, 250, 0.1)',
        grid: '#334155',
        text: '#cbd5e1',
        l1: '#fbbf24',
        l2: '#f87171'
    }
};

// Make config globally available
window.APP_CONFIG = CONFIG;
