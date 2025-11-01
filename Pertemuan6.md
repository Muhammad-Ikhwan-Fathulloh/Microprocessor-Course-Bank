# Materi: Implementasi JSON dengan LED dan Button

## Menggunakan Library ArduinoJson & Arduino_Json

---

## 1. Tujuan Pembelajaran

Peserta memahami cara:

1. Menggunakan **ArduinoJson** dan **Arduino_Json** untuk membuat serta membaca data JSON.
2. Mengendalikan **LED** dan **tombol (button)** menggunakan data JSON.
3. Mengetahui **perbedaan teknis dan performa** antara kedua library tersebut.

---

## 2. Alat dan Bahan

* Board: Arduino UNO atau ESP32
* 1 buah LED
* 1 buah resistor 220Ω
* 1 buah push button
* Breadboard dan kabel jumper

**Rangkaian:**

* LED → Pin 5
* Button → Pin 13 (gunakan internal pull-up)

---

## 3. Sekilas tentang JSON

**JSON (JavaScript Object Notation)** adalah format pertukaran data berbasis teks yang umum digunakan pada komunikasi antarperangkat IoT dan server.

Contoh:

```json
{
  "device": "ESP32",
  "temperature": 27.5,
  "led_state": "ON"
}
```

Arduino tidak memiliki dukungan JSON bawaan, sehingga digunakan dua library populer:

1. `ArduinoJson` (oleh Benoit Blanchon)
2. `Arduino_Json` (oleh Tim Resmi Arduino)

---

## 4. Perbandingan Teknis: ArduinoJson vs Arduino_Json

| Aspek                  | ArduinoJson                                   | Arduino_Json                           |
| ---------------------- | --------------------------------------------- | -------------------------------------- |
| Pengembang             | Benoit Blanchon                               | Tim resmi Arduino                      |
| Versi utama            | 6.x (stabil dan matang)                       | 0.2.x (ringan dan sederhana)           |
| Kompatibilitas         | Arduino, ESP8266, ESP32, STM32, dsb.          | Semua board Arduino                    |
| Sintaks                | Mirip C++ (Structured Document)               | Mirip JavaScript (JSONVar)             |
| Pembuatan JSON         | `serializeJson()`                             | `JSON.stringify()`                     |
| Parsing JSON           | `deserializeJson()`                           | `JSON.parse()`                         |
| Dukungan Nested Object | Ya (multi-level JSON)                         | Terbatas (1–2 level)                   |
| Penggunaan Memori      | Lebih besar, tapi efisien untuk data kompleks | Lebih kecil, cocok untuk proyek ringan |
| Fitur Validasi Error   | Ada (DeserializationError)                    | Tidak ada, hanya undefined check       |
| Kinerja Parsing        | Cepat untuk data besar                        | Cukup untuk data kecil                 |
| Dokumentasi            | Lengkap dan profesional                       | Sederhana dan ringkas                  |
| Cocok untuk            | Proyek IoT kompleks (API, MQTT)               | Pembelajaran dasar, proyek kecil       |

**Kesimpulan:**

* Gunakan **ArduinoJson** jika proyek menggunakan data JSON besar, nested object, atau butuh efisiensi parsing tinggi.
* Gunakan **Arduino_Json** untuk proyek ringan atau pembelajaran dasar.

---

## 5. Implementasi ArduinoJson (ESP32 / UNO)

### 5.1. Tujuan

* Membaca tombol.
* Mengubah status LED setiap kali tombol ditekan.
* Mengirim status LED dan tombol dalam format JSON.

### 5.2. Kode Program

```cpp
#include <ArduinoJson.h>

const int ledPin = 5;
const int buttonPin = 13;

bool ledState = false;
bool lastButtonState = HIGH;

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
}

void loop() {
  bool buttonState = digitalRead(buttonPin);

  // Jika tombol ditekan (tepi turun)
  if (buttonState == LOW && lastButtonState == HIGH) {
    ledState = !ledState;
    digitalWrite(ledPin, ledState);
    kirimJSON();
    delay(200); // debounce
  }

  lastButtonState = buttonState;
}

void kirimJSON() {
  StaticJsonDocument<200> doc;
  doc["device"] = "ESP32";
  doc["led_state"] = ledState ? "ON" : "OFF";
  doc["button_pressed"] = true;

  String jsonString;
  serializeJson(doc, jsonString);
  Serial.println(jsonString);
}
```

### 5.3. Output Serial

```json
{"device":"ESP32","led_state":"ON","button_pressed":true}
{"device":"ESP32","led_state":"OFF","button_pressed":true}
```

---

## 6. Implementasi Arduino_Json (Arduino UNO)

### 6.1. Tujuan

* Melakukan fungsi yang sama, tetapi menggunakan library **Arduino_Json**.
* Menghasilkan output JSON sederhana.

### 6.2. Kode Program

```cpp
#include <Arduino_JSON.h>

const int ledPin = 5;
const int buttonPin = 13;

bool ledState = false;
bool lastButtonState = HIGH;

void setup() {
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
}

void loop() {
  bool buttonState = digitalRead(buttonPin);

  if (buttonState == LOW && lastButtonState == HIGH) {
    ledState = !ledState;
    digitalWrite(ledPin, ledState);
    kirimJSON();
    delay(200); // debounce
  }

  lastButtonState = buttonState;
}

void kirimJSON() {
  JSONVar data;
  data["device"] = "Arduino UNO";
  data["led_state"] = ledState ? "ON" : "OFF";
  data["button_pressed"] = true;

  String jsonString = JSON.stringify(data);
  Serial.println(jsonString);
}
```

### 6.3. Output Serial

```json
{"device":"Arduino UNO","led_state":"ON","button_pressed":true}
{"device":"Arduino UNO","led_state":"OFF","button_pressed":true}
```

---

## 7. Analisis Perbedaan pada Implementasi

| Aspek                    | ArduinoJson                    | Arduino_Json                              |
| ------------------------ | ------------------------------ | ----------------------------------------- |
| Cara deklarasi JSON      | `StaticJsonDocument<200> doc;` | `JSONVar data;`                           |
| Penambahan data          | `doc["key"] = value;`          | `data["key"] = value;`                    |
| Konversi ke string       | `serializeJson(doc, output);`  | `JSON.stringify(data);`                   |
| Penanganan error parsing | `DeserializationError`         | `if (JSON.typeof(parsed) == "undefined")` |
| Output JSON              | Format rapi dan efisien        | Sederhana                                 |
| Penggunaan memori        | Sedikit lebih besar            | Lebih ringan                              |
| Kompatibilitas           | Lebih luas, termasuk ESP       | Semua board Arduino                       |

---

## 8. Latihan Pengembangan

1. Tambahkan **status waktu (timestamp)** ke dalam JSON.
2. Kirimkan data JSON setiap 2 detik menggunakan **millis()** tanpa `delay()`.
3. Tambahkan **parsing JSON input** agar LED bisa dikontrol dari Serial, contoh perintah:

   ```json
   {"command":"LED_ON"}
   ```
4. Bandingkan ukuran file hasil kompilasi untuk kedua library.

---

## 9. Kesimpulan

* **ArduinoJson** lebih kuat dan fleksibel, ideal untuk komunikasi data kompleks pada ESP32/ESP8266.
* **Arduino_Json** lebih ringan dan sederhana, cocok untuk board kecil seperti Arduino UNO atau Nano.
* Keduanya dapat digunakan untuk mengirim status perangkat dalam format JSON, tergantung kebutuhan proyek dan sumber daya board.
