# Materi: Interupsi pada Arduino dan ESP32 (Button dan LED)

---

## 1. Pengantar

Dalam pemrograman mikrokontroler, program utama biasanya berjalan berulang-ulang di dalam fungsi `loop()`.
Namun, terkadang kita membutuhkan cara agar mikrokontroler **segera merespons** suatu kejadian, misalnya **ketika tombol ditekan**, **sensor berubah**, atau **sinyal datang dari luar**.

Untuk itulah digunakan **interupsi (interrupt)**.

Interupsi memungkinkan mikrokontroler untuk:

* Menghentikan sementara program utama.
* Menjalankan **fungsi khusus** yang disebut **ISR (Interrupt Service Routine)**.
* Kembali melanjutkan program utama setelah ISR selesai.

---

## 2. Konsep Dasar

Tanpa interupsi, kita biasanya menggunakan **pembacaan tombol secara terus-menerus**:

```cpp
if (digitalRead(buttonPin) == LOW) {
  // Aksi jika tombol ditekan
}
```

Cara ini disebut **polling**, dan bisa menyebabkan respon lambat jika program utama sibuk.

Dengan interupsi, mikrokontroler **langsung tahu** saat tombol ditekan — bahkan jika program utama sedang melakukan hal lain.

---

## 3. Prinsip Kerja Interupsi

1. Program utama berjalan normal di dalam `loop()`.
2. Saat terjadi sinyal (misalnya tombol ditekan), CPU menghentikan sementara `loop()`.
3. CPU menjalankan **ISR** (fungsi penangan interupsi).
4. Setelah ISR selesai, CPU melanjutkan `loop()`.

---

## 4. Struktur Dasar Interupsi

Format umum pada Arduino dan ESP32:

```cpp
attachInterrupt(digitalPinToInterrupt(pin), nama_ISR, mode);
```

**Keterangan:**

* `pin` → Pin yang digunakan untuk mendeteksi sinyal.
* `nama_ISR` → Nama fungsi yang akan dijalankan saat interupsi terjadi.
* `mode` → Kondisi pemicu interupsi:

  * `RISING`  → Saat sinyal naik dari LOW ke HIGH
  * `FALLING` → Saat sinyal turun dari HIGH ke LOW
  * `CHANGE`  → Saat sinyal berubah (naik atau turun)

---

## 5. Praktik 1: Interupsi pada Arduino UNO

### Alat dan Bahan

* 1x Arduino UNO
* 1x Tombol push button
* 1x LED
* 1x Resistor 220Ω (untuk LED)
* 1x Resistor 10kΩ (untuk pull-down tombol)
* Kabel jumper dan breadboard

### Rangkaian

* Tombol dihubungkan ke pin **2** dan **GND**.
* LED dihubungkan ke pin **13** dan resistor ke **GND**.

```
Pin 2 -----> Tombol -----> GND  
Pin 13 -----> LED -----> Resistor -----> GND
```

### Program

```cpp
int ledPin = 13;
int buttonPin = 2;
volatile bool ledState = false;

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP); 
  attachInterrupt(digitalPinToInterrupt(buttonPin), toggleLED, FALLING);
}

void loop() {
  // Program utama tetap berjalan
  delay(100);
}

void toggleLED() {
  ledState = !ledState;
  digitalWrite(ledPin, ledState);
}
```

### Penjelasan

* `attachInterrupt()` digunakan untuk menghubungkan pin 2 dengan fungsi `toggleLED`.
* ISR `toggleLED()` dijalankan **saat tombol ditekan** (FALLING edge).
* Variabel `ledState` bertipe `volatile` karena diakses dari ISR dan loop utama.
* ISR tidak boleh memakai `delay()` atau `Serial.print()`.

### Hasil

Setiap kali tombol ditekan, LED menyala atau mati **tanpa menunggu program utama** selesai.

---

## 6. Praktik 2: Interupsi pada ESP32

### Alat dan Bahan

* 1x ESP32 Dev Board
* 1x Tombol push button
* 1x LED
* 2x Resistor (220Ω untuk LED, 10kΩ untuk tombol)
* Breadboard dan kabel jumper

### Rangkaian

* Tombol ke **pin 14** dan **GND**
* LED ke **pin 23** dan resistor ke **GND**

```
Pin 14 -----> Tombol -----> GND  
Pin 23 -----> LED -----> Resistor -----> GND
```

### Program

```cpp
const int ledPin = 23;
const int buttonPin = 14;
volatile bool ledState = false;

void IRAM_ATTR toggleLED() {
  ledState = !ledState;
  digitalWrite(ledPin, ledState);
}

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(buttonPin), toggleLED, FALLING);
}

void loop() {
  delay(100);
}
```

### Penjelasan

* Hampir sama seperti Arduino, tetapi fungsi ISR menggunakan atribut `IRAM_ATTR` agar disimpan di RAM (lebih cepat).
* Mode `FALLING` digunakan karena tombol aktif rendah (dari HIGH → LOW).
* Fungsi ISR hanya membalikkan status LED.

---

## 7. Praktik 3: Menangani “Bounce” pada Tombol

Masalah umum saat menggunakan tombol adalah **noise** atau **bouncing** sinyal tombol bisa berubah beberapa kali dalam milidetik, sehingga interupsi bisa terpanggil berkali-kali.

### Solusi: Tambahkan penundaan singkat (debounce) menggunakan waktu millis()

Contoh untuk ESP32 atau Arduino:

```cpp
const int buttonPin = 2;
const int ledPin = 13;
volatile bool ledState = false;
volatile unsigned long lastInterruptTime = 0;

void IRAM_ATTR handleInterrupt() {
  unsigned long currentTime = millis();
  if (currentTime - lastInterruptTime > 200) { // Debounce 200 ms
    ledState = !ledState;
    digitalWrite(ledPin, ledState);
    lastInterruptTime = currentTime;
  }
}

void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(buttonPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(buttonPin), handleInterrupt, FALLING);
}

void loop() {
  delay(100);
}
```

### Penjelasan

* Fungsi `millis()` digunakan untuk mencatat waktu terakhir interupsi.
* Jika tombol ditekan terlalu cepat (<200ms), maka interupsi diabaikan.
* Ini membantu mencegah LED menyala-mati terlalu cepat akibat bouncing.

---

## 8. Perbandingan Arduino vs ESP32

| Aspek                | Arduino UNO             | ESP32                              |
| -------------------- | ----------------------- | ---------------------------------- |
| Jumlah Pin Interupsi | 2 (pin 2 & 3)           | Hampir semua pin                   |
| Mode Pemicu          | RISING, FALLING, CHANGE | RISING, FALLING, CHANGE, HIGH, LOW |
| Kecepatan            | 16 MHz                  | 240 MHz                            |
| ISR di RAM           | Tidak perlu             | Perlu atribut `IRAM_ATTR`          |
| Multi-Core           | Tidak                   | Ya (Dual Core)                     |

---

## 9. Latihan

1. Buat program agar LED menyala saat tombol ditekan, lalu mati otomatis setelah 3 detik menggunakan interupsi.
2. Ubah ISR agar hanya mengatur **flag** (misalnya `buttonPressed = true`) dan proses logika diatur di `loop()`.
3. Coba gunakan **pin lain** untuk interupsi pada ESP32 (misalnya GPIO 26 atau 27).

---

## 10. Kesimpulan

* Interupsi digunakan untuk menangani kejadian yang harus segera ditanggapi tanpa menunggu loop utama.
* Fungsi `attachInterrupt()` digunakan untuk mendefinisikan pin dan mode pemicu.
* Fungsi ISR harus **ringan dan cepat**.
* Variabel global yang digunakan dalam ISR harus dideklarasikan sebagai `volatile`.
* Pada ESP32, tambahkan `IRAM_ATTR` untuk memastikan ISR berjalan cepat dari RAM.
* Teknik **debounce** penting untuk menghindari interupsi berulang akibat noise tombol.
