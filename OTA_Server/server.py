from flask import Flask, send_file, jsonify, request
import os
from datetime import datetime

app = Flask(__name__)

# Firmware directory
FIRMWARE_DIR = 'firmware'

# Current firmware version (update this when you upload new firmware)
CURRENT_VERSION = "1.0.1"

@app.route('/')
def home():
    return """
    <h1>ESP32 OTA Update Server</h1>
    <p>Server is running!</p>
    <p>Current firmware version: {}</p>
    <p>Available endpoints:</p>
    <ul>
        <li>/version - Check firmware version</li>
        <li>/update - Download firmware binary</li>
    </ul>
    """.format(CURRENT_VERSION)

@app.route('/version', methods=['GET'])
def get_version():
    """Return current firmware version"""
    return jsonify({
        'version': CURRENT_VERSION,
        'timestamp': datetime.now().isoformat()
    })

@app.route('/update', methods=['GET'])
def download_firmware():
    """Serve the firmware binary file"""
    firmware_file = os.path.join(FIRMWARE_DIR, 'firmware.bin')
    
    if not os.path.exists(firmware_file):
        return jsonify({'error': 'Firmware not found'}), 404
    
    return send_file(
        firmware_file,
        mimetype='application/octet-stream',
        as_attachment=True,
        download_name='firmware.bin'
    )

@app.route('/upload', methods=['POST'])
def upload_firmware():
    """Upload new firmware (for future use)"""
    if 'file' not in request.files:
        return jsonify({'error': 'No file provided'}), 400
    
    file = request.files['file']
    if file.filename == '':
        return jsonify({'error': 'No file selected'}), 400
    
    if file and file.filename.endswith('.bin'):
        filepath = os.path.join(FIRMWARE_DIR, 'firmware.bin')
        file.save(filepath)
        return jsonify({'message': 'Firmware uploaded successfully'}), 200
    
    return jsonify({'error': 'Invalid file format'}), 400

if __name__ == '__main__':
    # Create firmware directory if it doesn't exist
    if not os.path.exists(FIRMWARE_DIR):
        os.makedirs(FIRMWARE_DIR)
    
    # Run server on all network interfaces
    print("Starting OTA Server...")
    print("Server will be accessible at: http://YOUR_IP:5000")
    app.run(host='0.0.0.0', port=5000, debug=True)