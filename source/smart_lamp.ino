#include <WiFi.h>
#include "SinricPro.h"
#include "SinricProSwitch.h"

// ================= SINRIC =================
#define APP_KEY       "f8daa243-1baa-45b0-ba75-f3272aaa41c4"
#define APP_SECRET    "ceb76fb4-f15b-4b5e-b641-90a86d0255e6-6f3fbd20-13f1-4ff6-b64c-640d67aece02"

// ================= DEVICE ID = sesuaikan dengan id device di sinric nya =================
#define DEVICE_ID_1   "6a132ba7977a0619a747ce33" 
#define DEVICE_ID_2   "6a132b8df9b5f15fa7dda1d2"

// ================= RELAY PIN = sesuaikan dengan pin yang kalian gunakan =================
#define RELAY1 32
#define RELAY2 33

// ================= INI GAUSA DI UBAH" YAAAA, UDH SETTINGAN PALING ENAK SOALNYAAA =================
#define WIFI_TIMEOUT_MS   8000  
#define WIFI_RETRY_DELAY  500    

struct WiFiCredential {
  const char* ssid;
  const char* password;
};
// INI JUMLAH WIFI YANG DI MASUKAN KEDALAM ESP32, JIKA INGIN MERUBAH, SESUAIKAN SSID DAN PASSWORDNYA YAAAA!!!!
const WiFiCredential wifiList[] = {
  { "DUTASEBLAK",                  "123qwerty"       },
  { "butuh mah dateng",            "GAKTAULAH"       },
  { "KKCEI",                       "Ceicoffee"       },
  { "plsjanganini",                "janganmasukbabi" },
  { "PT.KOSPING MAJU SEJAHTERA",   "ABALPUQI123"     },
  { "AnggaHome5G",                 "24102006"        },
  { "Naufal baik hati",            "opaltampan808"   },
};

const int WIFI_COUNT = sizeof(wifiList) / sizeof(wifiList[0]);

// ================= CALLBACK =================
bool onPowerState1(const String &deviceId, bool &state) {
  Serial.printf("Lampu Dalam: %s\r\n", state ? "ON" : "OFF");
  digitalWrite(RELAY1, state ? LOW : HIGH);
  return true;
}

bool onPowerState2(const String &deviceId, bool &state) {
  Serial.printf("Lampu Luar: %s\r\n", state ? "ON" : "OFF");
  digitalWrite(RELAY2, state ? LOW : HIGH);
  return true;
}

// ================= MULTI-WIFI CONNECT =================
// Script untuk koneksi ke wifi secara berurutan.
// Mengembalikan true jika berhasil terhubung ke salah satu jaringan.
bool connectToAnyWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true);
  delay(100);

  for (int i = 0; i < WIFI_COUNT; i++) {
    Serial.printf("\n[WiFi] Mencoba (%d/%d): \"%s\" ...\n", i + 1, WIFI_COUNT, wifiList[i].ssid);
    WiFi.begin(wifiList[i].ssid, wifiList[i].password);

    unsigned long startTime = millis();
    bool connected = false;

    while (millis() - startTime < WIFI_TIMEOUT_MS) {
      if (WiFi.status() == WL_CONNECTED) {
        connected = true;
        break;
      }
      Serial.print(".");
      delay(WIFI_RETRY_DELAY);
    }

    if (connected) {
      Serial.printf("\n[WiFi] Berhasil terhubung ke: \"%s\"\n", wifiList[i].ssid);
      Serial.printf("[WiFi] IP Address: %s\n", WiFi.localIP().toString().c_str());
      return true;
    } else {
      Serial.printf("\n[WiFi] Gagal terhubung ke: \"%s\"\n", wifiList[i].ssid);
      WiFi.disconnect(true);
      delay(200);
    }
  }

  Serial.println("\n[WiFi] SEMUA jaringan gagal. Akan dicoba ulang...");
  return false;
}

// ================= SETUP WIFI =================
// Dipanggil saat booting. Loop terus sampai berhasil.
void setupWiFi() {
  Serial.println("\n========================================");
  Serial.println("       ESP32 Smart Lamp - Multi WiFi   ");
  Serial.println("========================================");

  while (!connectToAnyWiFi()) {
    Serial.println("[WiFi] Mengulang pencarian dari awal...\n");
    delay(2000);
  }
}

// ================= AUTO RECONNECT =================
// Dipanggil di loop(). Jika koneksi putus bakal cari ulang semua WiFi.
void handleWiFiReconnect() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("\n[WiFi] Koneksi terputus! Memulai auto-reconnect...");
    SinricPro.stop();
    WiFi.disconnect(true);
    delay(500);

    while (!connectToAnyWiFi()) {
      Serial.println("[WiFi] Reconnect gagal. Mencoba lagi...\n");
      delay(2000);
    }

    // Jalankan ulang Sinric Pro setelah WiFi tersambung
    SinricPro.begin(APP_KEY, APP_SECRET);
    Serial.println("[Sinric] Sinric Pro kembali aktif.");
  }
}

// ================= SINRIC =================
void setupSinricPro() {
  SinricProSwitch &lampuDalam = SinricPro[DEVICE_ID_1];
  lampuDalam.onPowerState(onPowerState1);

  SinricProSwitch &lampuLuar = SinricPro[DEVICE_ID_2];
  lampuLuar.onPowerState(onPowerState2);

  SinricPro.begin(APP_KEY, APP_SECRET);
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);

  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);

  // Relay OFF saat awal
  digitalWrite(RELAY1, HIGH);
  digitalWrite(RELAY2, HIGH);

  setupWiFi();
  setupSinricPro();
}

// ================= LOOP =================
void loop() {
  handleWiFiReconnect();
  SinricPro.handle();
}
