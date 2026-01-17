# Materi: Implementasi Algoritma AI Sederhana pada ESP32

## 1. Konsep Dasar AI pada ESP32
ESP32 dengan dual-core 240MHz cocok untuk algoritma AI sederhana seperti:
- Neural Network kecil
- Decision Tree
- K-Nearest Neighbors (KNN)
- Support Vector Machine (SVM) sederhana
- Fuzzy Logic

## 2. Contoh 1: Decision Tree Sederhana

```cpp
/*
 * ALGORITMA AI: DECISION TREE PADA ESP32
 * Klasifikasi kondisi lingkungan
 */

#include <Arduino.h>

// Struktur node decision tree
struct TreeNode {
  int feature_index;
  float threshold;
  int left_class;
  int right_class;
  bool is_leaf;
};

// Decision tree model (manual)
TreeNode decision_tree[] = {
  {0, 25.0, -1, 1, false},   // Node 0: Feature 0 (suhu)
  {1, 60.0, 0, 2, false},    // Node 1: Feature 1 (kelembaban)
  {-1, -1, 1, -1, true},     // Node 2: Leaf node - PANAS
  {-1, -1, 0, -1, true},     // Node 3: Leaf node - NORMAL
  {-1, -1, 2, -1, true}      // Node 4: Leaf node - DINGIN
};

// Klasifikasi: 0=NORMAL, 1=PANAS, 2=DINGIN
String class_names[] = {"NORMAL", "PANAS", "DINGIN"};

// Fungsi predict decision tree
int predict_decision_tree(float features[2]) {
  int current_node = 0;
  int steps = 0;
  
  Serial.println("\nDecision Tree Inference:");
  Serial.println("========================");
  
  while (!decision_tree[current_node].is_leaf && steps < 10) {
    steps++;
    int feature_idx = decision_tree[current_node].feature_index;
    float threshold = decision_tree[current_node].threshold;
    float feature_value = features[feature_idx];
    
    Serial.print("Step ");
    Serial.print(steps);
    Serial.print(": Node ");
    Serial.print(current_node);
    Serial.print(" -> Feature[");
    Serial.print(feature_idx);
    Serial.print("] = ");
    Serial.print(feature_value);
    Serial.print(" ?<= ");
    Serial.print(threshold);
    
    if (feature_value <= threshold) {
      current_node = current_node * 2 + 1;  // Go left
      Serial.println(" -> YES (Go Left)");
    } else {
      current_node = current_node * 2 + 2;  // Go right
      Serial.println(" -> NO (Go Right)");
    }
  }
  
  Serial.print("Leaf Node ");
  Serial.print(current_node);
  Serial.print(": Class = ");
  Serial.println(decision_tree[current_node].left_class);
  
  return decision_tree[current_node].left_class;
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n======================================");
  Serial.println("DECISION TREE AI PADA ESP32");
  Serial.println("======================================\n");
  
  Serial.println("Decision Tree Structure:");
  Serial.println("Node 0: Suhu <= 25°C ?");
  Serial.println("  ├─ Ya -> Node 1: Kelembaban <= 60% ?");
  Serial.println("  │         ├─ Ya -> Class: NORMAL");
  Serial.println("  │         └─ Tidak -> Class: PANAS");
  Serial.println("  └─ Tidak -> Class: DINGIN");
}

void loop() {
  static int cycle = 0;
  
  // Generate random sensor data
  float suhu = random(15, 35);      // 15-35°C
  float kelembaban = random(40, 90); // 40-90%
  float features[2] = {suhu, kelembaban};
  
  Serial.println("\n════════════════════════════════════");
  Serial.println("CYCLE " + String(cycle));
  Serial.println("════════════════════════════════════");
  
  Serial.print("Sensor Data: ");
  Serial.print("Suhu = ");
  Serial.print(suhu, 1);
  Serial.print("°C, Kelembaban = ");
  Serial.print(kelembaban, 1);
  Serial.println("%");
  
  // Predict dengan decision tree
  int prediction = predict_decision_tree(features);
  
  Serial.println("\n--- HASIL KLASIFIKASI ---");
  Serial.print("Predicted Class: ");
  Serial.print(prediction);
  Serial.print(" (");
  Serial.print(class_names[prediction]);
  Serial.println(")");
  
  // Rekomendasi aksi
  Serial.print("Rekomendasi: ");
  switch(prediction) {
    case 0: Serial.println("Kondisi normal, tidak perlu tindakan"); break;
    case 1: Serial.println("Kondisi panas, nyalakan kipas"); break;
    case 2: Serial.println("Kondisi dingin, nyalakan heater"); break;
  }
  
  cycle++;
  delay(4000);
}
```

## 3. Contoh 2: K-Nearest Neighbors (KNN)

```cpp
/*
 * ALGORITMA AI: K-NEAREST NEIGHBORS (KNN) PADA ESP32
 * Klasifikasi pola gerakan
 */

#include <Arduino.h>
#include <math.h>

// Dataset training (accelerometer data + label)
// Label: 0=diam, 1=berjalan, 2=lari
float training_data[][3] = {
  {0.1, 0.1, 0.95},  // diam
  {0.1, 0.2, 0.98},  // diam
  {0.8, 0.7, 0.3},   // berjalan
  {0.7, 0.8, 0.4},   // berjalan
  {1.5, 1.6, 0.1},   // lari
  {1.6, 1.5, 0.2},   // lari
  {0.2, 0.1, 0.96},  // diam
  {0.9, 0.8, 0.2}    // berjalan
};

int training_labels[] = {0, 0, 1, 1, 2, 2, 0, 1};
int training_size = 8;

// Nama kelas
String movement_names[] = {"DIAM", "BERJALAN", "LARI"};

// Fungsi Euclidean distance
float euclidean_distance(float a[3], float b[3]) {
  float sum = 0;
  for (int i = 0; i < 3; i++) {
    sum += pow(a[i] - b[i], 2);
  }
  return sqrt(sum);
}

// KNN prediction
int knn_predict(float test_point[3], int k = 3) {
  Serial.println("\nKNN Inference Process:");
  Serial.println("======================");
  
  // Hitung semua distances
  float distances[training_size];
  Serial.println("Menghitung distances ke titik training:");
  
  for (int i = 0; i < training_size; i++) {
    distances[i] = euclidean_distance(test_point, training_data[i]);
    
    Serial.print("  Data ");
    Serial.print(i);
    Serial.print(": [");
    for (int j = 0; j < 3; j++) {
      Serial.print(training_data[i][j], 2);
      if (j < 2) Serial.print(", ");
    }
    Serial.print("] Label: ");
    Serial.print(training_labels[i]);
    Serial.print(", Distance: ");
    Serial.println(distances[i], 4);
  }
  
  // Cari k nearest neighbors
  int nearest_indices[k];
  float nearest_distances[k];
  
  // Inisialisasi
  for (int i = 0; i < k; i++) {
    nearest_indices[i] = -1;
    nearest_distances[i] = INFINITY;
  }
  
  // Find k smallest distances
  Serial.println("\nMencari " + String(k) + " nearest neighbors:");
  for (int i = 0; i < training_size; i++) {
    for (int j = 0; j < k; j++) {
      if (distances[i] < nearest_distances[j]) {
        // Geser array
        for (int m = k - 1; m > j; m--) {
          nearest_distances[m] = nearest_distances[m - 1];
          nearest_indices[m] = nearest_indices[m - 1];
        }
        // Insert baru
        nearest_distances[j] = distances[i];
        nearest_indices[j] = i;
        break;
      }
    }
  }
  
  // Tampilkan neighbors terpilih
  Serial.println("Nearest Neighbors:");
  for (int i = 0; i < k; i++) {
    Serial.print("  ");
    Serial.print(i + 1);
    Serial.print(". Index: ");
    Serial.print(nearest_indices[i]);
    Serial.print(", Distance: ");
    Serial.print(nearest_distances[i], 4);
    Serial.print(", Label: ");
    Serial.println(training_labels[nearest_indices[i]]);
  }
  
  // Voting
  int votes[3] = {0, 0, 0};  // 3 kelas
  for (int i = 0; i < k; i++) {
    int label = training_labels[nearest_indices[i]];
    votes[label]++;
  }
  
  Serial.println("\nHasil Voting:");
  for (int i = 0; i < 3; i++) {
    Serial.print("  Kelas ");
    Serial.print(i);
    Serial.print(" (");
    Serial.print(movement_names[i]);
    Serial.print("): ");
    Serial.println(votes[i]);
  }
  
  // Cari pemenang
  int max_votes = -1;
  int predicted_class = -1;
  for (int i = 0; i < 3; i++) {
    if (votes[i] > max_votes) {
      max_votes = votes[i];
      predicted_class = i;
    }
  }
  
  return predicted_class;
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n======================================");
  Serial.println("K-NEAREST NEIGHBORS AI PADA ESP32");
  Serial.println("======================================\n");
  
  Serial.println("Training Dataset:");
  Serial.println("Index | X       Y       Z       | Label");
  Serial.println("----------------------------------------");
  for (int i = 0; i < training_size; i++) {
    Serial.print(i);
    Serial.print("     | ");
    for (int j = 0; j < 3; j++) {
      Serial.print(training_data[i][j], 2);
      Serial.print("  ");
    }
    Serial.print("| ");
    Serial.println(movement_names[training_labels[i]]);
  }
  Serial.println("K = 3 (3 nearest neighbors)");
}

void loop() {
  static int cycle = 0;
  
  // Generate random accelerometer data
  float accel_data[3];
  for (int i = 0; i < 3; i++) {
    accel_data[i] = random(0, 200) / 100.0;  // 0.0-2.0
  }
  
  Serial.println("\n════════════════════════════════════");
  Serial.println("KNN CYCLE " + String(cycle));
  Serial.println("════════════════════════════════════");
  
  Serial.print("Accelerometer Data: [");
  for (int i = 0; i < 3; i++) {
    Serial.print(accel_data[i], 2);
    if (i < 2) Serial.print(", ");
  }
  Serial.println("]");
  
  // Predict dengan KNN
  int prediction = knn_predict(accel_data);
  
  Serial.println("\n--- HASIL PREDIKSI ---");
  Serial.print("Gerakan: ");
  Serial.println(movement_names[prediction]);
  
  // Kalori estimasi
  Serial.print("Estimasi Kalori: ");
  switch(prediction) {
    case 0: Serial.println("1 kalori/menit"); break;
    case 1: Serial.println("5 kalori/menit"); break;
    case 2: Serial.println("10 kalori/menit"); break;
  }
  
  cycle++;
  delay(5000);
}
```

## 4. Contoh 3: Neural Network Multi-Layer

```cpp
/*
 * ALGORITMA AI: MULTI-LAYER NEURAL NETWORK PADA ESP32
 * Prediksi kualitas udara
 */

#include <Arduino.h>
#include <math.h>

// Fungsi aktivasi
float sigmoid(float x) {
  return 1.0 / (1.0 + exp(-x));
}

float relu(float x) {
  return (x > 0) ? x : 0;
}

// Neural Network 2-layer (3-4-1)
class SimpleNeuralNetwork {
private:
  // Layer 1 weights (3 input -> 4 hidden)
  float W1[3][4] = {
    {0.1, 0.2, -0.1, 0.3},
    {-0.2, 0.3, 0.1, -0.1},
    {0.3, -0.2, 0.2, 0.1}
  };
  
  // Layer 1 bias
  float b1[4] = {0.1, -0.1, 0.2, 0.0};
  
  // Layer 2 weights (4 hidden -> 1 output)
  float W2[4] = {0.2, -0.3, 0.1, 0.4};
  
  // Layer 2 bias
  float b2 = 0.1;
  
public:
  float predict(float inputs[3]) {
    Serial.println("\nNeural Network Forward Pass:");
    Serial.println("============================");
    
    // Layer 1: Input -> Hidden
    float hidden[4] = {0};
    
    Serial.println("Layer 1 (3 → 4 neurons):");
    for (int j = 0; j < 4; j++) {
      Serial.print("  Neuron H");
      Serial.print(j);
      Serial.print(": sum = ");
      
      float sum = 0;
      for (int i = 0; i < 3; i++) {
        sum += inputs[i] * W1[i][j];
        Serial.print(inputs[i]);
        Serial.print("*");
        Serial.print(W1[i][j], 2);
        if (i < 2) Serial.print(" + ");
      }
      
      sum += b1[j];
      Serial.print(" + ");
      Serial.print(b1[j], 2);
      Serial.print(" = ");
      Serial.print(sum, 4);
      
      hidden[j] = relu(sum);
      Serial.print(", ReLU = ");
      Serial.println(hidden[j], 4);
    }
    
    // Layer 2: Hidden -> Output
    Serial.println("\nLayer 2 (4 → 1 neuron):");
    Serial.print("  Output neuron: sum = ");
    
    float output_sum = 0;
    for (int j = 0; j < 4; j++) {
      output_sum += hidden[j] * W2[j];
      Serial.print(hidden[j], 4);
      Serial.print("*");
      Serial.print(W2[j], 2);
      if (j < 3) Serial.print(" + ");
    }
    
    output_sum += b2;
    Serial.print(" + ");
    Serial.print(b2, 2);
    Serial.print(" = ");
    Serial.print(output_sum, 4);
    
    float output = sigmoid(output_sum);
    Serial.print(", Sigmoid = ");
    Serial.println(output, 4);
    
    return output;
  }
  
  void print_architecture() {
    Serial.println("\nNeural Network Architecture:");
    Serial.println("     Input Layer (3 neurons)");
    Serial.println("         ↓ Fully Connected");
    Serial.println("  Hidden Layer (4 neurons) + ReLU");
    Serial.println("         ↓ Fully Connected");
    Serial.println("  Output Layer (1 neuron) + Sigmoid");
  }
};

SimpleNeuralNetwork air_quality_nn;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n======================================");
  Serial.println("NEURAL NETWORK AI PADA ESP32");
  Serial.println("======================================\n");
  
  air_quality_nn.print_architecture();
  
  Serial.println("\nAplikasi: Prediksi Kualitas Udara");
  Serial.println("Input: [CO2(ppm), PM2.5(μg/m³), VOC(ppb)]");
  Serial.println("Output: 0-1 (0=buruk, 1=baik)");
}

void loop() {
  static int cycle = 0;
  
  // Simulasi sensor udara
  float co2 = random(300, 2000);      // 300-2000 ppm
  float pm25 = random(0, 300);        // 0-300 μg/m³
  float voc = random(0, 500);         // 0-500 ppb
  
  // Normalisasi
  float inputs[3] = {
    co2 / 2000.0,    // Normalize to 0-1
    pm25 / 300.0,
    voc / 500.0
  };
  
  Serial.println("\n════════════════════════════════════");
  Serial.println("NN CYCLE " + String(cycle));
  Serial.println("════════════════════════════════════");
  
  Serial.print("Sensor Data: ");
  Serial.print("CO2=");
  Serial.print(co2);
  Serial.print("ppm, PM2.5=");
  Serial.print(pm25);
  Serial.print("μg/m³, VOC=");
  Serial.print(voc);
  Serial.println("ppb");
  
  Serial.print("Normalized: [");
  for (int i = 0; i < 3; i++) {
    Serial.print(inputs[i], 3);
    if (i < 2) Serial.print(", ");
  }
  Serial.println("]");
  
  // Predict dengan neural network
  float prediction = air_quality_nn.predict(inputs);
  
  Serial.println("\n--- HASIL PREDIKSI ---");
  Serial.print("Skor Kualitas Udara: ");
  Serial.println(prediction, 4);
  
  Serial.print("Kategori: ");
  if (prediction > 0.7) {
    Serial.println("BAIK 👍");
  } else if (prediction > 0.4) {
    Serial.println("SEDANG ⚠️");
  } else {
    Serial.println("BURUK 🚨");
  }
  
  // Rekomendasi
  Serial.print("Rekomendasi: ");
  if (prediction > 0.7) {
    Serial.println("Buka jendela untuk ventilasi");
  } else if (prediction > 0.4) {
    Serial.println("Nyalakan air purifier");
  } else {
    Serial.println("Gunakan masker, hindari aktivitas luar");
  }
  
  cycle++;
  delay(4000);
}
```

## 5. Contoh 4: Fuzzy Logic Controller

```cpp
/*
 * ALGORITMA AI: FUZZY LOGIC PADA ESP32
 * Kontrol suhu ruangan otomatis
 */

#include <Arduino.h>

// Fungsi keanggotaan segitiga
float triangular_mf(float x, float a, float b, float c) {
  if (x <= a) return 0;
  if (x >= c) return 0;
  if (x <= b) return (x - a) / (b - a);
  return (c - x) / (c - b);
}

// Fungsi keanggotaan trapezoid
float trapezoidal_mf(float x, float a, float b, float c, float d) {
  if (x <= a) return 0;
  if (x >= d) return 0;
  if (x >= b && x <= c) return 1;
  if (x <= b) return (x - a) / (b - a);
  return (d - x) / (d - c);
}

// Fuzzy Logic Controller untuk AC
class FuzzyACController {
private:
  // Input: Suhu ruangan (18-32°C)
  struct {
    float dingin;
    float normal;
    float panas;
  } suhu_fuzzy;
  
  // Output: Kecepatan kipas (0-100%)
  struct {
    float lambat;
    float sedang;
    float cepat;
  } fan_fuzzy;
  
public:
  // Fuzzifikasi suhu
  void fuzzify_suhu(float suhu) {
    Serial.println("\nFuzzifikasi Input:");
    Serial.println("==================");
    
    suhu_fuzzy.dingin = triangular_mf(suhu, 18, 20, 23);
    suhu_fuzzy.normal = trapezoidal_mf(suhu, 21, 23, 26, 28);
    suhu_fuzzy.panas = triangular_mf(suhu, 26, 29, 32);
    
    Serial.print("Suhu: ");
    Serial.print(suhu, 1);
    Serial.println("°C");
    Serial.print("  Dingin: ");
    Serial.println(suhu_fuzzy.dingin, 3);
    Serial.print("  Normal: ");
    Serial.println(suhu_fuzzy.normal, 3);
    Serial.print("  Panas:  ");
    Serial.println(suhu_fuzzy.panas, 3);
  }
  
  // Inferensi fuzzy (aturan-aturan)
  void inference() {
    Serial.println("\nInferensi Fuzzy (Rules):");
    Serial.println("========================");
    
    // Reset output
    fan_fuzzy.lambat = 0;
    fan_fuzzy.sedang = 0;
    fan_fuzzy.cepat = 0;
    
    // Rule 1: IF suhu dingin THEN fan lambat
    fan_fuzzy.lambat = max(fan_fuzzy.lambat, suhu_fuzzy.dingin);
    Serial.print("Rule 1: IF Dingin THEN Lambat = ");
    Serial.println(suhu_fuzzy.dingin, 3);
    
    // Rule 2: IF suhu normal THEN fan sedang
    fan_fuzzy.sedang = max(fan_fuzzy.sedang, suhu_fuzzy.normal);
    Serial.print("Rule 2: IF Normal THEN Sedang = ");
    Serial.println(suhu_fuzzy.normal, 3);
    
    // Rule 3: IF suhu panas THEN fan cepat
    fan_fuzzy.cepat = max(fan_fuzzy.cepat, suhu_fuzzy.panas);
    Serial.print("Rule 3: IF Panas THEN Cepat  = ");
    Serial.println(suhu_fuzzy.panas, 3);
  }
  
  // Defuzzifikasi (Centroid Method sederhana)
  float defuzzify() {
    Serial.println("\nDefuzzifikasi Output:");
    Serial.println("=====================");
    
    // Representasi singleton untuk output
    float lambat_center = 25;   // 25% kecepatan
    float sedang_center = 50;   // 50% kecepatan
    float cepat_center = 85;    // 85% kecepatan
    
    Serial.print("Singleton Centers: ");
    Serial.print("Lambat=");
    Serial.print(lambat_center);
    Serial.print("%, Sedang=");
    Serial.print(sedang_center);
    Serial.print("%, Cepat=");
    Serial.print(cepat_center);
    Serial.println("%");
    
    // Weighted average
    float numerator = (fan_fuzzy.lambat * lambat_center) +
                     (fan_fuzzy.sedang * sedang_center) +
                     (fan_fuzzy.cepat * cepat_center);
    
    float denominator = fan_fuzzy.lambat + fan_fuzzy.sedang + fan_fuzzy.cepat;
    
    Serial.print("Numerator: ");
    Serial.println(numerator, 2);
    Serial.print("Denominator: ");
    Serial.println(denominator, 2);
    
    float fan_speed = (denominator > 0) ? numerator / denominator : 0;
    
    Serial.print("Kecepatan Kipas: ");
    Serial.print(fan_speed, 1);
    Serial.println("%");
    
    return fan_speed;
  }
  
  float control(float suhu) {
    Serial.println("\n════════════════════════════════════");
    Serial.println("FUZZY LOGIC PROCESS:");
    Serial.println("════════════════════════════════════");
    
    fuzzify_suhu(suhu);
    inference();
    return defuzzify();
  }
};

FuzzyACController fuzzy_ac;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("\n======================================");
  Serial.println("FUZZY LOGIC AI PADA ESP32");
  Serial.println("======================================\n");
  
  Serial.println("Fuzzy System for AC Control:");
  Serial.println("Input Variable: Suhu Ruangan");
  Serial.println("  • Dingin: 18-23°C (peak: 20°C)");
  Serial.println("  • Normal: 21-28°C (flat: 23-26°C)");
  Serial.println("  • Panas:  26-32°C (peak: 29°C)");
  
  Serial.println("\nOutput Variable: Kecepatan Kipas");
  Serial.println("  • Lambat: 25%");
  Serial.println("  • Sedang: 50%");
  Serial.println("  • Cepat:  85%");
  
  Serial.println("\nRules:");
  Serial.println("1. IF Suhu Dingin THEN Fan Lambat");
  Serial.println("2. IF Suhu Normal THEN Fan Sedang");
  Serial.println("3. IF Suhu Panas  THEN Fan Cepat");
}

void loop() {
  static int cycle = 0;
  
  // Generate random suhu
  float suhu = random(180, 320) / 10.0;  // 18.0-32.0°C
  
  Serial.println("\n════════════════════════════════════");
  Serial.println("CYCLE " + String(cycle));
  Serial.println("════════════════════════════════════");
  
  // Fuzzy logic control
  float fan_speed = fuzzy_ac.control(suhu);
  
  Serial.println("\n--- KONTROL AC ---");
  Serial.print("Suhu Aktual: ");
  Serial.print(suhu, 1);
  Serial.println("°C");
  
  Serial.print("Kecepatan Fan: ");
  Serial.print(fan_speed, 1);
  Serial.println("%");
  
  // Konversi ke PWM (0-255)
  int pwm_value = map(fan_speed, 0, 100, 0, 255);
  Serial.print("PWM Value: ");
  Serial.println(pwm_value);
  
  // Status AC
  Serial.print("Status AC: ");
  if (fan_speed < 30) {
    Serial.println("Mati/Standby");
  } else if (fan_speed < 60) {
    Serial.println("Low Power");
  } else {
    Serial.println("High Power");
  }
  
  cycle++;
  delay(3000);
}
```

## 6. Ringkasan Algoritma AI yang Cocok untuk ESP32

| Algoritma | Kompleksitas | Memory | Kecepatan | Aplikasi |
|-----------|--------------|--------|-----------|----------|
| Neural Network Sederhana | Rendah | 1-5KB | Cepat | Klasifikasi, Prediksi |
| Decision Tree | Sangat Rendah | <1KB | Sangat Cepat | Aturan sederhana |
| K-NN | Sedang | 2-10KB | Medium | Pattern matching |
| Fuzzy Logic | Rendah | 1-3KB | Cepat | Kontrol otomatis |
| Linear Regression | Rendah | <1KB | Cepat | Prediksi numerik |

## 7. Tips Optimasi untuk ESP32

1. **Gunakan Fixed-point arithmetic** untuk menghindari floating-point
2. **Precompute values** jika mungkin
3. **Gunakan lookup tables** untuk fungsi kompleks
4. **Optimasi loop** dan hindari dynamic allocation
5. **Gunakan PSRAM** jika tersedia pada ESP32
