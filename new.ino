#define BLYNK_TEMPLATE_ID "BLYNK_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "SensorPIR"
#define BLYNK_AUTH_TOKEN "BLYNK_AUTH_TOKEN"

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <BlynkSimpleEsp8266.h>

// Definisi pin
#define PIR_PIN 5     // Pin sensor PIR
#define BUZZER_PIN 4   // Pin buzzer

// Informasi WiFi
char ssid[] = "WiFi_SSID"; // Nama WiFi Anda
char pass[] = "WiFi_Password";     // Password WiFi Anda

// Variabel untuk status sensor PIR dari Blynk Switch
bool pirEnabled = true;  // Sensor PIR diaktifkan secara default

// Definisi variabel untuk webhook pada script yak akan di kirim ke spreedsheet
const char* webhookUrl = "https://script.google.com/macros/s/WEB_HOOK_URL/exec";

// Fungsi untuk menangani perubahan status switch di Blynk
BLYNK_WRITE(V3) {
  pirEnabled = param.asInt();  // Ambil status switch (0 atau 1)
  if (pirEnabled == 0) {
    digitalWrite(BUZZER_PIN, LOW);  // Matikan buzzer jika PIR dimatikan
    Serial.println("Sensor PIR dimatikan!");
  } else {
    Serial.println("Sensor PIR diaktifkan!");
  }
}

void sendToGoogleSheets(int pirState) {
  WiFiClientSecure client;
  client.setInsecure(); 
  if (client.connect("script.google.com", 443)) { // HTTPS
    String url = String(webhookUrl) + "?pirState=" + pirState;
    client.println(String("GET ") + url + " HTTP/1.1");
    client.println("Host: script.google.com");
    client.println("Connection: close");
    client.println();

    // Tunggu respons dari server
    while(client.available()) {
      String line = client.readStringUntil('\r');
      Serial.print(line);  // Menampilkan respons server
    }

    Serial.println("Data dikirim ke Google Sheets: " + url);
  } else {
    Serial.println("Gagal terhubung ke Google Sheets.");
  }
}

void setup() {
  Serial.begin(115200);         // Mulai komunikasi serial untuk debugging
  pinMode(PIR_PIN, INPUT);     // Atur PIR sebagai input
  pinMode(BUZZER_PIN, OUTPUT); // Atur buzzer sebagai output

  // Hubungkan ke WiFi
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Terhubung!");

  // Hubungkan ke Blynk
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Serial.println("Menghubungkan....");
}

void loop() {
  Blynk.run();

  if (pirEnabled) {
    int pirState = digitalRead(PIR_PIN); // Membaca status sensor PIR

    if (pirState == HIGH) { // Jika ada gerakan
      digitalWrite(BUZZER_PIN, HIGH); // Nyalakan buzzer
      Serial.println("Gerakan terdeteksi!");
      sendToGoogleSheets(1);

      // Update widget di Blynk
      Blynk.virtualWrite(V1, 1);   // Mengirimkan nilai 1 ke Gauge (Virtual Pin V1)
      Blynk.virtualWrite(V2, 255); // Menyalakan LED di Virtual Pin V2 (nilai maksimal 255)
    } else {
      digitalWrite(BUZZER_PIN, LOW); // Matikan buzzer
      Serial.println("Tidak ada gerakan!");
      sendToGoogleSheets(0);

      // Update widget di Blynk
      Blynk.virtualWrite(V1, 0); // Mengirimkan nilai 0 ke Gauge (Virtual Pin V1)
      Blynk.virtualWrite(V2, 0); // Mematikan LED di Virtual Pin V2
    }
  } else {
    // Jika PIR dimatikan, pastikan buzzer mati dan widget diperbarui
    digitalWrite(BUZZER_PIN, LOW);
    Blynk.virtualWrite(V1, 0);  // Mengirimkan nilai 0 ke Gauge
    Blynk.virtualWrite(V2, 0);  // Mematikan LED
  }

  delay(1000); // Delay untuk stabilisasi
}
