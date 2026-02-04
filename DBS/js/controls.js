/**
 * Controls Module
 * Handles user interactions (mode switching, valve control)
 */

class Controls {
    constructor() {
        this.currentMode = 'AUTOMATIC';
        this.isUserDraggingSlider = false;
        this.setupEventListeners();
    }

    setupEventListeners() {
        // Mode buttons
        document.getElementById('autoBtn').addEventListener('click', () => {
            this.switchMode('AUTOMATIC');
        });

        document.getElementById('manualBtn').addEventListener('click', () => {
            this.switchMode('MANUAL');
        });

        // Valve slider
        const slider = document.getElementById('valveSlider');
        const sliderValue = document.getElementById('sliderValue');

        slider.addEventListener('input', (e) => {
            sliderValue.textContent = e.target.value + '%';
        });

        slider.addEventListener('mousedown', () => {
            this.isUserDraggingSlider = true;
        });

        slider.addEventListener('mouseup', () => {
            this.isUserDraggingSlider = false;
        });

        slider.addEventListener('touchstart', () => {
            this.isUserDraggingSlider = true;
        });

        slider.addEventListener('touchend', () => {
            this.isUserDraggingSlider = false;
        });

        // Global release listeners to ensure state is reset even if mouse leaves slider
        const releaseSlider = () => {
            if (this.isUserDraggingSlider) {
                this.isUserDraggingSlider = false;
                console.log('Slider released globally');
            }
        };

        window.addEventListener('mouseup', releaseSlider);
        window.addEventListener('touchend', releaseSlider);

        // Apply valve button
        document.getElementById('applyValveBtn').addEventListener('click', () => {
            this.applyValveSetting();
        });
    }

    /**
     * Switch system mode
     */
    async switchMode(mode) {
        try {
            const result = await api.setMode(mode);

            if (result.success) {
                this.currentMode = mode;
                this.updateModeUI(mode);
                showToast(`Switched to ${mode} mode`, 'success');
            } else {
                showToast(result.message || 'Failed to switch mode', 'error');
            }
        } catch (error) {
            showToast('Failed to communicate with CUS', 'error');
            console.error('Mode switch error:', error);
        }
    }

    /**
     * Apply valve setting (MANUAL mode only)
     */
    async applyValveSetting() {
        if (this.currentMode !== 'MANUAL') {
            showToast('Valve control only available in MANUAL mode', 'error');
            return;
        }

        const opening = parseInt(document.getElementById('valveSlider').value);

        try {
            const result = await api.setValveOpening(opening);

            if (result.success) {
                showToast(`Valve set to ${opening}%`, 'success');
            } else {
                showToast(result.message || 'Failed to set valve', 'error');
            }
        } catch (error) {
            showToast('Failed to communicate with CUS', 'error');
            console.error('Valve control error:', error);
        }
    }

    /**
     * Update UI based on current mode
     */
    updateModeUI(mode) {
        const autoBtn = document.getElementById('autoBtn');
        const manualBtn = document.getElementById('manualBtn');
        const slider = document.getElementById('valveSlider');
        const applyBtn = document.getElementById('applyValveBtn');
        const modeBadge = document.getElementById('modeBadge');

        // Update buttons
        autoBtn.classList.toggle('active', mode === 'AUTOMATIC');
        manualBtn.classList.toggle('active', mode === 'MANUAL');

        // Update badge
        modeBadge.textContent = mode;
        modeBadge.className = `mode-badge ${mode}`;

        // Enable/disable manual controls
        const isManual = mode === 'MANUAL';
        slider.disabled = !isManual;
        applyBtn.disabled = !isManual;
    }

    /**
     * Update mode from external source (e.g., WCS button press)
     */
    setMode(mode) {
        this.currentMode = mode;
        this.updateModeUI(mode);
    }
}

/**
 * Show toast notification
 */
function showToast(message, type = 'info') {
    const toast = document.getElementById('toast');
    const toastMessage = toast.querySelector('.toast-message');

    // Set message
    toastMessage.textContent = message;

    // Set type
    toast.className = `toast ${type}`;

    // Show toast
    toast.classList.add('show');

    // Hide after duration
    setTimeout(() => {
        toast.classList.remove('show');
    }, APP_CONFIG.TOAST_DURATION);
}

// Create global controls instance
window.controls = null;
