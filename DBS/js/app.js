/**
 * Main Application Module
 * Coordinates all dashboard functionality
 */

class DashboardApp {
    constructor() {
        this.updateInterval = null;
        this.isConnected = false;
        this.config = {
            l1: 30,
            l2: 50
        };
    }

    /**
     * Initialize the dashboard
     */
    async init() {
        console.log('Initializing Dashboard Subsystem...');

        // Initialize chart
        window.levelChart = new LevelChart('levelChart');

        // Initialize controls
        window.controls = new Controls();

        // Load system configuration
        await this.loadConfig();

        // Start periodic updates
        this.startPeriodicUpdates();

        // Initial data load
        await this.updateDashboard();

        console.log('Dashboard initialized successfully');
    }

    /**
     * Load system configuration from CUS
     */
    async loadConfig() {
        try {
            const config = await api.getConfig();
            this.config.l1 = config.l1_threshold;
            this.config.l2 = config.l2_threshold;

            // Update UI
            document.getElementById('l1Threshold').textContent = `${this.config.l1} cm`;
            document.getElementById('l2Threshold').textContent = `${this.config.l2} cm`;

            // Update chart thresholds
            levelChart.updateThresholds(this.config.l1, this.config.l2);

            console.log('Configuration loaded:', config);
        } catch (error) {
            console.error('Failed to load configuration:', error);
            // Use defaults
            levelChart.updateThresholds(this.config.l1, this.config.l2);
        }
    }

    /**
     * Start periodic dashboard updates
     */
    startPeriodicUpdates() {
        // Clear existing interval if any
        if (this.updateInterval) {
            clearInterval(this.updateInterval);
        }

        // Update immediately
        this.updateDashboard();

        // Set up periodic updates
        this.updateInterval = setInterval(() => {
            this.updateDashboard();
        }, APP_CONFIG.UPDATE_INTERVAL);

        console.log(`Started periodic updates (${APP_CONFIG.UPDATE_INTERVAL}ms interval)`);
    }

    /**
     * Stop periodic updates
     */
    stopPeriodicUpdates() {
        if (this.updateInterval) {
            clearInterval(this.updateInterval);
            this.updateInterval = null;
            console.log('Stopped periodic updates');
        }
    }

    /**
     * Update all dashboard data
     */
    async updateDashboard() {
        try {
            // Fetch status and rainwater data in parallel
            const [status, rainwaterData] = await Promise.all([
                api.getStatus(),
                api.getRainwaterLevels()
            ]);

            // Update connection status
            this.updateConnectionStatus(true);

            // Update system status
            this.updateSystemStatus(status);

            // Update chart
            this.updateChart(rainwaterData);

            // Update measurement count
            document.getElementById('measurementCount').textContent = rainwaterData.count;

        } catch (error) {
            console.error('Dashboard update failed:', error);
            this.updateConnectionStatus(false);
        }
    }

    /**
     * Update connection status indicator
     */
    updateConnectionStatus(connected) {
        const statusEl = document.getElementById('connectionStatus');
        const statusText = statusEl.querySelector('.status-text');

        if (connected && !this.isConnected) {
            // Just connected
            this.isConnected = true;
            statusEl.classList.remove('error');
            statusEl.classList.add('connected');
            statusText.textContent = 'Connected';
        } else if (!connected && this.isConnected) {
            // Just disconnected
            this.isConnected = false;
            statusEl.classList.remove('connected');
            statusEl.classList.add('error');
            statusText.textContent = 'Disconnected';
            showToast('Lost connection to CUS', 'error');
        } else if (!connected && !this.isConnected) {
            // Still disconnected
            statusText.textContent = 'Connecting...';
        }
    }

    /**
     * Update system status display
     */
    updateSystemStatus(status) {
        // Update mode
        const mode = status.mode || 'NOT AVAILABLE';
        controls.setMode(mode);

        // Update current water level
        const levelNumber = document.querySelector('#currentLevel .value-number');
        if (status.latest_level !== null && status.latest_level !== undefined) {
            levelNumber.textContent = status.latest_level.toFixed(1);

            // Add color based on threshold
            if (status.latest_level >= this.config.l2) {
                levelNumber.style.background = 'linear-gradient(135deg, #f87171, #ef4444)';
            } else if (status.latest_level >= this.config.l1) {
                levelNumber.style.background = 'linear-gradient(135deg, #fbbf24, #f59e0b)';
            } else {
                levelNumber.style.background = 'linear-gradient(135deg, #60a5fa, #22d3ee)';
            }
            levelNumber.style.webkitBackgroundClip = 'text';
            levelNumber.style.webkitTextFillColor = 'transparent';
        } else {
            levelNumber.textContent = '--';
        }

        // Update valve opening
        const valveNumber = document.querySelector('#valveOpening .value-number');
        const valveFill = document.getElementById('valveFill');
        const valveSlider = document.getElementById('valveSlider');

        valveNumber.textContent = status.valve_opening;
        valveFill.style.width = status.valve_opening + '%';

        // Update slider if not in manual mode
        if (status.mode !== 'MANUAL') {
            valveSlider.value = status.valve_opening;
            document.getElementById('sliderValue').textContent = status.valve_opening + '%';
        }

        // Update last update time
        if (status.last_update) {
            const date = new Date(status.last_update * 1000);
            document.getElementById('lastUpdate').textContent = date.toLocaleString('it-IT');
        }
    }

    /**
     * Update chart with rainwater data
     */
    updateChart(rainwaterData) {
        if (rainwaterData.measurements && rainwaterData.measurements.length > 0) {
            levelChart.updateData(rainwaterData.measurements);
        }
    }
}

/**
 * Initialize app when DOM is ready
 */
document.addEventListener('DOMContentLoaded', () => {
    window.app = new DashboardApp();
    app.init();
});

/**
 * Handle page visibility changes
 */
document.addEventListener('visibilitychange', () => {
    if (document.hidden) {
        // Page hidden - stop updates to save resources
        console.log('Page hidden - pausing updates');
        app.stopPeriodicUpdates();
    } else {
        // Page visible again - resume updates
        console.log('Page visible - resuming updates');
        app.startPeriodicUpdates();
    }
});

/**
 * Handle window unload
 */
window.addEventListener('beforeunload', () => {
    if (app) {
        app.stopPeriodicUpdates();
    }
});
