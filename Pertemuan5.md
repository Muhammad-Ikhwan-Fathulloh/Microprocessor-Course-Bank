# **Materi Lengkap: PWM (Pulse Width Modulation) pada Arduino dan ESP32**

---

## **1. Pengantar PWM**

**PWM (Pulse Width Modulation)** adalah teknik pengendalian daya dengan cara **mengubah lebar pulsa sinyal digital**.
PWM tidak menghasilkan sinyal analog secara langsung, melainkan **mengatur proporsi waktu sinyal HIGH dan LOW** dalam satu periode waktu.

Dengan kata lain, PWM mengatur **berapa lama sinyal aktif (ON)** dibandingkan dengan **waktu total satu siklus**.

PWM digunakan di banyak perangkat elektronik seperti:

* Pengaturan **kecerahan LED**
* Pengaturan **kecepatan motor DC**
* **Kontrol servo**
* **Audio signal / buzzer**
* **Efisiensi daya sistem embedded**

---

## **2. Konsep Dasar PWM**

PWM memiliki tiga parameter penting:

| Parameter          | Deskripsi                                                   |
| ------------------ | ----------------------------------------------------------- |
| **Frekuensi (Hz)** | Jumlah pulsa per detik.                                     |
| **Periode (T)**    | Waktu untuk satu siklus lengkap HIGH + LOW, dengan T = 1/f. |
| **Duty Cycle (%)** | Persentase waktu sinyal HIGH terhadap total periode.        |

---

### **3. Representasi Gelombang PWM**

```
Duty Cycle 25%   ___----___----___----___
Duty Cycle 50%   ________--------________
Duty Cycle 75%   ____________----________
```

* Semakin besar **Duty Cycle**, semakin lama sinyal HIGH → daya rata-rata meningkat.
* Semakin kecil **Duty Cycle**, semakin singkat sinyal HIGH → daya rata-rata menurun.

---

## **4. Rumus Tegangan Rata-Rata PWM**

Secara matematis, tegangan rata-rata yang dihasilkan PWM adalah:

[
V_{avg} = DutyCycle \times V_{max}
]

Keterangan:

* ( V_{avg} ): Tegangan rata-rata ke beban
* ( DutyCycle ): Nilai antara 0.0–1.0 (misal 50% = 0.5)
* ( V_{max} ): Tegangan HIGH (misal 5V pada Arduino, 3.3V pada ESP32)

**Contoh:**

Jika Arduino mengeluarkan 5V dan duty cycle = 60%, maka:

[
V_{avg} = 0.6 \times 5V = 3V
]

Artinya, LED akan menyala **setara dengan** diberi tegangan konstan 3V.

---

## **5. Ilustrasi Visual PWM**

### **Gambar: Gelombang PWM dan Duty Cycle**

![PWM Illustration](https://www.etechnophiles.com/wp-content/uploads/2021/03/pwm-graph.jpg)

Keterangan:

* Semakin besar duty cycle, semakin tinggi daya rata-rata.
* Periode tetap, tetapi durasi HIGH berubah.

---

## **6. Implementasi PWM pada Arduino**

Arduino menggunakan **fungsi `analogWrite()`** untuk menghasilkan sinyal PWM.

> Nilai PWM di Arduino: **0–255**
> (0 = 0%, 255 = 100%)

---

### **Contoh 1: Mengatur Kecerahan LED**

```cpp
int ledPin = 9; // pin PWM

void setup() {
  pinMode(ledPin, OUTPUT);
}

void loop() {
  for (int brightness = 0; brightness <= 255; brightness++) {
    analogWrite(ledPin, brightness); // Naikkan duty cycle
    delay(10);
  }

  for (int brightness = 255; brightness >= 0; brightness--) {
    analogWrite(ledPin, brightness); // Turunkan duty cycle
    delay(10);
  }
}
```

**Penjelasan:**

* `analogWrite()` menghasilkan PWM dengan nilai 0–255.
* LED tampak semakin terang seiring naiknya duty cycle.

---

### **Contoh 2: PWM Mengatur Kecepatan Motor DC**

```cpp
int motorPin = 5;

void setup() {
  pinMode(motorPin, OUTPUT);
}

void loop() {
  for (int speed = 0; speed <= 255; speed += 50) {
    analogWrite(motorPin, speed);
    delay(500);
  }
}
```

---

## **7. Implementasi PWM pada ESP32**

ESP32 menggunakan **PWM berbasis hardware** yang disebut **LEDC (LED Control)**.
PWM pada ESP32 jauh lebih fleksibel karena bisa mengatur **frekuensi, resolusi, dan channel** secara terpisah.

---

### **Contoh 1: PWM LED dengan ESP32**

```cpp
#define LED_PIN 2
#define PWM_CHANNEL 0
#define PWM_FREQ 5000
#define PWM_RESOLUTION 8 // 8-bit = 0–255

void setup() {
  ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(LED_PIN, PWM_CHANNEL);
}

void loop() {
  for (int duty = 0; duty <= 255; duty++) {
    ledcWrite(PWM_CHANNEL, duty);
    delay(10);
  }
  for (int duty = 255; duty >= 0; duty--) {
    ledcWrite(PWM_CHANNEL, duty);
    delay(10);
  }
}
```

**Penjelasan:**

* `ledcSetup(channel, freq, resolution)` → konfigurasi PWM.
* `ledcAttachPin(pin, channel)` → sambungkan pin ke channel PWM.
* `ledcWrite(channel, duty)` → ubah duty cycle dinamis.

---

### **Contoh 2: PWM + Button (Interaktif)**

```cpp
#define LED_PIN 2
#define BUTTON_PIN 4
#define PWM_CHANNEL 0
#define PWM_FREQ 5000
#define PWM_RESOLUTION 8

int brightness = 0;

void setup() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
  ledcAttachPin(LED_PIN, PWM_CHANNEL);
}

void loop() {
  if (digitalRead(BUTTON_PIN) == LOW) {
    brightness += 50;
    if (brightness > 255) brightness = 0;
    ledcWrite(PWM_CHANNEL, brightness);
    delay(300);
  }
}
```

**Penjelasan:**

* Tiap kali tombol ditekan, LED menjadi lebih terang.
* PWM membuat perubahan kecerahan tampak halus.

---

## **8. Perbandingan PWM Arduino vs ESP32**

| Aspek             | Arduino UNO            | ESP32                         |
| ----------------- | ---------------------- | ----------------------------- |
| Pin PWM           | 6 pin (~3,5,6,9,10,11) | 16 channel                    |
| Frekuensi Default | ~490 Hz                | Dapat diatur hingga 40 MHz    |
| Resolusi          | 8-bit (0–255)          | Hingga 16-bit (0–65535)       |
| Fungsi Utama      | `analogWrite()`        | `ledcSetup()` & `ledcWrite()` |
| Presisi           | Standar                | Sangat tinggi                 |
| Fleksibilitas     | Rendah                 | Tinggi (per channel berbeda)  |

---

## **9. Hubungan PWM dengan Tegangan dan Energi**

PWM bekerja karena **rata-rata energi yang diterima beban** sebanding dengan **duty cycle**.

| Duty Cycle | Tegangan Rata-Rata | Dampak pada LED | Dampak pada Motor |
| ---------- | ------------------ | --------------- | ----------------- |
| 0%         | 0V                 | Mati            | Mati              |
| 25%        | 1.25V (dari 5V)    | Redup           | Lambat            |
| 50%        | 2.5V               | Sedang          | Normal            |
| 75%        | 3.75V              | Terang          | Cepat             |
| 100%       | 5V                 | Sangat Terang   | Maksimum          |

---

## **10. Hal yang Perlu Diperhatikan**

| Faktor                        | Penjelasan                                               |
| ----------------------------- | -------------------------------------------------------- |
| **Frekuensi terlalu rendah**  | LED bisa berkedip terlihat mata (flicker).               |
| **Duty cycle terlalu tinggi** | Daya dan suhu meningkat, hati-hati untuk beban besar.    |
| **Power supply**              | Pastikan arus mencukupi untuk motor/servo.               |
| **Channel overlap (ESP32)**   | Jangan gunakan channel PWM yang sama untuk beberapa pin. |

---

## **11. Implementasi Pointer + PWM (Gabungan Konsep)**

Kita bisa membuat sistem dinamis menggunakan pointer.

```cpp
int ledPin = 9;
int *led = &ledPin;
int brightness = 0;

void setup() {
  pinMode(*led, OUTPUT);
}

void loop() {
  analogWrite(*led, brightness);
  brightness += 25;
  if (brightness > 255) brightness = 0;
  delay(300);
}
```

Versi **ESP32**:

```cpp
int ledPin = 2;
int *led = &ledPin;
int brightness = 0;

void setup() {
  ledcSetup(0, 5000, 8);
  ledcAttachPin(*led, 0);
}

void loop() {
  ledcWrite(0, brightness);
  brightness += 25;
  if (brightness > 255) brightness = 0;
  delay(300);
}
```

---

## **12. Diagram Hubungan PWM dengan Output Tegangan**

![PWM Average Voltage](https://image.slidesharecdn.com/pulsewidthmodulation-140911010945-phpapp02/75/Pulse-width-modulation-1-2048.jpg)

**Keterangan:**

* Warna biru: sinyal PWM.
* Warna merah: tegangan rata-rata ( V_{avg} ) yang diterima oleh beban.

---

## **13. Kesimpulan**

PWM (Pulse Width Modulation) memungkinkan kontrol **daya, tegangan rata-rata, dan efisiensi energi** hanya dengan sinyal digital.

**Arduino**:

* Mudah digunakan dengan `analogWrite()`.
* Cocok untuk aplikasi sederhana seperti LED dan motor kecil.

**ESP32**:

* Menawarkan **resolusi tinggi, frekuensi variabel, dan multi-channel**.
* Cocok untuk aplikasi **AIoT, kontrol motor presisi, atau LED RGB**.

Apakah kamu ingin saya bantu tambahkan **bagian lanjutan “PWM Lanjutan (Servo, RGB LED, dan kontrol kecepatan motor dengan feedback sensor)”** agar bisa dijadikan **modul kelas tingkat menengah (IoT/Robotik)**?
