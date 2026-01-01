#include <WiFi.h>
#include <HTTPClient.h>
#include <Update.h>

// WiFi credentials - CHANGE THESE
const char* ssid = "Paramtap's iPhone";
const char* password = "0987654321";

// OTA Server details - CHANGE IP to your computer's IP
const char* otaServerIP = "172.20.10.2";  // Your computer's IP address
const int otaServerPort = 5000;

// Current firmware version
const char* firmwareVersion = "1.0.0";

// Check interval (in milliseconds)
const unsigned long checkInterval = 60000; // Check every 60 seconds
unsigned long lastCheck = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n\n=================================");
  Serial.println("ESP32 OTA Update Client");
  Serial.println("=================================");
  Serial.print("Current Firmware Version: ");
  Serial.println(firmwareVersion);
  
  // Connect to WiFi
  connectToWiFi();
  
  Serial.println("\nSetup complete!");
  Serial.println("Will check for updates every 60 seconds...\n");
}

void loop() {
  // Check if WiFi is still connected
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected! Reconnecting...");
    connectToWiFi();
  }
  
  // Check for updates periodically
  if (millis() - lastCheck >= checkInterval) {
    lastCheck = millis();
    checkForUpdates();
  }
  
  // Your normal program code goes here
  delay(1000);
}

void connectToWiFi() {
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWiFi Connection Failed!");
  }
}

void checkForUpdates() {
  Serial.println("\n--- Checking for updates ---");
  
  HTTPClient http;
  String versionUrl = "http://" + String(otaServerIP) + ":" + String(otaServerPort) + "/version";
  
  http.begin(versionUrl);
  int httpCode = http.GET();
  
  if (httpCode == 200) {
    String payload = http.getString();
    Serial.println("Server Response: " + payload);
    
    // Parse version (simple string search)
    int versionStart = payload.indexOf("\"version\":\"") + 11;
    int versionEnd = payload.indexOf("\"", versionStart);
    String serverVersion = payload.substring(versionStart, versionEnd);
    
    Serial.print("Server Version: ");
    Serial.println(serverVersion);
    Serial.print("Current Version: ");
    Serial.println(firmwareVersion);
    
    if (serverVersion != String(firmwareVersion)) {
      Serial.println("New version available! Starting update...");
      performOTAUpdate();
    } else {
      Serial.println("Already running latest version.");
    }
  } else {
    Serial.print("Failed to check version. HTTP Code: ");
    Serial.println(httpCode);
  }
  
  http.end();
}

void performOTAUpdate() {
  Serial.println("\n=================================");
  Serial.println("Starting OTA Update...");
  Serial.println("=================================");
  
  HTTPClient http;
  String updateUrl = "http://" + String(otaServerIP) + ":" + String(otaServerPort) + "/update";
  
  http.begin(updateUrl);
  int httpCode = http.GET();
  
  if (httpCode == 200) {
    int contentLength = http.getSize();
    Serial.print("Firmware size: ");
    Serial.print(contentLength);
    Serial.println(" bytes");
    
    bool canBegin = Update.begin(contentLength);
    
    if (canBegin) {
      Serial.println("Starting update...");
      WiFiClient* client = http.getStreamPtr();
      
      size_t written = Update.writeStream(*client);
      
      if (written == contentLength) {
        Serial.println("Written : " + String(written) + " successfully");
      } else {
        Serial.println("Written only : " + String(written) + "/" + String(contentLength));
      }
      
      if (Update.end()) {
        if (Update.isFinished()) {
          Serial.println("Update successfully completed!");
          Serial.println("Rebooting...");
          delay(1000);
          ESP.restart();
        } else {
          Serial.println("Update not finished? Something went wrong!");
        }
      } else {
        Serial.println("Error Occurred. Error #: " + String(Update.getError()));
      }
    } else {
      Serial.println("Not enough space to begin OTA");
    }
  } else {
    Serial.print("Failed to download firmware. HTTP Code: ");
    Serial.println(httpCode);
  }
  
  http.end();
}