from flask import Flask, render_template, jsonify
import requests

app = Flask(__name__)

ESP32_IP = "http://192.168.1.55"  # Replace with your ESP32's IP address

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/data')
def get_data():
    try:
        response = requests.get(ESP32_IP + "/data")
        data = response.json()
    except Exception as e:
        data = {"error": str(e)}
    return jsonify(data)

if __name__ == '__main__':
    app.run(debug=True)
