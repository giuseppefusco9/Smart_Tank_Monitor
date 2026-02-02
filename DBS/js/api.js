/**
 * API Communication Module
 * Handles all HTTP requests to the CUS
 */

class API {
    constructor(baseUrl) {
        this.baseUrl = baseUrl;
    }

    /**
     * Generic fetch wrapper with error handling
     */
    async request(endpoint, options = {}) {
        try {
            const url = `${this.baseUrl}${endpoint}`;
            const response = await fetch(url, {
                ...options,
                headers: {
                    'Content-Type': 'application/json',
                    ...options.headers
                }
            });

            if (!response.ok) {
                throw new Error(`HTTP ${response.status}: ${response.statusText}`);
            }

            return await response.json();
        } catch (error) {
            console.error(`API request failed for ${endpoint}:`, error);
            throw error;
        }
    }

    /**
     * GET /api/status
     * Get current system status
     */
    async getStatus() {
        return await this.request(APP_CONFIG.ENDPOINTS.STATUS);
    }

    /**
     * GET /api/rainwater
     * Get rainwater level history
     */
    async getRainwaterLevels() {
        return await this.request(APP_CONFIG.ENDPOINTS.RAINWATER);
    }

    /**
     * POST /api/mode
     * Switch system mode (AUTOMATIC or MANUAL)
     */
    async setMode(mode) {
        return await this.request(APP_CONFIG.ENDPOINTS.MODE, {
            method: 'POST',
            body: JSON.stringify({ mode })
        });
    }

    /**
     * POST /api/valve
     * Set valve opening (MANUAL mode only)
     */
    async setValveOpening(opening) {
        return await this.request(APP_CONFIG.ENDPOINTS.VALVE, {
            method: 'POST',
            body: JSON.stringify({ opening })
        });
    }

    /**
     * GET /api/config
     * Get system configuration
     */
    async getConfig() {
        return await this.request(APP_CONFIG.ENDPOINTS.CONFIG);
    }

    /**
     * GET /health
     * Health check
     */
    async checkHealth() {
        return await this.request(APP_CONFIG.ENDPOINTS.HEALTH);
    }
}

// Create global API instance
window.api = new API(APP_CONFIG.API_BASE_URL);
