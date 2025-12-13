#include <Preferences.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>

AsyncWebServer server(80);
Preferences prefs;

String ssid = prefs.getString("ssid", "Hadid.Technos");
String password = prefs.getString("password", "DatinLembur");

unsigned long previousMillis = 0;
const long interval = 5000; // cek koneksi setiap 10 detik

const char webpage[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta http-equiv="X-UA-Compatible" content="IE=edge">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32</title>
</head>
<body>
    <h1>Hello ESP32</h1>
</body>
</html>
)=====";

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid.c_str(), password.c_str());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", webpage);
    });

  server.begin();
}

void loop() {
  connectWifi();
  // Lanjut Kode  
}

void connectWifi(){
  unsigned long currentMillis = millis();

  // Cek setiap 10 detik
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi terputus, mencoba reconnect...");
      WiFi.disconnect();
      WiFi.begin(ssid.c_str(), password.c_str());

      int retry = 0;
      while (WiFi.status() != WL_CONNECTED && retry < 10) {
        delay(500);
        Serial.print(".");
        retry++;
      }

      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("");
        Serial.println("WiFi kembali terhubung!");
        Serial.print("IP Address: ");
        Serial.println(WiFi.localIP());
      } else {
        Serial.println("");
        Serial.println("Gagal reconnect WiFi.");
      }
    } else {
      Serial.println("WiFi masih terhubung.");
    }
  }
}
