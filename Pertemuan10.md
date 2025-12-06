# **Membuat Web Control LED ESP32 (HTML + CSS + JS + WebSocket)**

## Tujuan Materi

Setelah selesai, kamu bisa:

* Mengontrol LED ESP32 lewat browser
* Mengirim data dari browser → ESP32
* Menerima data dari ESP32 → browser
* Membuat halaman web sederhana
* Menghubungkan HTML dengan ESP32 menggunakan WebSocket

---

# **PART 1 - Konsep Dasar**

Kita akan membuat sistem seperti ini:

```
Browser (HTML + JS)
        ↓ WebSocket
     ESP32
        ↓ LED
        ↓ Counter
        ↑
  Kirim data balik ke browser
```

Jadi alurnya:

1. User klik tombol **ON/OFF** → browser kirim data
2. ESP32 membaca → menyalakan/mematikan LED
3. ESP32 setiap detik mengirim **counter**
4. Browser menampilkan counter

---

# **PART 2 - Siapkan Perangkat**

### Alat:

* ESP32
* Kabel USB
* LED + resistor (opsional, bisa pakai LED built-in GPIO 2)

---

# **PART 3 - Struktur Program**

Program dibagi menjadi:

### 1. WebServer → Menyajikan halaman HTML

### 2. WebSocket → Pengiriman data realtime

### 3. LED → Dikontrol

### 4. Counter → Disimulasi sensor

---

# **PART 4 - Kode ESP32 (Langkah demi langkah)**

---

## **STEP 4.1 — Import library**

```cpp
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <WebSocketsServer.h>
```

> Ini library untuk web server dan WebSocket.

---

## **STEP 4.2 — Setup LED dan server**

```cpp
#define LED_PIN 2

AsyncWebServer server(80);
WebSocketsServer websockets(81);

int counter = 0;

const char* ssid = "ESP32";
const char* password = "12345678";
```

---

## **STEP 4.3 — HTML sederhana**

Kita sisipkan HTML langsung di program ESP32.

```cpp
const char webpage[] PROGMEM = R"====(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>LED Control</title>

<style>
    body { font-family: Arial; text-align: center; margin-top: 40px; }
    button { padding: 12px 25px; font-size: 18px; margin: 10px; cursor: pointer; }
    .on { background: green; color: white; }
    .off { background: red; color: white; }
    #sensor { font-size: 24px; margin-top: 20px; }
</style>

</head>
<body>

<h2>LED Control ESP32</h2>

<button id="btnOn" class="on">LED ON</button>
<button id="btnOff" class="off">LED OFF</button>

<div id="sensor">Sensor: 0</div>

<script>
    var ws = new WebSocket("ws://" + location.hostname + ":81/");

    let ledStatus = 0;

    function sendData() {
        let data = { led: ledStatus };
        ws.send(JSON.stringify(data));
    }

    document.getElementById("btnOn").onclick = function() {
        ledStatus = 1;
        sendData();
    }

    document.getElementById("btnOff").onclick = function() {
        ledStatus = 0;
        sendData();
    }

    ws.onmessage = function(event) {
        let obj = JSON.parse(event.data);
        document.getElementById("sensor").innerText = "Sensor: " + obj.counter;
    }

</script>

</body>
</html>
)=====";
```

---

# **PART 5 - WebSocket Handler**

```cpp
void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length)
{
    if (type == WStype_TEXT)
    {
        String msg = String((char*)payload);

        if (msg.indexOf("led") >= 0) {
            if (msg.indexOf("1") >= 0) {
                digitalWrite(LED_PIN, HIGH);
            } else {
                digitalWrite(LED_PIN, LOW);
            }
        }
    }
}
```

---

# **PART 6 - Setup()**

```cpp
void setup()
{
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);

    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(300);
        Serial.print(".");
    }

    Serial.println("\nWiFi Connected!");

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", webpage);
    });

    server.begin();

    websockets.begin();
    websockets.onEvent(webSocketEvent);
}
```

---

# **PART 7 - Loop()**

ESP32 akan mengirim counter setiap detik.

```cpp
void loop()
{
    websockets.loop();

    counter++;

    String json = "{\"counter\": " + String(counter) + "}";

    websockets.broadcastTXT(json);

    delay(1000);
}
```

---

# **PART 8 - Jalankan**

1. Upload program ke ESP32
2. Buka Serial Monitor
3. Lihat IP Address
4. Buka browser → `http://IP-ADDRESS/`

---

# **PART 9 - Hasil**

Halaman web akan menampilkan:

```
LED Control ESP32

[ LED ON ]   [ LED OFF ]

Sensor: 12
```

* Klik ON → LED menyala
* Klik OFF → LED mati
* Counter berjalan seperti data sensor
