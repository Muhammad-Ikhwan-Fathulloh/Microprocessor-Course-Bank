# Materi Instalasi ESP32 dan EEPROM di Arduino IDE

## Bagian 1: Instalasi ESP32 di Arduino IDE

### Step 1: Pastikan Arduino IDE Terinstall

* Download Arduino IDE versi terbaru dari [arduino.cc](https://www.arduino.cc/en/software).
* Bisa pakai versi klasik (1.8.x) atau versi 2.0.
* Installing the ESP32 Board in Arduino IDE (Windows, Mac OS X, Linux) [randomnerdtutorials.com]([https://www.arduino.cc/en/software](https://randomnerdtutorials.com/installing-the-esp32-board-in-arduino-ide-windows-instructions/)).

### Step 2: Buka Preferences

* Buka Arduino IDE.
* Pilih menu **File > Preferences**.
* Akan muncul jendela Preferences.

### Step 3: Tambahkan Board URL

* Di field **“Additional Board Manager URLs”**, masukkan URL berikut:

```
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
```

* Jika sudah ada URL lain, pisahkan dengan koma.
* Klik **OK**.

### Step 4: Buka Boards Manager

* Pilih **Tools > Board > Boards Manager…**.
* Ketik “ESP32” pada search box.
* Pilih **“ESP32 by Espressif Systems”**, kemudian klik **Install**.
* Tunggu proses instalasi selesai.

### Step 5: Pilih Board

* Pilih board yang sesuai dengan papan ESP32 kamu:
  **Tools > Board > ESP32 Arduino > DOIT ESP32 DEVKIT V1** (atau board lain yang cocok).

### Step 6: Pilih Port

* Hubungkan ESP32 ke komputer via USB.
* Pilih port COM yang sesuai: **Tools > Port**.
* Jika tidak muncul, install driver USB-to-UART seperti **CP210x**.

### Step 7: Tes Instalasi

* Buka contoh sketch: **File > Examples > WiFi (ESP32) > WiFiScan**.
* Upload ke ESP32.
* Buka **Serial Monitor**: baud rate **115200**.
* Tekan tombol Enable jika diperlukan, dan cek hasil scan WiFi.

### Troubleshooting

* Error saat upload:

```
A fatal error occurred: Failed to connect to ESP32: Timed out
```

* Solusi: tahan tombol **BOOT**, klik **Upload**, lepaskan setelah muncul “Connecting…”.

---

## Bagian 2: ESP32 dan EEPROM

### Step 1: Apa itu EEPROM pada ESP32

* ESP32 tidak memiliki EEPROM fisik.
* Arduino-ESP32 menyediakan **EEPROM emulasi** di flash internal.
* Cocok untuk menyimpan data kecil non-volatile, misal pengaturan atau status sensor.

### Step 2: Instalasi Library EEPROM

* Library EEPROM sudah tersedia di Arduino IDE ESP32.
* Tidak perlu instal tambahan.

### Step 3: Contoh Penggunaan EEPROM

```cpp
#include <EEPROM.h>

#define EEPROM_SIZE 64  // jumlah byte

void setup() {
  Serial.begin(115200);
  EEPROM.begin(EEPROM_SIZE);

  // Menulis data
  EEPROM.write(0, 42);
  EEPROM.commit();

  // Membaca data
  byte val = EEPROM.read(0);
  Serial.print("Nilai di EEPROM[0] = ");
  Serial.println(val);
}

void loop() {
  // Kosong
}
```

### Step 4: Penjelasan

* `EEPROM.begin(size)`: inisialisasi buffer EEPROM.
* `EEPROM.write(address, value)`: menulis satu byte.
* `EEPROM.read(address)`: membaca byte.
* `EEPROM.commit()`: menyimpan ke flash.

### Step 5: Catatan

* Flash memiliki umur terbatas → hindari menulis berulang di alamat sama.
* Untuk data konfigurasi lebih fleksibel, gunakan **Preferences**.

---

## Bagian 3: ESP32 dan Preferences (Alternatif EEPROM)

### Step 1: Library Preferences

* Library Preferences sudah tersedia di Arduino IDE ESP32.
* Lebih fleksibel untuk menyimpan integer, string, boolean.

### Step 2: Contoh Penggunaan Preferences

```cpp
#include <Preferences.h>

Preferences prefs;

void setup() {
  Serial.begin(115200);
  prefs.begin("my-app", false); // namespace my-app, read/write

  // Menyimpan data
  prefs.putInt("bootCount", prefs.getInt("bootCount", 0) + 1);

  // Membaca data
  int bc = prefs.getInt("bootCount", 0);
  Serial.print("Boot count: ");
  Serial.println(bc);

  prefs.end();
}

void loop() {
}
```

### Step 3: Penjelasan

* `prefs.begin(namespace, false)`: memulai penyimpanan dengan namespace tertentu.
* `prefs.putInt(key, value)`: simpan integer.
* `prefs.getInt(key, default)`: baca integer.
* `prefs.end()`: menutup akses.

# Perbedaan EEPROM vs Preferences di ESP32

| Aspek                    | EEPROM                                                                                | Preferences                                                                                         |
| ------------------------ | ------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------- |
| **Definisi**             | Emulasi memori EEPROM menggunakan flash internal ESP32. Menyimpan data byte-per-byte. | Library ESP32 untuk penyimpanan key-value di flash internal, lebih fleksibel.                       |
| **Tipe Data**            | Byte (1 byte per alamat). Bisa menulis angka 0–255 per alamat.                        | Mendukung int, long, float, double, bool, string, dan byte array.                                   |
| **Ukuran Maksimum**      | Terbatas sesuai buffer EEPROM yang diinisialisasi (misal 512 byte, 1 KB).             | Terbatas oleh ukuran flash, tapi lebih fleksibel untuk data lebih besar dan banyak.                 |
| **Cara Menulis**         | `EEPROM.write(address, value)` diikuti `EEPROM.commit()`.                             | `prefs.putInt(key, value)` / `prefs.putString(key, value)` langsung menulis dengan key.             |
| **Cara Membaca**         | `EEPROM.read(address)`.                                                               | `prefs.getInt(key, default)` / `prefs.getString(key, default)`.                                     |
| **Penyimpanan Permanen** | Harus memanggil `EEPROM.commit()` untuk menyimpan ke flash.                           | Otomatis tersimpan di flash ketika `put` dipanggil.                                                 |
| **Kelebihan**            | Simpel untuk data byte sederhana, contoh: status ON/OFF, angka kecil.                 | Fleksibel, mendukung berbagai tipe data, lebih mudah digunakan untuk konfigurasi dan data aplikasi. |
| **Kekurangan**           | Sulit menyimpan tipe data kompleks, menulis berulang bisa mengurangi umur flash.      | Membutuhkan key untuk tiap data, tidak byte-per-byte, sedikit lebih berat dari EEPROM.              |
| **Use Case Ideal**       | Data kecil, jarang berubah, misal flag sensor, status perangkat.                      | Data konfigurasi, counter, string, setting aplikasi, data IoT yang lebih kompleks.                  |

---

### Kesimpulan Singkat

* Gunakan **EEPROM** untuk data kecil dan sederhana yang jarang diubah.
* Gunakan **Preferences** untuk data konfigurasi lebih kompleks atau ketika ingin menyimpan tipe data selain byte.
