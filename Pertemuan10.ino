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
  <title>ESP32 Dashboard</title>
  <style>
    body { 
      font-family: Arial, sans-serif; 
      margin: 20px; 
      background-color: #f0f0f0; 
    }
    .container {
      max-width: 800px;
      margin: 0 auto;
      background: white;
      padding: 20px;
      border-radius: 8px;
      box-shadow: 0 2px 4px rgba(0,0,0,0.1);
    }
    h1 {
      color: #2c3e50;
      border-bottom: 2px solid #3498db;
      padding-bottom: 10px;
    }
    h2 {
      color: #34495e;
      margin-top: 25px;
    }
    .status-box {
      background-color: #ecf0f1;
      padding: 15px;
      border-radius: 5px;
      margin-bottom: 20px;
    }
    .status-item {
      margin-bottom: 10px;
    }
    .label {
      font-weight: bold;
      color: #2c3e50;
      display: inline-block;
      width: 150px;
    }
    .value {
      color: #7f8c8d;
    }
    .led-control {
      margin: 20px 0;
    }
    .btn {
      padding: 10px 20px;
      margin-right: 10px;
      border: none;
      border-radius: 4px;
      cursor: pointer;
      font-size: 16px;
      font-weight: bold;
    }
    .btn-on {
      background-color: #27ae60;
      color: white;
    }
    .btn-off {
      background-color: #e74c3c;
      color: white;
    }
    .wifi-form {
      background-color: #e8f4f8;
      padding: 20px;
      border-radius: 5px;
      margin-top: 20px;
    }
    .form-group {
      margin-bottom: 15px;
    }
    .form-label {
      display: block;
      margin-bottom: 5px;
      font-weight: bold;
      color: #2c3e50;
    }
    .form-input {
      width: 100%;
      padding: 8px;
      border: 1px solid #bdc3c7;
      border-radius: 4px;
      box-sizing: border-box;
    }
    .form-submit {
      background-color: #3498db;
      color: white;
      padding: 10px 20px;
      border: none;
      border-radius: 4px;
      cursor: pointer;
      font-size: 16px;
    }
    .message {
      margin-top: 15px;
      padding: 10px;
      border-radius: 4px;
      display: none;
    }
    .message-success {
      background-color: #d4edda;
      color: #155724;
      border: 1px solid #c3e6cb;
    }
    .message-error {
      background-color: #f8d7da;
      color: #721c24;
      border: 1px solid #f5c6cb;
    }
    .config-link {
      display: inline-block;
      margin-top: 10px;
      color: #2980b9;
      text-decoration: none;
    }
    .config-link:hover {
      text-decoration: underline;
    }
    .led-indicator {
      display: inline-block;
      width: 12px;
      height: 12px;
      border-radius: 50%;
      margin-left: 10px;
      vertical-align: middle;
    }
    .led-on {
      background-color: #27ae60;
    }
    .led-off {
      background-color: #95a5a6;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>ESP32 Dashboard</h1>
    
    <div class="status-box">
      <h2>System Status</h2>
      <div class="status-item">
        <span class="label">WiFi Status:</span>
        <span class="value" id="wifiStatus">Checking...</span>
      </div>
      <div class="status-item">
        <span class="label">IP Address:</span>
        <span class="value" id="ipAddress">-</span>
      </div>
      <div class="status-item">
        <span class="label">Connected to:</span>
        <span class="value" id="connectedSSID">-</span>
      </div>
      <div class="status-item">
        <span class="label">LED Status:</span>
        <span class="value" id="ledStatus">OFF</span>
        <span class="led-indicator led-off" id="ledIndicator"></span>
      </div>
    </div>

    <h2>LED Control</h2>
    <div class="led-control">
      <button class="btn btn-on" onclick="controlLED('on')">LED ON</button>
      <button class="btn btn-off" onclick="controlLED('off')">LED OFF</button>
    </div>
    
    <div class="wifi-form">
      <h2>WiFi Configuration</h2>
      <p>Change WiFi network settings:</p>
      <form id="wifiForm">
        <div class="form-group">
          <label class="form-label" for="ssid">WiFi Name (SSID)</label>
          <input type="text" class="form-input" id="ssid" placeholder="Enter WiFi name" required>
        </div>
        <div class="form-group">
          <label class="form-label" for="password">WiFi Password</label>
          <input type="password" class="form-input" id="password" placeholder="Enter WiFi password" required>
        </div>
        <button type="submit" class="form-submit">Save & Reconnect</button>
      </form>
      <div id="wifiMessage" class="message"></div>
    </div>
    
    <div style="margin-top: 30px; padding: 15px; background-color: #f8f9fa; border-radius: 5px;">
      <p><strong>Access Information:</strong></p>
      <p>This device: <strong>http://dash.local</strong></p>
      <p>For configuration only: <a href="/config" class="config-link">Go to Config Page</a></p>
    </div>
  </div>

  <script>
  // Update status
  function updateStatus() {
    fetch('/status')
      .then(response => response.json())
      .then(data => {
        document.getElementById('wifiStatus').textContent = data.wifi;
        document.getElementById('ipAddress').textContent = data.ip;
        document.getElementById('connectedSSID').textContent = data.ssid;
        document.getElementById('ledStatus').textContent = data.led ? 'ON' : 'OFF';
        
        // Update LED indicator
        const ledIndicator = document.getElementById('ledIndicator');
        ledIndicator.className = 'led-indicator ' + (data.led ? 'led-on' : 'led-off');
      })
      .catch(error => {
        console.error('Error:', error);
        document.getElementById('wifiStatus').textContent = 'Error';
      });
  }
  
  // Control LED
  function controlLED(state) {
    fetch('/led/' + state, { method: 'POST' })
      .then(() => updateStatus())
      .catch(error => console.error('Error:', error));
  }
  
  // WiFi configuration form
  document.getElementById('wifiForm').addEventListener('submit', function(e) {
    e.preventDefault();
    
    const ssid = document.getElementById('ssid').value;
    const password = document.getElementById('password').value;
    const messageDiv = document.getElementById('wifiMessage');
    
    if (!ssid || !password) {
      messageDiv.textContent = 'Error: Please fill in all fields';
      messageDiv.className = 'message message-error';
      messageDiv.style.display = 'block';
      return;
    }
    
    messageDiv.textContent = 'Connecting...';
    messageDiv.className = 'message';
    messageDiv.style.display = 'block';
    
    fetch('/wifi', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ ssid: ssid, password: password })
    })
    .then(response => response.json())
    .then(data => {
      if (data.message) {
        messageDiv.textContent = 'Success: ' + data.message;
        messageDiv.className = 'message message-success';
        
        // Clear form
        document.getElementById('ssid').value = '';
        document.getElementById('password').value = '';
        
        // Show reconnect message
        setTimeout(() => {
          messageDiv.innerHTML = 'Device will reconnect to the new WiFi network.<br>' +
                                'Please wait and refresh the page in 30 seconds.';
        }, 2000);
      } else if (data.error) {
        messageDiv.textContent = 'Error: ' + data.error;
        messageDiv.className = 'message message-error';
      }
    })
    .catch(error => {
      messageDiv.textContent = 'Error: ' + error;
      messageDiv.className = 'message message-error';
    });
  });
  
  // Auto-update status every 2 seconds
  setInterval(updateStatus, 2000);
  
  // Initial update
  updateStatus();
  </script>
</body>
</html>
)=====";

// ============================================
// HTML KONFIGURASI (AP Mode dan Config Page)
// ============================================
const char configHTML[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
  <title>ESP32 WiFi Configuration</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      margin: 0;
      padding: 20px;
      background-color: #e9ecef;
      display: flex;
      justify-content: center;
      align-items: center;
      min-height: 100vh;
    }
    .config-container {
      background: white;
      padding: 30px;
      border-radius: 8px;
      box-shadow: 0 4px 6px rgba(0,0,0,0.1);
      max-width: 400px;
      width: 100%;
    }
    h1 {
      color: #2c3e50;
      text-align: center;
      margin-bottom: 5px;
    }
    .mode-indicator {
      text-align: center;
      color: #7f8c8d;
      margin-bottom: 25px;
      padding: 8px;
      background-color: #f8f9fa;
      border-radius: 4px;
    }
    .form-group {
      margin-bottom: 20px;
    }
    label {
      display: block;
      margin-bottom: 8px;
      color: #2c3e50;
      font-weight: bold;
    }
    input {
      width: 100%;
      padding: 10px;
      border: 1px solid #ced4da;
      border-radius: 4px;
      box-sizing: border-box;
      font-size: 16px;
    }
    input:focus {
      border-color: #3498db;
      outline: none;
      box-shadow: 0 0 0 3px rgba(52, 152, 219, 0.2);
    }
    .submit-btn {
      background-color: #3498db;
      color: white;
      padding: 12px;
      border: none;
      border-radius: 4px;
      font-size: 16px;
      font-weight: bold;
      cursor: pointer;
      width: 100%;
      transition: background-color 0.3s;
    }
    .submit-btn:hover {
      background-color: #2980b9;
    }
    .message {
      margin-top: 20px;
      padding: 12px;
      border-radius: 4px;
      display: none;
    }
    .success {
      background-color: #d4edda;
      color: #155724;
      border: 1px solid #c3e6cb;
    }
    .error {
      background-color: #f8d7da;
      color: #721c24;
      border: 1px solid #f5c6cb;
    }
    .info-box {
      background-color: #e8f4f8;
      padding: 15px;
      border-radius: 5px;
      margin-bottom: 20px;
      font-size: 14px;
    }
    .back-link {
      display: block;
      text-align: center;
      margin-top: 20px;
      color: #3498db;
      text-decoration: none;
    }
    .back-link:hover {
      text-decoration: underline;
    }
  </style>
</head>
<body>
  <div class="config-container">
    <h1>ESP32 WiFi Setup</h1>
    
    <div class="mode-indicator" id="modeIndicator">
      <!-- Filled by JavaScript -->
    </div>
    
    <div class="info-box">
      <p><strong>Current Mode:</strong> <span id="currentMode">Checking...</span></p>
      <p><strong>Access via:</strong> <span id="accessInfo">-</span></p>
    </div>
    
    <form id="wifiForm">
      <div class="form-group">
        <label for="ssid">WiFi Network Name (SSID)</label>
        <input type="text" id="ssid" placeholder="Enter WiFi name" required>
      </div>
      
      <div class="form-group">
        <label for="password">WiFi Password</label>
        <input type="password" id="password" placeholder="Enter WiFi password" required>
      </div>
      
      <button type="submit" class="submit-btn">Save WiFi Settings</button>
    </form>
    
    <div id="message" class="message"></div>
    
    <a href="/" class="back-link" id="backLink">Back to Dashboard</a>
  </div>

  <script>
  // Set mode indicator based on current state
  function setModeInfo() {
    const modeIndicator = document.getElementById('modeIndicator');
    const currentMode = document.getElementById('currentMode');
    const accessInfo = document.getElementById('accessInfo');
    const backLink = document.getElementById('backLink');
    
    // Check if we're in AP mode (no WiFi connection)
    if (!navigator.onLine || window.location.hostname.includes('esp32-config')) {
      modeIndicator.innerHTML = '<strong>Access Point Mode</strong><br>Configure WiFi settings';
      currentMode.textContent = 'AP Mode (ESP32-Config)';
      accessInfo.textContent = 'http://esp32-config.local or 192.168.4.1';
      backLink.style.display = 'none'; // Hide back link in AP mode
    } else {
      modeIndicator.innerHTML = '<strong>WiFi Configuration</strong><br>Change existing WiFi settings';
      currentMode.textContent = 'WiFi Connected Mode';
      accessInfo.textContent = 'http://dash.local';
      backLink.style.display = 'block';
    }
  }
  
  // WiFi configuration form
  document.getElementById('wifiForm').addEventListener('submit', function(e) {
    e.preventDefault();
    
    const ssid = document.getElementById('ssid').value;
    const password = document.getElementById('password').value;
    const messageDiv = document.getElementById('message');
    
    if (!ssid || !password) {
      messageDiv.textContent = 'Error: Please fill in all fields';
      messageDiv.className = 'message error';
      messageDiv.style.display = 'block';
      return;
    }
    
    messageDiv.textContent = 'Saving and reconnecting...';
    messageDiv.className = 'message';
    messageDiv.style.display = 'block';
    
    fetch('/wifi', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ ssid: ssid, password: password })
    })
    .then(response => response.json())
    .then(data => {
      if (data.message) {
        messageDiv.textContent = 'Success: ' + data.message;
        messageDiv.className = 'message success';
        
        // Clear form
        document.getElementById('ssid').value = '';
        document.getElementById('password').value = '';
        
        // Show restart message
        setTimeout(() => {
          messageDiv.innerHTML = 'WiFi settings saved.<br>' +
                                'Device will restart and connect to the new network.<br>' +
                                'Please reconnect your device to the new WiFi and access:<br>' +
                                '<strong>http://dash.local</strong>';
        }, 1500);
      } else if (data.error) {
        messageDiv.textContent = 'Error: ' + data.error;
        messageDiv.className = 'message error';
      }
    })
    .catch(error => {
      messageDiv.textContent = 'Error: Could not connect to server';
      messageDiv.className = 'message error';
    });
  });
  
  // Set initial mode info
  setModeInfo();
  </script>
</body>
</html>
)=====";

// ============================================
// SETUP FUNCTION
// ============================================
void setup() {
  Serial.begin(115200);
  Serial.println("\nESP32 WiFi Dashboard Starting...");
  
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  // Initialize Preferences
  prefs.begin("wifi", false);
  ssid = prefs.getString("ssid", "");
  password = prefs.getString("password", "");
  
  Serial.print("Stored SSID: ");
  Serial.println(ssid.length() > 0 ? ssid : "(empty)");
  
  // Try to connect to WiFi or start AP mode
  if (ssid.length() > 0) {
    connectWiFi();
  } else {
    startAPMode();
  }
  
  setupWebServer();
  Serial.println("Setup complete");
}

// ============================================
// SETUP WEB SERVER ROUTES
// ============================================
void setupWebServer() {
  // Root route - show different page based on mode
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (apMode) {
      request->send_P(200, "text/html", configHTML);
    } else {
      request->send_P(200, "text/html", dashboardHTML);
    }
  });
  
  // Configuration page (always accessible)
  server.on("/config", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", configHTML);
  });
  
  // WiFi configuration endpoint
  server.on("/wifi", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t, size_t){
      String body = String((char*)data);
      Serial.println("Received WiFi config: " + body);
      
      // Parse JSON
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
        
        Serial.println("Saved WiFi: " + ssid);
        
        String response = "{\"message\":\"WiFi configuration saved. Device will reconnect.\"}";
        request->send(200, "application/json", response);
        
        // Disconnect and reconnect
        delay(1000);
        WiFi.disconnect(true);
        delay(1000);
        connectWiFi();
      } else {
        request->send(400, "application/json", "{\"error\":\"Invalid data format\"}");
      }
    }
  );
  
  // LED control endpoints
  server.on("/led/on", HTTP_POST, [](AsyncWebServerRequest *request){
    ledState = true;
    digitalWrite(LED_PIN, HIGH);
    Serial.println("LED turned ON");
    request->send(200, "application/json", "{\"status\":\"LED ON\"}");
  });
  
  server.on("/led/off", HTTP_POST, [](AsyncWebServerRequest *request){
    ledState = false;
    digitalWrite(LED_PIN, LOW);
    Serial.println("LED turned OFF");
    request->send(200, "application/json", "{\"status\":\"LED OFF\"}");
  });
  
  // Status endpoint
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
  
  // Start server
  server.begin();
  Serial.println("Web server started on port 80");
}

// ============================================
// LOOP FUNCTION
// ============================================
void loop() {
  if (!apMode) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      
      if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi disconnected, reconnecting...");
        connectWiFi();
      }
    }
  }
}

// ============================================
// CONNECT TO WIFI
// ============================================
void connectWiFi() {
  Serial.println("Connecting to WiFi...");
  Serial.print("SSID: ");
  Serial.println(ssid);
  
  WiFi.disconnect(true);
  delay(1000);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());
  
  unsigned long startTime = millis();
  bool connected = false;
  
  // Try for 20 seconds
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
    Serial.println("\nWiFi Connected");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    
    // Start mDNS
    if (MDNS.begin("dash")) {
      Serial.println("mDNS: http://dash.local");
    }
  } else {
    Serial.println("\nFailed to connect to WiFi");
    Serial.println("Starting AP Mode...");
    startAPMode();
  }
}

// ============================================
// START AP MODE
// ============================================
void startAPMode() {
  Serial.println("Starting AP Mode...");
  
  WiFi.disconnect(true);
  delay(1000);
  
  WiFi.mode(WIFI_AP);
  WiFi.softAP("ESP32-Config", "");
  
  apMode = true;
  
  Serial.println("AP Mode Started");
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());
  
  // Start mDNS for AP mode
  if (MDNS.begin("esp32-config")) {
    Serial.println("mDNS: http://esp32-config.local");
  }
}
