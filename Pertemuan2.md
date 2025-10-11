# Simulasi Register dan Flag

## Tujuan Pembelajaran

Praktikum ini bertujuan untuk:

* Memahami fungsi register umum (R0–R31) dalam prosesor atau mikrokontroler.
* Mengenal dan mengamati status bit flag (C, Z, N, V, S) yang muncul akibat operasi aritmatika.
* Menampilkan perubahan flag secara visual menggunakan LED.
* Melatih pemahaman antara operasi logika (simulasi acak) dan urutan demonstrasi flag (demo bergantian).

---

## Deskripsi Umum

Program ini mensimulasikan operasi aritmatika 8-bit (penjumlahan) dan menampilkan hasilnya dalam bentuk nilai register serta status flag yang muncul dari hasil operasi tersebut.

Selain simulasi acak, terdapat juga mode demo flag bergantian yang memperlihatkan bagaimana masing-masing flag bekerja satu per satu.

Program berjalan otomatis dan berganti mode setiap 20 detik:

* Mode 0 → Simulasi acak operasi register
* Mode 1 → Demo urutan flag satu per satu

---

## Komponen yang Digunakan

| Komponen           | Jumlah     | Keterangan                |
| ------------------ | ---------- | ------------------------- |
| Arduino Uno / Nano | 1          | Mikrokontroler utama      |
| LED                | 5          | Indikator setiap flag     |
| Resistor 220Ω      | 5          | Pembatas arus LED         |
| Kabel jumper       | secukupnya | Penghubung ke pin digital |
| Breadboard         | 1          | Rangkaian uji             |

---

## Skema Pin LED Flag

| Flag | Nama     | Pin Arduino | Warna LED (disarankan) | Fungsi                                               |
| ---- | -------- | ----------- | ---------------------- | ---------------------------------------------------- |
| C    | Carry    | D2          | Merah                  | Menandakan overflow pada operasi 8-bit               |
| Z    | Zero     | D3          | Hijau                  | Menandakan hasil operasi bernilai 0                  |
| N    | Negative | D4          | Kuning                 | Menandakan bit ke-7 hasil bernilai 1 (angka negatif) |
| V    | Overflow | D5          | Biru                   | Menandakan overflow signed (tanda hasil salah)       |
| S    | Sign     | D6          | Putih                  | Kombinasi dari N XOR V                               |

---

## Penjelasan Teori Dasar

### Register

Register adalah memori kecil di dalam CPU yang digunakan untuk menyimpan data sementara selama proses perhitungan.

Dalam simulasi ini:

* R0 dan R1: Operand (nilai yang akan dijumlahkan)
* R2: Hasil operasi R0 + R1

---

### Status Flag

Setelah operasi aritmatika atau logika, prosesor mengubah beberapa bit khusus yang disebut status flag.

| Flag | Nama          | Kondisi Aktif          | Makna                                                  |
| ---- | ------------- | ---------------------- | ------------------------------------------------------ |
| C    | Carry Flag    | sum > 255              | Terjadi carry out dari bit ke-7 (overflow unsigned)    |
| Z    | Zero Flag     | hasil == 0             | Hasil operasi adalah nol                               |
| N    | Negative Flag | bit7 == 1              | Hasil memiliki bit paling signifikan 1 (angka negatif) |
| V    | Overflow Flag | logika signed overflow | Overflow dalam operasi bertanda (signed)               |
| S    | Sign Flag     | S = N XOR V            | Menentukan tanda hasil berdasarkan kombinasi N dan V   |

---

## Alur Program

1. **Inisialisasi**

   * Serial monitor dimulai pada kecepatan 9600 bps.
   * Semua pin LED diatur sebagai output.
   * Program menyalakan semua LED selama 2 detik untuk menguji rangkaian.

2. **Mode Simulasi Acak (mode = 0)**

   * Program menghasilkan nilai acak untuk R0 dan R1.
   * Menghitung R2 = (R0 + R1) & 0xFF.
   * Menentukan nilai setiap flag berdasarkan hasil operasi.
   * Menampilkan data ke Serial Monitor dan mengupdate LED sesuai flag aktif.

3. **Mode Demo Flag Bergantian (mode = 1)**

   * Menyalakan setiap flag satu per satu untuk menjelaskan arti setiap LED.
   * Terdapat 5 langkah (C, Z, N, V, S).
   * Setiap langkah tampil bergantian setiap sekitar 1,5 detik.

4. **Pergantian Mode**

   * Setiap 20 detik, mode akan berpindah otomatis antara simulasi acak dan demo flag.

---

## Contoh Output di Serial Monitor

```
----------------------
R0: 39
R1: 113
R2: 152
Flags -> C: 0 | Z: 0 | N: 1 | V: 1 | S: 0
```

Penjelasan:

* Hasil penjumlahan menghasilkan nilai negatif dalam konteks signed.
* Overflow terjadi (V=1).
* Tidak ada carry (C=0).
* Hasil bukan nol (Z=0).

---

## Penjelasan Praktik

### Simulasi Acak

* LED akan menyala sesuai kondisi flag yang aktif.
* Data acak menunjukkan beragam kondisi hasil operasi.
* Cocok untuk memahami kombinasi flag yang mungkin muncul dari berbagai operasi aritmatika.

### Demo Flag Bergantian

* Setiap 1,5 detik LED akan berpindah, menyalakan satu flag saja.
* Mode ini membantu memahami arti dan fungsi masing-masing flag secara individual.

---

## Eksperimen Lanjutan

Beberapa ide pengembangan:

* Menambahkan tombol untuk mengubah mode secara manual.
* Menampilkan hasil di LCD 16x2 atau OLED.
* Mengirim data ke Serial Plotter untuk melihat perubahan flag secara grafik.
* Mengubah operasi menjadi pengurangan atau logika bitwise.

---

## Kesimpulan

Dari simulasi ini, mahasiswa diharapkan dapat:

1. Memahami bagaimana register bekerja dalam operasi dasar mikrokontroler.
2. Mengenal fungsi status flag (Carry, Zero, Negative, Overflow, Sign).
3. Menghubungkan hasil perhitungan logika/aritmatika dengan indikasi LED secara visual.
4. Menjelaskan hubungan antara operasi data 8-bit dan reaksi logika biner pada flag.