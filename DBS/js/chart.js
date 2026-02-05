/**
 * Chart Module
 * Handles the water level chart using Chart.js
 */

class LevelChart {
    constructor(canvasId) {
        this.ctx = document.getElementById(canvasId).getContext('2d');
        this.l1Threshold = 30;
        this.l2Threshold = 50;

        this.chart = new Chart(this.ctx, {
            type: 'line',
            data: {
                labels: [],
                datasets: [{
                    label: 'Water Level (cm)',
                    data: [],
                    borderColor: APP_CONFIG.CHART_COLORS.line,
                    backgroundColor: APP_CONFIG.CHART_COLORS.fill,
                    borderWidth: 2,
                    fill: true,
                    tension: 0.4,
                    pointRadius: 0,
                    pointHoverRadius: 6,
                    pointHoverBackgroundColor: APP_CONFIG.CHART_COLORS.line,
                    pointHoverBorderColor: '#fff',
                    pointHoverBorderWidth: 2
                }]
            },
            options: {
                responsive: true,
                maintainAspectRatio: false,
                interaction: {
                    intersect: false,
                    mode: 'index'
                },
                plugins: {
                    legend: {
                        display: false
                    },
                    tooltip: {
                        backgroundColor: 'rgba(30, 41, 59, 0.95)',
                        titleColor: '#f1f5f9',
                        bodyColor: '#cbd5e1',
                        borderColor: '#334155',
                        borderWidth: 1,
                        padding: 12,
                        displayColors: false,
                        callbacks: {
                            label: (context) => {
                                return `Level: ${context.parsed.y.toFixed(1)} cm`;
                            }
                        }
                    },
                    annotation: {
                        annotations: {
                            l1Line: {
                                type: 'line',
                                yMin: this.l1Threshold,
                                yMax: this.l1Threshold,
                                borderColor: APP_CONFIG.CHART_COLORS.l1,
                                borderWidth: 2,
                                borderDash: [5, 5],
                                label: {
                                    display: true,
                                    content: 'L1',
                                    position: 'end',
                                    backgroundColor: APP_CONFIG.CHART_COLORS.l1,
                                    color: '#111827'
                                }
                            },
                            l2Line: {
                                type: 'line',
                                yMin: this.l2Threshold,
                                yMax: this.l2Threshold,
                                borderColor: APP_CONFIG.CHART_COLORS.l2,
                                borderWidth: 2,
                                borderDash: [5, 5],
                                label: {
                                    display: true,
                                    content: 'L2',
                                    position: 'end',
                                    backgroundColor: APP_CONFIG.CHART_COLORS.l2,
                                    color: '#fff'
                                }
                            }
                        }
                    }
                },
                scales: {
                    x: {
                        display: true,
                        grid: {
                            color: APP_CONFIG.CHART_COLORS.grid,
                            drawBorder: false
                        },
                        ticks: {
                            color: APP_CONFIG.CHART_COLORS.text,
                            maxTicksLimit: 8,
                            maxRotation: 0
                        }
                    },
                    y: {
                        display: true,
                        beginAtZero: true,
                        grid: {
                            color: APP_CONFIG.CHART_COLORS.grid,
                            drawBorder: false
                        },
                        ticks: {
                            color: APP_CONFIG.CHART_COLORS.text,
                            callback: (value) => value + ' cm'
                        },
                        min: 0,
                        max: 100
                    }
                }
            }
        });
    }

    /**
     * Update chart with new data
     */
    updateData(measurements) {
        const maxPoints = APP_CONFIG.CHART_MAX_POINTS;

        // Limit number of points
        const displayData = measurements.slice(-maxPoints);

        // Format data
        const labels = displayData.map((m, index) => {
            if (m.timestamp) {
                const date = new Date(m.timestamp * 1000);
                return date.toLocaleTimeString('it-IT', {
                    hour: '2-digit',
                    minute: '2-digit',
                    second: '2-digit'
                });
            }
            return `#${index + 1}`;
        });

        const data = displayData.map(m => m.level);

        // Update chart
        this.chart.data.labels = labels;
        this.chart.data.datasets[0].data = data;
        this.chart.update('none');
    }

    /**
     * Update threshold lines
     */
    updateThresholds(l1, l2) {
        this.l1Threshold = l1;
        this.l2Threshold = l2;

        if (this.chart.options.plugins.annotation) {
            this.chart.options.plugins.annotation.annotations.l1Line.yMin = l1;
            this.chart.options.plugins.annotation.annotations.l1Line.yMax = l1;
            this.chart.options.plugins.annotation.annotations.l2Line.yMin = l2;
            this.chart.options.plugins.annotation.annotations.l2Line.yMax = l2;
            this.chart.update('none');
        }
    }
}

// Create global chart instance
// Register the annotation plugin
if (window['chartjs-plugin-annotation']) {
    Chart.register(window['chartjs-plugin-annotation']);
}

// Create global chart instance
window.levelChart = null;
