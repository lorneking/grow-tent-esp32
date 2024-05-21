#include "wifi_driver.h"

void WiFiDriver::begin(const char* ssid, const char* password) {
    _ssid = ssid;
    _password = password;

    Serial.begin(115200);
    delay(10);

    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(_ssid);

    WiFi.begin(_ssid, _password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("Wi-Fi connected.");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    // Start the web server
    server.on("/", std::bind(&WiFiDriver::handleRoot, this));
    server.on("/data", std::bind(&WiFiDriver::handleData, this));  // Add endpoint for JSON data
    server.onNotFound(std::bind(&WiFiDriver::handleNotFound, this));
    server.begin();
    Serial.println("HTTP server started");
}

bool WiFiDriver::isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

IPAddress WiFiDriver::getLocalIP() {
    return WiFi.localIP();
}

void WiFiDriver::handleClient() {
    server.handleClient();
}

void WiFiDriver::updateSensorValues(float voltage3V3, float voltage5V, float tdsValue, float phValue, float temperature) {
    _voltage3V3 = voltage3V3;
    _voltage5V = voltage5V;
    _tdsValue = tdsValue;
    _phValue = phValue;
    _temperature = temperature;
}

void WiFiDriver::handleRoot() {
    String html = "<html><body><h1>Grow Controller Dashboard</h1>"
                  "<p>3.3V Rail: " + String(_voltage3V3) + " V</p>"
                  "<p>5V Rail: " + String(_voltage5V) + " V</p>"
                  "<p>TDS Value: " + String(_tdsValue) + " ppm</p>"
                  "<p>pH Value: " + String(_phValue) + "</p>"
                  "<p>Temperature: " + String(_temperature) + " °C</p>"
                  "</body></html>";
    server.send(200, "text/html", html);
}

void WiFiDriver::handleData() {
    String json = "{";
    json += "\"voltage3V3\":" + String(_voltage3V3) + ",";
    json += "\"voltage5V\":" + String(_voltage5V) + ",";
    json += "\"tdsValue\":" + String(_tdsValue) + ",";
    json += "\"phValue\":" + String(_phValue) + ",";
    json += "\"temperature\":" + String(_temperature);
    json += "}";
    server.send(200, "application/json", json);
}

void WiFiDriver::handleNotFound() {
    server.send(404, "text/plain", "404: Not found");
}
