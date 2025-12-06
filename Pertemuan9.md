# Materi ESP32: Koneksi WiFi dan Manajemen Koneksi

## Bagian 1: Persiapan

1. Pastikan ESP32 sudah terinstal di Arduino IDE.
2. Hubungkan ESP32 ke komputer melalui USB.
3. Pilih board dan port yang sesuai:

   * **Board:** DOIT ESP32 DEVKIT V1 (atau board kamu)
   * **Port:** COM yang sesuai

---

## Bagian 2: Koneksi WiFi Dasar (Station Mode)

ESP32 bisa terhubung ke WiFi sebagai **Station (STA)**, artinya ESP32 akan menjadi client di jaringan WiFi.

### Contoh Sketch Koneksi WiFi

```cpp
#include <WiFi.h>

const char* ssid = "Nama_WiFi";       // Ganti dengan SSID WiFi
const char* password = "Password_WiFi"; // Ganti dengan password WiFi

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("Mencoba koneksi ke WiFi...");
  WiFi.begin(ssid, password);

  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < 20) { // maksimal 20 percobaan
    delay(500);
    Serial.print(".");
    retry++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi Terhubung!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("");
    Serial.println("Gagal koneksi WiFi!");
  }
}

void loop() {
  // Bisa tambahkan kode lainnya
}
```

**Penjelasan:**

* `WiFi.begin(ssid, password)`: memulai koneksi ke WiFi.
* `WiFi.status()`: mengecek status koneksi (WL_CONNECTED = berhasil).
* `WiFi.localIP()`: menampilkan IP ESP32 setelah berhasil koneksi.
* `retry` dan `delay` digunakan untuk membatasi percobaan koneksi agar tidak infinite loop.

---

## Bagian 3: Manajemen Koneksi WiFi

Dalam proyek IoT, ESP32 kadang kehilangan koneksi. Jadi perlu manajemen koneksi otomatis.

### Contoh Manajemen Koneksi

```cpp
#include <WiFi.h>

const char* ssid = "Nama_WiFi";
const char* password = "Password_WiFi";

unsigned long previousMillis = 0;
const long interval = 10000; // cek koneksi setiap 10 detik

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
}

void loop() {
  unsigned long currentMillis = millis();

  // Cek setiap 10 detik
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi terputus, mencoba reconnect...");
      WiFi.disconnect();
      WiFi.begin(ssid, password);

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
```

**Penjelasan:**

* `millis()` digunakan untuk membuat timer tanpa delay blocking.
* Jika koneksi hilang, ESP32 akan otomatis `disconnect()` dan mencoba `WiFi.begin()` kembali.
* `retry` membatasi percobaan reconnect agar tidak infinite loop.

---

## Bagian 4: Tips Koneksi WiFi ESP32

1. Gunakan power supply stabil, karena WiFi memerlukan arus lebih besar.
2. Hindari delay panjang yang memblokir loop, gunakan `millis()` untuk timer.
3. Simpan kredensial WiFi menggunakan **Preferences** agar tidak hardcode di sketch.

### Contoh Menyimpan SSID & Password dengan Preferences

```cpp
#include <Preferences.h>
#include <WiFi.h>

Preferences prefs;

void setup() {
  Serial.begin(115200);
  prefs.begin("wifi-creds", false);

  String ssid = prefs.getString("ssid", "defaultSSID");
  String password = prefs.getString("password", "defaultPassword");

  WiFi.begin(ssid.c_str(), password.c_str());

  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < 20) {
    delay(500);
    Serial.print(".");
    retry++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi Terhubung!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("");
    Serial.println("Gagal koneksi WiFi!");
  }
}

void loop() {
}
```

* Dengan Preferences, pengguna bisa update SSID & password tanpa mengubah sketch.
