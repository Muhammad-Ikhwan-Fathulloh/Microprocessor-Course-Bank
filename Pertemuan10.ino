#include <Preferences.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ESPmDNS.h>

#define LED_PIN 2

AsyncWebServer server(80);
Preferences prefs;

String ssid;
String password;
bool ledState = false;
bool apMode = false;

unsigned long previousMillis = 0;
const long interval = 5000;

// ============================================
// HTML DASHBOARD LENGKAP (Mode STA - WiFi Terkoneksi)
// ============================================
const char dashboardHTML[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
  <title>ESP32 Dashboard - Connected</title>
  <style>
    body { font-family: Arial, sans-serif; margin: 30px; background: #f5f5f5; }
    .container { max-width: 800px; margin: 0 auto; background: white; padding: 30px; border-radius: 10px; box-shadow: 0 2px 10px rgba(0,0,0,0.1); }
    h1 { color: #4CAF50; border-bottom: 2px solid #4CAF50; padding-bottom: 10px; }
    h2 { color: #333; }
    .status-box { background: #e8f5e8; padding: 15px; border-radius: 5px; margin-bottom: 20px; }
    .status-label { font-weight: bold; color: #555; }
    .status-value { color: #333; }
    .btn { padding: 10px 20px; margin: 5px; border: none; border-radius: 5px; cursor: pointer; font-size: 16px; }
    .btn-on { background: #4CAF50; color: white; }
    .btn-off { background: #f44336; color: white; }
    .btn-config { background: #2196F3; color: white; }
    .wifi-form { background: #e3f2fd; padding: 20px; border-radius: 5px; margin-top: 20px; }
    .form-input { width: 100%; padding: 10px; margin: 8px 0; border: 1px solid #ddd; border-radius: 4px; }
    .form-submit { background: #FF9800; color: white; padding: 12px 20px; border: none; border-radius: 4px; cursor: pointer; }
    .led-indicator { display: inline-block; width: 20px; height: 20px; border-radius: 50%; margin-left: 10px; }
    .led-on { background: #4CAF50; box-shadow: 0 0 10px #4CAF50; }
    .led-off { background: #ccc; }
  </style>
</head>
<body>
  <div class="container">
    <h1>🏠 ESP32 Dashboard</h1>
    
    <div class="status-box">
      <h2>📊 System Status</h2>
      <p><span class="status-label">WiFi Status:</span> 
        <span class="status-value" id="wifiStatus">-</span>
        <span id="wifiIcon">📶</span>
      </p>
      <p><span class="status-label">IP Address:</span> 
        <span class="status-value" id="ipAddress">-</span>
      </p>
      <p><span class="status-label">LED Status:</span> 
        <span class="status-value" id="ledStatus">-</span>
        <span class="led-indicator" id="ledIndicator"></span>
      </p>
    </div>

    <h2>💡 LED Control</h2>
    <button class="btn btn-on" onclick="controlLED('on')">💡 LED ON</button>
    <button class="btn btn-off" onclick="controlLED('off')">🔌 LED OFF</button>
    
    <div class="wifi-form">
      <h2>🔄 Change WiFi Configuration</h2>
      <form id="wifiForm">
        <input type="text" class="form-input" id="ssid" placeholder="WiFi Name (SSID)" required>
        <input type="password" class="form-input" id="password" placeholder="WiFi Password" required>
        <button type="submit" class="form-submit">💾 Save & Reconnect</button>
      </form>
      <p id="wifiMessage" style="margin-top: 10px;"></p>
    </div>
    
    <div style="margin-top: 30px; color: #777; font-size: 14px;">
      <p>🔗 Access this device using: <strong>http://dash.local</strong></p>
      <p>📡 Connected to: <span id="connectedSSID">-</span></p>
    </div>
  </div>

  <script>
  // Update status setiap 2 detik
  function updateStatus() {
    fetch('/status')
      .then(response => response.json())
      .then(data => {
        document.getElementById('wifiStatus').textContent = data.wifi;
        document.getElementById('ipAddress').textContent = data.ip;
        document.getElementById('ledStatus').textContent = data.led ? 'ON' : 'OFF';
        document.getElementById('connectedSSID').textContent = data.ssid;
        
        // Update LED indicator
        const ledIndicator = document.getElementById('ledIndicator');
        ledIndicator.className = 'led-indicator ' + (data.led ? 'led-on' : 'led-off');
        
        // Update WiFi icon
        const wifiIcon = document.getElementById('wifiIcon');
        wifiIcon.textContent = data.wifi === 'Connected' ? '📶' : '❌';
      })
      .catch(error => console.error('Error:', error));
  }
  
  // Kontrol LED
  function controlLED(state) {
    fetch('/led/' + state, { method: 'POST' })
      .then(() => updateStatus())
      .catch(error => console.error('Error:', error));
  }
  
  // Form WiFi configuration
  document.getElementById('wifiForm').addEventListener('submit', function(e) {
    e.preventDefault();
    
    const ssid = document.getElementById('ssid').value;
    const password = document.getElementById('password').value;
    
    if (!ssid || !password) {
      document.getElementById('wifiMessage').textContent = '❌ Please fill in all fields';
      document.getElementById('wifiMessage').style.color = '#f44336';
      return;
    }
    
    fetch('/wifi', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ ssid: ssid, password: password })
    })
    .then(response => response.json())
    .then(data => {
      document.getElementById('wifiMessage').textContent = '✅ ' + data.message;
      document.getElementById('wifiMessage').style.color = '#4CAF50';
      
      // Clear form
      document.getElementById('ssid').value = '';
      document.getElementById('password').value = '';
      
      // Informasi reconnect
      setTimeout(() => {
        document.getElementById('wifiMessage').textContent = '🔄 Reconnecting to new WiFi... Please refresh page in 30 seconds.';
      }, 2000);
    })
    .catch(error => {
      document.getElementById('wifiMessage').textContent = '❌ Error: ' + error;
      document.getElementById('wifiMessage').style.color = '#f44336';
    });
  });
  
  // Auto-update status
  setInterval(updateStatus, 2000);
  updateStatus(); // Initial update
  </script>
</body>
</html>
)=====";

// ============================================
// HTML KONFIGURASI MINIMAL (AP Mode)
// ============================================
const char configHTML[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
  <title>ESP32 WiFi Configuration</title>
  <style>
    body { font-family: Arial, sans-serif; margin: 0; padding: 20px; background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); min-height: 100vh; display: flex; justify-content: center; align-items: center; }
    .config-box { background: white; padding: 40px; border-radius: 15px; box-shadow: 0 10px 30px rgba(0,0,0,0.2); max-width: 400px; width: 100%; text-align: center; }
    h1 { color: #333; margin-bottom: 5px; }
    .subtitle { color: #666; margin-bottom: 30px; }
    .form-group { margin-bottom: 20px; text-align: left; }
    label { display: block; margin-bottom: 8px; color: #555; font-weight: bold; }
    input { width: 100%; padding: 12px; border: 2px solid #ddd; border-radius: 6px; font-size: 16px; box-sizing: border-box; }
    input:focus { border-color: #667eea; outline: none; }
    .btn { background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); color: white; padding: 15px; border: none; border-radius: 6px; font-size: 16px; cursor: pointer; width: 100%; transition: transform 0.2s; }
    .btn:hover { transform: translateY(-2px); }
    .message { margin-top: 20px; padding: 10px; border-radius: 6px; display: none; }
    .success { background: #d4edda; color: #155724; border: 1px solid #c3e6cb; }
    .error { background: #f8d7da; color: #721c24; border: 1px solid #f5c6cb; }
    .info { background: #d1ecf1; color: #0c5460; border: 1px solid #bee5eb; margin-bottom: 20px; padding: 15px; border-radius: 6px; }
  </style>
</head>
<body>
  <div class="config-box">
    <h1>⚙️ ESP32 Setup</h1>
    <p class="subtitle">Configure your WiFi connection</p>
    
    <div class="info">
      📶 <strong>Access Point Mode</strong><br>
      Device: ESP32-Config<br>
      Connect to this network to configure
    </div>
    
    <form id="wifiForm">
      <div class="form-group">
        <label for="ssid">WiFi Network (SSID)</label>
        <input type="text" id="ssid" placeholder="Enter your WiFi name" required>
      </div>
      
      <div class="form-group">
        <label for="password">WiFi Password</label>
        <input type="password" id="password" placeholder="Enter your WiFi password" required>
      </div>
      
      <button type="submit" class="btn">🔗 Connect to WiFi</button>
    </form>
    
    <div id="message" class="message"></div>
    
    <div style="margin-top: 30px; font-size: 14px; color: #777;">
      <p>After saving, device will restart and connect to your WiFi.</p>
      <p>Access via: <strong>http://dash.local</strong></p>
    </div>
  </div>

  <script>
  document.getElementById('wifiForm').addEventListener('submit', function(e) {
    e.preventDefault();
    
    const ssid = document.getElementById('ssid').value;
    const password = document.getElementById('password').value;
    const messageDiv = document.getElementById('message');
    
    if (!ssid || !password) {
      messageDiv.textContent = '❌ Please fill in all fields';
      messageDiv.className = 'message error';
      messageDiv.style.display = 'block';
      return;
    }
    
    // Tampilkan loading
    messageDiv.textContent = '🔄 Connecting...';
    messageDiv.className = 'message info';
    messageDiv.style.display = 'block';
    
    fetch('/wifi', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ ssid: ssid, password: password })
    })
    .then(response => response.json())
    .then(data => {
      messageDiv.textContent = '✅ ' + data.message;
      messageDiv.className = 'message success';
      
      // Tampilkan informasi restart
      setTimeout(() => {
        messageDiv.innerHTML = '🔄 Device will restart and connect to:<br>' +
                              '<strong>' + ssid + '</strong><br>' +
                              'Please connect your device to that network and access:<br>' +
                              '<strong>http://dash.local</strong>';
      }, 2000);
    })
    .catch(error => {
      messageDiv.textContent = '❌ Error: ' + error;
      messageDiv.className = 'message error';
    });
  });
  </script>
</body>
</html>
)=====";

// ============================================
// SETUP FUNCTION
// ============================================
void setup() {
  Serial.begin(115200);
  Serial.println("\n\n🚀 ESP32 WiFi Dashboard Starting...");
  
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  // Initialize Preferences
  prefs.begin("wifi", false);
  ssid = prefs.getString("ssid", "");
  password = prefs.getString("password", "");
  
  Serial.print("📂 Stored SSID: ");
  Serial.println(ssid.length() > 0 ? ssid : "(empty)");
  
  // Try to connect to WiFi or start AP mode
  if (ssid.length() > 0) {
    connectWiFi();
  } else {
    startAPMode();
  }
  
  setupWebServer();
  Serial.println("✅ Setup complete!");
}

// ============================================
// SETUP WEB SERVER ROUTES
// ============================================
void setupWebServer() {
  // Root route - show different page based on mode
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (apMode) {
      // Show configuration page in AP mode
      request->send_P(200, "text/html", configHTML);
    } else {
      // Show full dashboard in STA mode
      request->send_P(200, "text/html", dashboardHTML);
    }
  });
  
  // WiFi configuration endpoint (works in both modes)
  server.on("/wifi", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t, size_t){
      String body = String((char*)data);
      Serial.println("📝 Received WiFi config: " + body);
      
      // Simple JSON parsing
      int ssidStart = body.indexOf("\"ssid\":\"") + 8;
      int ssidEnd = body.indexOf("\"", ssidStart);
      int passStart = body.indexOf("\"password\":\"") + 12;
      int passEnd = body.indexOf("\"", passStart);
      
      if (ssidStart > 7 && ssidEnd > ssidStart && passStart > 11 && passEnd > passStart) {
        ssid = body.substring(ssidStart, ssidEnd);
        password = body.substring(passStart, passEnd);
        
        // Save to preferences
        prefs.putString("ssid", ssid);
        prefs.putString("password", password);
        
        Serial.println("💾 Saved WiFi: " + ssid);
        
        String response = "{\"message\":\"WiFi configuration saved. Device will reconnect.\"}";
        request->send(200, "application/json", response);
        
        // Disconnect and reconnect after a short delay
        delay(1000);
        WiFi.disconnect(true);
        delay(1000);
        connectWiFi();
      } else {
        request->send(400, "application/json", "{\"error\":\"Invalid JSON format\"}");
      }
    }
  );
  
  // LED control endpoints
  server.on("/led/on", HTTP_POST, [](AsyncWebServerRequest *request){
    ledState = true;
    digitalWrite(LED_PIN, HIGH);
    Serial.println("💡 LED turned ON");
    request->send(200, "application/json", "{\"status\":\"LED ON\"}");
  });
  
  server.on("/led/off", HTTP_POST, [](AsyncWebServerRequest *request){
    ledState = false;
    digitalWrite(LED_PIN, LOW);
    Serial.println("💡 LED turned OFF");
    request->send(200, "application/json", "{\"status\":\"LED OFF\"}");
  });
  
  // Status endpoint - returns system status as JSON
  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request){
    String wifiStatus;
    if (WiFi.status() == WL_CONNECTED) {
      wifiStatus = "Connected";
    } else if (apMode) {
      wifiStatus = "AP Mode";
    } else {
      wifiStatus = "Disconnected";
    }
    
    String ipAddress = apMode ? WiFi.softAPIP().toString() : WiFi.localIP().toString();
    String connectedSSID = apMode ? "ESP32-Config (AP Mode)" : WiFi.SSID();
    
    String json = "{";
    json += "\"wifi\":\"" + wifiStatus + "\",";
    json += "\"ip\":\"" + ipAddress + "\",";
    json += "\"ssid\":\"" + connectedSSID + "\",";
    json += "\"led\":" + String(ledState ? "true" : "false");
    json += "}";
    
    request->send(200, "application/json", json);
  });
  
  // Configuration page (always accessible, even in STA mode)
  server.on("/config", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", configHTML);
  });
  
  // Start server
  server.begin();
  Serial.println("🌐 Web server started on port 80");
}

// ============================================
// LOOP FUNCTION
// ============================================
void loop() {
  // Check WiFi connection periodically (only in STA mode)
  if (!apMode) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      
      if (WiFi.status() != WL_CONNECTED) {
        Serial.println("⚠️ WiFi disconnected, attempting to reconnect...");
        connectWiFi();
      }
    }
  }
}

// ============================================
// CONNECT TO WIFI FUNCTION
// ============================================
void connectWiFi() {
  Serial.println("\n📡 Attempting to connect to WiFi...");
  Serial.print("SSID: ");
  Serial.println(ssid);
  
  WiFi.disconnect(true);
  delay(1000);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());
  
  unsigned long startTime = millis();
  bool connected = false;
  
  // Try to connect for 20 seconds
  while (millis() - startTime < 20000) {
    if (WiFi.status() == WL_CONNECTED) {
      connected = true;
      break;
    }
    Serial.print(".");
    delay(500);
  }
  
  if (connected) {
    apMode = false;
    Serial.println("\n✅ WiFi Connected!");
    Serial.print("📶 IP Address: ");
    Serial.println(WiFi.localIP());
    
    // Start mDNS with hostname "dash"
    if (MDNS.begin("dash")) {
      Serial.println("🔗 mDNS responder started: http://dash.local");
    } else {
      Serial.println("❌ Error setting up mDNS responder!");
    }
  } else {
    Serial.println("\n❌ Failed to connect to WiFi");
    Serial.println("🔄 Starting AP Mode for configuration...");
    startAPMode();
  }
}

// ============================================
// START AP MODE FUNCTION
// ============================================
void startAPMode() {
  Serial.println("\n📡 Starting AP Mode...");
  
  WiFi.disconnect(true);
  delay(1000);
  
  WiFi.mode(WIFI_AP);
  WiFi.softAP("ESP32-Config", ""); // No password for easier configuration
  
  apMode = true;
  
  Serial.println("✅ AP Mode Started!");
  Serial.print("📶 AP IP Address: ");
  Serial.println(WiFi.softAPIP());
  
  // Start mDNS with different hostname in AP mode
  if (MDNS.begin("esp32-config")) {
    Serial.println("🔗 mDNS responder started: http://esp32-config.local");
  }
}

// ============================================
// WIFI CONNECTION CHECK (Non-blocking)
// ============================================
void checkWiFiConnection() {
  static unsigned long lastCheck = 0;
  unsigned long currentMillis = millis();
  
  if (currentMillis - lastCheck >= 5000 && !apMode) {
    lastCheck = currentMillis;
    
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("⚠️ WiFi connection lost!");
      
      // Try to reconnect 3 times before switching to AP mode
      static int reconnectAttempts = 0;
      reconnectAttempts++;
      
      if (reconnectAttempts >= 3) {
        Serial.println("🔄 Switching to AP Mode after 3 failed attempts");
        startAPMode();
        reconnectAttempts = 0;
      } else {
        Serial.println("🔄 Attempting to reconnect...");
        WiFi.disconnect();
        WiFi.reconnect();
      }
    }
  }
}
