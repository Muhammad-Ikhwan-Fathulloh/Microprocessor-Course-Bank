# ESP32 WiFi Configuration, LED Control, dan mDNS Dashboard

## 1. **Pendahuluan Kode**

Kode ini membuat sistem IoT ESP32 dengan kemampuan:
- **WiFi Management**: Koneksi WiFi dinamis dengan penyimpanan NVS
- **Web Dashboard**: Antarmuka web untuk konfigurasi dan kontrol
- **mDNS**: Akses via `dash.local` tanpa perlu IP address
- **Dual Mode**: Mode STA (Client WiFi) dan AP (Access Point)
- **Serial Monitor**: Kontrol via command line

---

## 2. **Penjelasan**

### **2.1 Inisialisasi Library dan Variabel Global**

```cpp
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
```

**Penjelasan:**
- `Preferences.h`: Untuk penyimpanan data permanen di flash memory
- `ESPAsyncWebServer.h`: Web server non-blocking (asynchronous)
- `ESPmDNS.h`: Multicast DNS untuk domain lokal
- `LED_PIN 2`: Pin LED onboard ESP32 (bisa disesuaikan)
- `server(80)`: Web server berjalan di port HTTP standar (80)
- `prefs`: Objek untuk mengakses Preferences
- `apMode`: Flag untuk status mode Access Point
- `interval`: Interval pengecekan koneksi WiFi (5 detik)

---

### **2.2 HTML Dashboard (Disimpan di Flash Memory)**

```cpp
const char webpage[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
  <title>ESP32 Dashboard</title>
</head>
<body>
  <h1>ESP32 Dashboard</h1>

  <h3>Status</h3>
  <p>WiFi: <span id="wifi">-</span></p>
  <p>LED: <span id="led">-</span></p>

  <h3>WiFi Configuration</h3>
  <form id="wifiForm">
    <input type="text" id="ssid" placeholder="SSID"><br><br>
    <input type="password" id="password" placeholder="Password"><br><br>
    <button type="submit">Save WiFi</button>
  </form>

  <h3>LED Control</h3>
  <button onclick="ledOn()">LED ON</button>
  <button onclick="ledOff()">LED OFF</button>

<script>
function refreshStatus(){
  fetch('/status').then(r=>r.json()).then(d=>{
    document.getElementById('wifi').innerText = d.wifi;
    document.getElementById('led').innerText = d.led ? 'ON':'OFF';
  })
}

setInterval(refreshStatus,2000);
refreshStatus();

function ledOn(){ fetch('/led/on',{method:'POST'}); }
function ledOff(){ fetch('/led/off',{method:'POST'}); }

document.getElementById('wifiForm').addEventListener('submit',e=>{
  e.preventDefault();
  fetch('/wifi',{method:'POST',headers:{'Content-Type':'application/json'},
    body:JSON.stringify({ssid:ssid.value,password:password.value})});
});
</script>
</body>
</html>
)=====";
```

**Penjelasan:**
- `PROGMEM`: Menyimpan HTML di flash memory, bukan RAM
- **Struktur Dashboard**:
  - Bagian status WiFi dan LED
  - Form konfigurasi WiFi
  - Tombol kontrol LED
- **JavaScript Features**:
  - `refreshStatus()`: Polling status setiap 2 detik via fetch API
  - Kontrol LED dengan HTTP POST
  - Pengiriman data WiFi dalam format JSON

---

### **2.3 Fungsi Setup() - Bagian 1: Inisialisasi Dasar**

```cpp
void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  prefs.begin("wifi", false);
  ssid = prefs.getString("ssid", "");
  password = prefs.getString("password", "");

  if (ssid == "") {
    startAPMode();
  } else {
    connectWiFi();
  }
```

**Penjelasan:**
1. `Serial.begin(115200)`: Mulai komunikasi serial untuk debugging
2. `pinMode(LED_PIN, OUTPUT)`: Konfigurasi pin LED sebagai output
3. `prefs.begin("wifi", false)`: Buka namespace "wifi" di Preferences
4. `getString()`: Ambil SSID dan password yang tersimpan
5. Logika: Jika SSID kosong → Mode AP, jika ada → Mode STA

---

### **2.4 Fungsi Setup() - Bagian 2: Route Web Server**

```cpp
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", webpage);
  });
```

**Penjelasan:**
- `server.on()`: Mendefinisikan route/endpoint
- `"/"`: Route utama (root)
- `HTTP_GET`: Hanya merespon metode GET
- `request->send_P()`: Kirim halaman HTML dari PROGMEM

---

### **2.5 Fungsi Setup() - Bagian 3: Route WiFi Configuration (POST)**

```cpp
  server.on("/wifi", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t, size_t){
      String body = String((char*)data);
      int s = body.indexOf("ssid");
      int p = body.indexOf("password");
      ssid = body.substring(body.indexOf(':',s)+2, body.indexOf('"', body.indexOf(':',s)+2));
      password = body.substring(body.indexOf(':',p)+2, body.lastIndexOf('"'));

      prefs.putString("ssid", ssid);
      prefs.putString("password", password);

      request->send(200, "application/json", "{\"status\":\"saved\"}");
      WiFi.disconnect(true);
      connectWiFi();
    }
  );
```

**Penjelasan:**
1. **Parsing JSON Manual**: Mengurai data JSON dari body request
   - `body.indexOf("ssid")`: Cari posisi field ssid
   - `substring()`: Ekstrak nilai ssid dan password
2. **Simpan ke Preferences**:
   - `prefs.putString()`: Simpan ke flash memory
3. **Reset WiFi**: 
   - `WiFi.disconnect(true)`: Putuskan koneksi
   - `connectWiFi()`: Coba konek ulang dengan kredensial baru

---

### **2.6 Fungsi Setup() - Bagian 4: Route LED Control**

```cpp
  server.on("/led/on", HTTP_POST, [](AsyncWebServerRequest *request){
    ledState = true;
    digitalWrite(LED_PIN, HIGH);
    request->send(200);
  });

  server.on("/led/off", HTTP_POST, [](AsyncWebServerRequest *request){
    ledState = false;
    digitalWrite(LED_PIN, LOW);
    request->send(200);
  });
```

**Penjelasan:**
- **Endpoint `/led/on`**: Nyalakan LED dan update state
- **Endpoint `/led/off`**: Matikan LED dan update state
- `request->send(200)`: Kirim response HTTP 200 (OK)

---

### **2.7 Fungsi Setup() - Bagian 5: Route Status JSON**

```cpp
  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = "{\"wifi\":\"" + String(WiFi.status()==WL_CONNECTED?"Connected":"Disconnected") +
                  "\",\"led\":" + String(ledState?"true":"false") + "}";
    request->send(200, "application/json", json);
  });

  server.begin();
}
```

**Penjelasan:**
- **Dynamic JSON Generation**: Membuat JSON string secara manual
- **Kondisi WiFi**: `WL_CONNECTED` vs `WL_DISCONNECTED`
- **LED State**: Boolean `true`/`false`
- `server.begin()`: Mulai web server (non-blocking)

---

### **2.8 Fungsi Loop Utama**

```cpp
void loop() {
  if (!apMode) connectWifiCheck();
}
```

**Penjelasan:**
- Minimalis: Hanya cek WiFi jika bukan mode AP
- Non-blocking: Tidak delay(), semua asynchronous

---

### **2.9 Fungsi connectWiFi() - Koneksi ke Jaringan**

```cpp
void connectWiFi() {
  WiFi.begin(ssid.c_str(), password.c_str());
  unsigned long start = millis();

  while (WiFi.status() != WL_CONNECTED && millis()-start < 15000) {
    delay(500);
  }

  if (WiFi.status() == WL_CONNECTED) {
    apMode = false;
    if (MDNS.begin("dash")) {
      Serial.println("mDNS aktif: http://dash.local");
    }
  } else {
    startAPMode();
  }
}
```

**Penjelasan:**
1. **Koneksi dengan Timeout**:
   - `WiFi.begin()`: Mulai koneksi
   - Timeout 15 detik: `millis()-start < 15000`
2. **Jika Berhasil**:
   - `apMode = false`: Keluar dari mode AP
   - `MDNS.begin("dash")`: Aktifkan mDNS dengan hostname "dash"
3. **Jika Gagal**: Panggil `startAPMode()`

---

### **2.10 Fungsi startAPMode()**

```cpp
void startAPMode() {
  apMode = true;
  WiFi.softAP("ESP32-Config");
  Serial.println("AP Mode aktif: ESP32-Config");
}
```

**Penjelasan:**
- `WiFi.softAP()`: Buat access point dengan SSID "ESP32-Config"
- **Tidak ada password**: Untuk kemudahan konfigurasi awal
- **AP IP Default**: Biasanya `192.168.4.1`

---

### **2.11 Fungsi connectWifiCheck()**

```cpp
void connectWifiCheck() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    if (WiFi.status() != WL_CONNECTED) connectWiFi();
  }
}
```

**Penjelasan:**
- **Non-blocking Timer**: Menggunakan `millis()` bukan `delay()`
- **Interval Check**: Cek koneksi setiap 5 detik
- **Reconnect Logic**: Jika terputus, coba sambung kembali

---

## 3. **KODE PROGRAM LENGKAP**

```cpp
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

const char webpage[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
  <title>ESP32 Dashboard</title>
</head>
<body>
  <h1>ESP32 Dashboard</h1>

  <h3>Status</h3>
  <p>WiFi: <span id="wifi">-</span></p>
  <p>LED: <span id="led">-</span></p>

  <h3>WiFi Configuration</h3>
  <form id="wifiForm">
    <input type="text" id="ssid" placeholder="SSID"><br><br>
    <input type="password" id="password" placeholder="Password"><br><br>
    <button type="submit">Save WiFi</button>
  </form>

  <h3>LED Control</h3>
  <button onclick="ledOn()">LED ON</button>
  <button onclick="ledOff()">LED OFF</button>

<script>
function refreshStatus(){
  fetch('/status').then(r=>r.json()).then(d=>{
    document.getElementById('wifi').innerText = d.wifi;
    document.getElementById('led').innerText = d.led ? 'ON':'OFF';
  })
}

setInterval(refreshStatus,2000);
refreshStatus();

function ledOn(){ fetch('/led/on',{method:'POST'}); }
function ledOff(){ fetch('/led/off',{method:'POST'}); }

document.getElementById('wifiForm').addEventListener('submit',e=>{
  e.preventDefault();
  fetch('/wifi',{method:'POST',headers:{'Content-Type':'application/json'},
    body:JSON.stringify({ssid:ssid.value,password:password.value})});
});
</script>
</body>
</html>
)=====";

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  prefs.begin("wifi", false);
  ssid = prefs.getString("ssid", "");
  password = prefs.getString("password", "");

  if (ssid == "") {
    startAPMode();
  } else {
    connectWiFi();
  }

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", webpage);
  });

  server.on("/wifi", HTTP_POST, [](AsyncWebServerRequest *request){}, NULL,
    [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t, size_t){
      String body = String((char*)data);
      int s = body.indexOf("ssid");
      int p = body.indexOf("password");
      ssid = body.substring(body.indexOf(':',s)+2, body.indexOf('"', body.indexOf(':',s)+2));
      password = body.substring(body.indexOf(':',p)+2, body.lastIndexOf('"'));

      prefs.putString("ssid", ssid);
      prefs.putString("password", password);

      request->send(200, "application/json", "{\"status\":\"saved\"}");
      WiFi.disconnect(true);
      connectWiFi();
    }
  );

  server.on("/led/on", HTTP_POST, [](AsyncWebServerRequest *request){
    ledState = true;
    digitalWrite(LED_PIN, HIGH);
    request->send(200);
  });

  server.on("/led/off", HTTP_POST, [](AsyncWebServerRequest *request){
    ledState = false;
    digitalWrite(LED_PIN, LOW);
    request->send(200);
  });

  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = "{\"wifi\":\"" + String(WiFi.status()==WL_CONNECTED?"Connected":"Disconnected") +
                  "\",\"led\":" + String(ledState?"true":"false") + "}";
    request->send(200, "application/json", json);
  });

  server.begin();
}

void loop() {
  if (!apMode) connectWifiCheck();
}

void connectWiFi() {
  WiFi.begin(ssid.c_str(), password.c_str());
  unsigned long start = millis();

  while (WiFi.status() != WL_CONNECTED && millis()-start < 15000) {
    delay(500);
  }

  if (WiFi.status() == WL_CONNECTED) {
    apMode = false;
    if (MDNS.begin("dash")) {
      Serial.println("mDNS aktif: http://dash.local");
    }
  } else {
    startAPMode();
  }
}

void startAPMode() {
  apMode = true;
  WiFi.softAP("ESP32-Config");
  Serial.println("AP Mode aktif: ESP32-Config");
}

void connectWifiCheck() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    if (WiFi.status() != WL_CONNECTED) connectWiFi();
  }
}
```

## 4. **ALUR KERJA SISTEM**

```
Power On ESP32
       ↓
Baca Preferences (SSID/Password)
       ↓
Ada SSID? ──No──→ Start AP Mode
   ↓ Yes                  ↓
Connect WiFi         User Connect to AP
   ↓                    (192.168.4.1)
Berhasil? ──No──→ Config via Web
   ↓ Yes                  ↓
Start Web Server     Save Credentials
       ↓                    ↓
Start mDNS              Reboot/Reconnect
(dash.local)                  ↓
       ←──────────────────────┘
       ↓
Monitor WiFi Status (every 5s)
       ↓
Handle Web Requests
```
