# Materi Lanjutan: Implementasi JSON, millis(), Interrupt, Pointer, dan PWM di Arduino / ESP32

---

## 1. Tujuan Pembelajaran

Peserta memahami cara:

1. Menggunakan **millis()** untuk membuat sistem non-blocking (tanpa `delay()`).
2. Menggunakan **interrupt** untuk mendeteksi tombol secara cepat dan efisien.
3. Menggunakan **pointer** untuk manipulasi variabel LED secara dinamis.
4. Mengatur **intensitas LED (PWM)** dan mengirim data dalam format JSON.
5. Menggabungkan semuanya dalam satu sistem terintegrasi.

---

## 2. Konsep Dasar yang Diterapkan

### 2.1. millis()

`millis()` memberikan waktu (dalam milidetik) sejak board mulai menyala.
Digunakan untuk menggantikan `delay()` agar program tetap responsif.

Contoh:

```cpp
if (millis() - lastTime > 1000) {
  // jalankan aksi setiap 1 detik
  lastTime = millis();
}
```

---

### 2.2. Interrupt

Interrupt memungkinkan board **merespons peristiwa secara instan**, tanpa menunggu loop selesai.
Biasanya digunakan untuk tombol, sensor, atau input cepat.

Contoh:

```cpp
attachInterrupt(digitalPinToInterrupt(pinButton), handleButton, FALLING);
```

---

### 2.3. Pointer

Pointer menyimpan **alamat memori variabel lain**.
Berguna saat kita ingin mengubah variabel dari fungsi berbeda tanpa return value.

Contoh:

```cpp
int ledValue = 128;
int *ptr = &ledValue;
*ptr = 200; // mengubah nilai langsung lewat pointer
```

---

### 2.4. PWM (Pulse Width Modulation)

PWM mengatur **kecerahan LED** dengan mengubah siklus hidup (duty cycle) sinyal digital.
Semakin tinggi duty cycle → semakin terang LED.

Pada ESP32 bisa menggunakan `ledcWrite()`, sedangkan Arduino UNO menggunakan `analogWrite()`.

---

## 3. Skema Implementasi

Kita akan:

* Menyalakan LED dengan **PWM** (brightness variabel).
* Menggunakan **interrupt** untuk membaca tombol.
* Menggunakan **millis()** untuk mengirim data JSON tiap 2 detik.
* Menggunakan **pointer** untuk memanipulasi intensitas LED.
* Mengirim data JSON berisi status LED, brightness, dan waktu.

---

## 4. Kode Program (ESP32 / ArduinoJson)

```cpp
#include <ArduinoJson.h>

const int ledPin = 5;
const int buttonPin = 13;

volatile bool buttonPressed = false;
int brightness = 0;
unsigned long lastTime = 0;
int step = 25;

// Pointer ke variabel brightness
int *ptrBrightness = &brightness;

void IRAM_ATTR handleButton() {
  buttonPressed = true;
}

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(buttonPin), handleButton, FALLING);

#if defined(ESP32)
  ledcAttachPin(ledPin, 0);
  ledcSetup(0, 5000, 8);
#endif
}

void loop() {
  unsigned long currentMillis = millis();

  // Tombol ditekan: ubah brightness
  if (buttonPressed) {
    buttonPressed = false;

    *ptrBrightness += step;
    if (*ptrBrightness > 255) *ptrBrightness = 0;

#if defined(ESP32)
    ledcWrite(0, *ptrBrightness);
#else
    analogWrite(ledPin, *ptrBrightness);
#endif
  }

  // Kirim data JSON setiap 2 detik
  if (currentMillis - lastTime >= 2000) {
    lastTime = currentMillis;
    kirimJSON(currentMillis);
  }
}

void kirimJSON(unsigned long waktu) {
  StaticJsonDocument<200> doc;
  doc["device"] = "ESP32";
  doc["brightness"] = *ptrBrightness;
  doc["led_state"] = (*ptrBrightness > 0) ? "ON" : "OFF";
  doc["timestamp"] = waktu;

  String jsonString;
  serializeJson(doc, jsonString);
  Serial.println(jsonString);
}
```

---

### Output Serial

Contoh hasil di Serial Monitor:

```json
{"device":"ESP32","brightness":0,"led_state":"OFF","timestamp":1234}
{"device":"ESP32","brightness":75,"led_state":"ON","timestamp":3245}
{"device":"ESP32","brightness":150,"led_state":"ON","timestamp":5246}
{"device":"ESP32","brightness":225,"led_state":"ON","timestamp":7247}
{"device":"ESP32","brightness":0,"led_state":"OFF","timestamp":9248}
```

---

## 5. Penjelasan Program

| Bagian                       | Fungsi                                                                          |
| ---------------------------- | ------------------------------------------------------------------------------- |
| `attachInterrupt()`          | Menjalankan fungsi `handleButton()` setiap tombol ditekan.                      |
| `volatile bool`              | Menandakan variabel bisa berubah di luar kontrol loop utama (karena interrupt). |
| `millis()`                   | Mengatur waktu pengiriman data tanpa menghentikan program.                      |
| `pointer` (`*ptrBrightness`) | Mengubah brightness secara dinamis di luar fungsi utama.                        |
| `PWM`                        | Mengatur intensitas LED dari 0–255.                                             |
| `ArduinoJson`                | Mengemas data LED menjadi format JSON efisien.                                  |

---

## 6. Alternatif Arduino UNO (Arduino_Json)

Jika menggunakan UNO, ganti bagian JSON seperti berikut:

```cpp
#include <Arduino_JSON.h>

void kirimJSON(unsigned long waktu) {
  JSONVar data;
  data["device"] = "Arduino UNO";
  data["brightness"] = *ptrBrightness;
  data["led_state"] = (*ptrBrightness > 0) ? "ON" : "OFF";
  data["timestamp"] = waktu;

  String jsonString = JSON.stringify(data);
  Serial.println(jsonString);
}
```

---

## 7. Latihan Eksperimen

1. Tambahkan input serial:

   * Jika menerima `{"brightness":128}`, ubah PWM sesuai nilai JSON.
2. Simpan log JSON ke EEPROM setiap 10 detik.
3. Gunakan 2 tombol: satu untuk menaikkan, satu untuk menurunkan brightness.
4. Ganti PWM menjadi kendali servo (0–180 derajat) menggunakan `Servo.h`.
5. Integrasikan JSON ini ke komunikasi MQTT untuk IoT dashboard.

---

## 8. Kesimpulan

* **millis()** membuat sistem tetap responsif tanpa delay.
* **interrupt** memberikan respon cepat terhadap event eksternal.
* **pointer** berguna untuk mengelola variabel secara efisien di fungsi terpisah.
* **PWM** memungkinkan kontrol analog semu pada LED.
* **ArduinoJson** dan **Arduino_Json** dapat digunakan untuk mengirim status sistem dalam format standar IoT (JSON).

Apakah kamu ingin saya lanjutkan ke **versi integrasi IoT (MQTT + JSON + millis + PWM)** untuk dijadikan **materi lanjutan sesi 3**, agar bisa dikirim ke dashboard seperti Node-RED atau Blynk?
