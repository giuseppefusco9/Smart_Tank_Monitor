"""
HTTP Server for Control Unit Subsystem (CUS)
Provides REST API for Dashboard Subsystem (DBS)
"""

import logging
from flask import Flask, jsonify, request
from flask_cors import CORS
from typing import Callable, Optional
from . import config


logger = logging.getLogger(__name__)


class HTTPServer:
    """HTTP REST API server for dashboard communication"""
    
    def __init__(self, get_state_callback: Callable,
                 set_mode_callback: Callable[[str], bool],
                 set_valve_callback: Callable[[int], bool]):
        """
        Initialize HTTP server
        
        Args:
            get_state_callback: Function that returns current system state dict
            set_mode_callback: Function(mode) to set system mode, returns success bool
            set_valve_callback: Function(opening) to set valve opening, returns success bool
        """
        self.get_state = get_state_callback
        self.set_mode = set_mode_callback
        self.set_valve = set_valve_callback
        
        # Create Flask app
        self.app = Flask(__name__)
        CORS(self.app)  # Enable CORS for web dashboard
        
        # Disable Flask's default logging to use our logger
        logging.getLogger('werkzeug').setLevel(logging.WARNING)
        
        # Register routes
        self._register_routes()
    
    def _register_routes(self):
        """Register all API endpoints"""
        
        @self.app.route('/api/status', methods=['GET'])
        def get_status():
            """
            Get current system status
            Returns: {
                mode: "AUTOMATIC"|"MANUAL"|"UNCONNECTED",
                valve_opening: 0-100,
                latest_level: float,
                last_update: timestamp
            }
            """
            try:
                state = self.get_state()
                
                response = {
                    'mode': state['mode'],
                    'valve_opening': state['valve_opening'],
                    'latest_level': state.get('latest_level'),
                    'last_update': state.get('last_tms_message_time'),
                    'l1_threshold': config.L1_THRESHOLD,
                    'l2_threshold': config.L2_THRESHOLD
                }
                
                return jsonify(response), 200
                
            except Exception as e:
                logger.error(f"Error in /api/status: {e}", exc_info=True)
                return jsonify({'error': 'Internal server error'}), 500
        
        @self.app.route('/api/rainwater', methods=['GET'])
        def get_rainwater():
            """
            Get rainwater level history
            Returns: {
                measurements: [{level: float, timestamp: float}, ...],
                count: int
            }
            """
            try:
                state = self.get_state()
                measurements = state.get('rainwater_levels', [])
                
                response = {
                    'measurements': measurements,
                    'count': len(measurements)
                }
                
                return jsonify(response), 200
                
            except Exception as e:
                logger.error(f"Error in /api/rainwater: {e}", exc_info=True)
                return jsonify({'error': 'Internal server error'}), 500
        
        @self.app.route('/api/mode', methods=['POST'])
        def set_mode():
            """
            Switch system mode
            Request body: {"mode": "AUTOMATIC"|"MANUAL"}
            Returns: {success: bool, message: string}
            """
            try:
                data = request.get_json()
                
                # DEBUG: Print HTTP API mode change request
                print(f"\n{'='*60}")
                print(f"DEBUG [CUS-HTTP]: Mode change request from DBS")
                print(f"  Request Data: {data}")
                print(f"{'='*60}\n")
                
                if not data or 'mode' not in data:
                    return jsonify({'success': False, 'message': 'Missing mode parameter'}), 400
                
                mode = data['mode']
                
                if mode not in [config.MODE_AUTOMATIC, config.MODE_MANUAL]:
                    return jsonify({'success': False, 'message': f'Invalid mode: {mode}'}), 400
                
                success = self.set_mode(mode)
                
                if success:
                    print(f"DEBUG [CUS-HTTP]: Mode successfully changed to {mode}")
                    logger.info(f"Mode changed to {mode} via API")
                    return jsonify({'success': True, 'message': f'Mode set to {mode}'}), 200
                else:
                    return jsonify({'success': False, 'message': 'Failed to set mode (may be in UNCONNECTED state)'}), 400
                
            except Exception as e:
                logger.error(f"Error in /api/mode: {e}", exc_info=True)
                return jsonify({'error': 'Internal server error'}), 500
        
        @self.app.route('/api/valve', methods=['POST'])
        def set_valve():
            """
            Set valve opening (MANUAL mode only)
            Request body: {"opening": 0-100}
            Returns: {success: bool, message: string}
            """
            try:
                data = request.get_json()
                
                # DEBUG: Print HTTP API valve control request
                print(f"\n{'='*60}")
                print(f"DEBUG [CUS-HTTP]: Valve control request from DBS")
                print(f"  Request Data: {data}")
                print(f"{'='*60}\n")
                
                if not data or 'opening' not in data:
                    return jsonify({'success': False, 'message': 'Missing opening parameter'}), 400
                
                opening = int(data['opening'])
                
                if not (config.VALVE_MIN <= opening <= config.VALVE_MAX):
                    return jsonify({'success': False, 'message': f'Invalid opening value: {opening}'}), 400
                
                # Check if in MANUAL mode
                state = self.get_state()
                if state['mode'] != config.MODE_MANUAL:
                    return jsonify({'success': False, 'message': 'Can only set valve in MANUAL mode'}), 400
                
                success = self.set_valve(opening)
                
                if success:
                    print(f"DEBUG [CUS-HTTP]: Valve successfully set to {opening}%")
                    logger.info(f"Valve opening set to {opening}% via API")
                    return jsonify({'success': True, 'message': f'Valve set to {opening}%'}), 200
                else:
                    return jsonify({'success': False, 'message': 'Failed to set valve'}), 400
                
            except Exception as e:
                logger.error(f"Error in /api/valve: {e}", exc_info=True)
                return jsonify({'error': 'Internal server error'}), 500
        
        @self.app.route('/api/config', methods=['GET'])
        def get_config():
            """
            Get system configuration parameters
            Returns: {l1, l2, t1, t2, n}
            """
            try:
                response = {
                    'l1_threshold': config.L1_THRESHOLD,
                    'l2_threshold': config.L2_THRESHOLD,
                    't1_time': config.T1_TIME,
                    't2_timeout': config.T2_TIMEOUT,
                    'n_measurements': config.N_MEASUREMENTS
                }
                
                return jsonify(response), 200
                
            except Exception as e:
                logger.error(f"Error in /api/config: {e}", exc_info=True)
                return jsonify({'error': 'Internal server error'}), 500
        
        @self.app.route('/health', methods=['GET'])
        def health_check():
            """Health check endpoint"""
            return jsonify({'status': 'healthy', 'service': 'CUS'}), 200
    
    def run(self):
        """Start the HTTP server (blocking)"""
        logger.info(f"Starting HTTP server on {config.HTTP_HOST}:{config.HTTP_PORT}")
        self.app.run(
            host=config.HTTP_HOST,
            port=config.HTTP_PORT,
            debug=config.HTTP_DEBUG,
            use_reloader=False  # Disable reloader to avoid double initialization
        )
    
    def run_async(self):
        """
        Start the HTTP server in a background thread (non-blocking)
        Note: For production, use a proper WSGI server like gunicorn
        """
        import threading
        server_thread = threading.Thread(target=self.run, daemon=True)
        server_thread.start()
        logger.info("HTTP server started in background thread")
