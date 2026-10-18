#include <M5StickCPlus2.h>
#include <WiFi.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>

// Wbudowana dioda podczerwieni w M5StickC PLUS2 znajduje sie na GPIO 19
#define IR_LED_PIN 19

// Zmienne do nawigacji w menu
int currentMenu = 0;
const int totalMenuItems = 4;
String menuItems[] = {
  "1. Skaner Wi-Fi",
  "2. Skaner Bluetooth",
  "3. TV IR (Mute/Power)",
  "4. Bateria i System"
};

void drawMenu();
void runWiFiScanner();
void runBLEScanner();
void runIRRemote();
void runSystemInfo();

void setup() {
  auto cfg = M5.config();
  StickCP2.begin(cfg);

  // Konfiguracja ekranu
  StickCP2.Display.setRotation(3); // Pozioma orientacja
  StickCP2.Display.setTextSize(2);

  // Konfiguracja pinu IR jako wyjscie
  pinMode(IR_LED_PIN, OUTPUT);
  digitalWrite(IR_LED_PIN, LOW);

  // Wyłączenie niepotrzebnych modułów radiowych na start (oszczędność baterii)
  WiFi.mode(WIFI_OFF);
  
  drawMenu();
}

void loop() {
  StickCP2.update();

  // Przycisk M5 (Główny na froncie) -> Przewijanie menu w dół
  if (StickCP2.BtnA.wasPressed()) {
    currentMenu = (currentMenu + 1) % totalMenuItems;
    drawMenu();
  }

  // Przycisk Boczny (BtnB) -> Wybór / Uruchomienie opcji
  if (StickCP2.BtnB.wasPressed()) {
    switch (currentMenu) {
      case 0: runWiFiScanner(); break;
      case 1: runBLEScanner(); break;
      case 2: runIRRemote(); break;
      case 3: runSystemInfo(); break;
    }
    drawMenu(); // Powrót do menu po zakończeniu działania funkcji
  }

  delay(50);
}

// Funkcja rysująca interfejs menu
void drawMenu() {
  StickCP2.Display.fillScreen(BLACK);
  StickCP2.Display.setCursor(5, 2);
  StickCP2.Display.setTextColor(YELLOW, BLACK);
  StickCP2.Display.setTextSize(2);
  StickCP2.Display.println("--- BRUCE MINI ---");

  StickCP2.Display.setTextSize(1);
  for (int i = 0; i < totalMenuItems; i++) {
    StickCP2.Display.setCursor(5, 25 + (i * 14));
    if (i == currentMenu) {
      StickCP2.Display.setTextColor(BLACK, GREEN); // Wyróżnienie wybranej opcji
      StickCP2.Display.print(" > ");
      StickCP2.Display.println(menuItems[i]);
    } else {
      StickCP2.Display.setTextColor(WHITE, BLACK);
      StickCP2.Display.print("   ");
      StickCP2.Display.println(menuItems[i]);
    }
  }

  StickCP2.Display.setCursor(5, 115);
  StickCP2.Display.setTextColor(DARKGREY, BLACK);
  StickCP2.Display.println("[M5] Nastepny  |  [Bok] Wybierz");
}

// 1. DZIAŁAJĄCY SKANER WI-FI
void runWiFiScanner() {
  StickCP2.Display.fillScreen(BLACK);
  StickCP2.Display.setCursor(5, 5);
  StickCP2.Display.setTextColor(CYAN, BLACK);
  StickCP2.Display.setTextSize(2);
  StickCP2.Display.println("Skanowanie...");

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  int n = WiFi.scanNetworks();

  StickCP2.Display.fillScreen(BLACK);
  StickCP2.Display.setCursor(5, 2);
  StickCP2.Display.setTextColor(YELLOW, BLACK);
  StickCP2.Display.printf("Sieci: %d\n", n);
  StickCP2.Display.setTextSize(1);

  if (n == 0) {
    StickCP2.Display.setTextColor(RED, BLACK);
    StickCP2.Display.println("Brak sieci w zasiegu.");
  } else {
    int showCount = (n > 6) ? 6 : n; // Wyświetl max 6 sieci na ekranie
    for (int i = 0; i < showCount; ++i) {
      StickCP2.Display.setTextColor(WHITE, BLACK);
      String ssid = WiFi.SSID(i);
      if (ssid.length() > 14) ssid = ssid.substring(0, 14) + "..";
      StickCP2.Display.printf("%d:%s (%d)\n", i + 1, ssid.c_str(), WiFi.RSSI(i));
    }
  }

  WiFi.mode(WIFI_OFF); // Wyłącz moduł po skanowaniu

  StickCP2.Display.setCursor(5, 115);
  StickCP2.Display.setTextColor(RED, BLACK);
  StickCP2.Display.println("Wcisnij [M5] aby wrocic");

  while (true) {
    StickCP2.update();
    if (StickCP2.BtnA.wasPressed()) break;
    delay(50);
  }
}

// 2. DZIAŁAJĄCY SKANER BLUETOOTH LE
void runBLEScanner() {
  StickCP2.Display.fillScreen(BLACK);
  StickCP2.Display.setCursor(5, 5);
  StickCP2.Display.setTextColor(MAGENTA, BLACK);
  StickCP2.Display.setTextSize(2);
  StickCP2.Display.println("Skan BLE...");

  BLEDevice::init("");
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setActiveScan(true);
  BLEScanResults foundDevices = pBLEScan->start(3, false);

  StickCP2.Display.fillScreen(BLACK);
  StickCP2.Display.setCursor(5, 2);
  StickCP2.Display.setTextColor(YELLOW, BLACK);
  int count = foundDevices.getCount();
  StickCP2.Display.printf("Urzadzenia BLE: %d\n", count);
  StickCP2.Display.setTextSize(1);

  int showCount = (count > 5) ? 5 : count;
  for (int i = 0; i < showCount; i++) {
    BLEAdvertisedDevice device = foundDevices.getDevice(i);
    String name = device.getName().c_str();
    if (name.length() == 0) name = device.getAddress().toString().c_str();
    if (name.length() > 18) name = name.substring(0, 18);
    
    StickCP2.Display.setTextColor(GREEN, BLACK);
    StickCP2.Display.printf("[%d] %s\n", i + 1, name.c_str());
  }

  BLEDevice::deinit(true);

  StickCP2.Display.setCursor(5, 115);
  StickCP2.Display.setTextColor(RED, BLACK);
  StickCP2.Display.println("Wcisnij [M5] aby wrocic");

  while (true) {
    StickCP2.update();
    if (StickCP2.BtnA.wasPressed()) break;
    delay(50);
  }
}

// Wysyłanie surowego ciągu impulsów IR (38kHz)
void sendIRRaw(const uint16_t* buf, int len) {
  for (int i = 0; i < len; i++) {
    uint16_t duration = buf[i];
    if (i % 2 == 0) {
      // Impuls 38kHz (Mark)
      long startTime = micros();
      while (micros() - startTime < duration) {
        digitalWrite(IR_LED_PIN, HIGH);
        delayMicroseconds(9);
        digitalWrite(IR_LED_PIN, LOW);
        delayMicroseconds(16);
      }
    } else {
      // Przerwa (Space)
      digitalWrite(IR_LED_PIN, LOW);
      delayMicroseconds(duration);
    }
  }
  digitalWrite(IR_LED_PIN, LOW);
}

// 3. DZIAŁAJĄCY PILOT IR (NEC MUTE / POWER)
void runIRRemote() {
  StickCP2.Display.fillScreen(BLACK);
  StickCP2.Display.setCursor(5, 5);
  StickCP2.Display.setTextColor(ORANGE, BLACK);
  StickCP2.Display.setTextSize(2);
  StickCP2.Display.println("TV IR Remote");
  
  StickCP2.Display.setTextSize(1);
  StickCP2.Display.setCursor(5, 35);
  StickCP2.Display.setTextColor(WHITE, BLACK);
  StickCP2.Display.println("[Bok] - Wyslij Mute / Power");
  StickCP2.Display.println("[M5]  - Wyjscie");

  // Przykładowa surowa ramka IR w protokole NEC (Power/Mute dla większości TV)
  uint16_t necMuteSignal[] = {
    9000, 4500, 560, 560, 560, 560, 560, 1690, 560, 560,
    560, 560, 560, 560, 560, 560, 560, 560, 560, 1690,
    560, 1690, 560, 560, 560, 1690, 560, 1690, 560, 1690,
    560, 1690, 560, 560, 560, 1690, 560, 1690, 560, 560,
    560, 560, 560, 560, 560, 560, 560, 560, 560, 1690,
    560, 1690, 560, 1690, 560, 1690, 560, 1690, 560, 1690,
    560, 1690, 560
  };
  int signalLen = sizeof(necMuteSignal) / sizeof(necMuteSignal[0]);

  while (true) {
    StickCP2.update();

    if (StickCP2.BtnB.wasPressed()) {
      StickCP2.Display.fillRect(5, 70, 200, 20, BLACK);
      StickCP2.Display.setCursor(5, 70);
      StickCP2.Display.setTextColor(GREEN, BLACK);
      StickCP2.Display.println(">>> Wysylam IR! <<<");
      
      // Fizyczna aktywacja LED podczerwieni
      sendIRRaw(necMuteSignal, signalLen);
      delay(200);
      
      StickCP2.Display.fillRect(5, 70, 200, 20, BLACK);
    }

    if (StickCP2.BtnA.wasPressed()) break;
    delay(50);
  }
}

// 4. INFORMACJE O BATERII I SYSTEMIE
void runSystemInfo() {
  while (true) {
    StickCP2.update();

    StickCP2.Display.fillScreen(BLACK);
    StickCP2.Display.setCursor(5, 5);
    StickCP2.Display.setTextColor(GREEN, BLACK);
    StickCP2.Display.setTextSize(2);
    StickCP2.Display.println("SYSTEM INFO");

    StickCP2.Display.setTextSize(1);
    StickCP2.Display.setTextColor(WHITE, BLACK);

    // Odczyt napięcia baterii
    int mvolts = StickCP2.Power.getBatteryVoltage();
    StickCP2.Display.setCursor(5, 35);
    StickCP2.Display.printf("Bateria: %d mV\n", mvolts);

    // Pamięć RAM
    StickCP2.Display.setCursor(5, 50);
    StickCP2.Display.printf("Free RAM: %d KB\n", ESP.getFreeHeap() / 1024);

    // Czas pracy
    StickCP2.Display.setCursor(5, 65);
    StickCP2.Display.printf("Uptime: %lu s\n", millis() / 1000);

    StickCP2.Display.setCursor(5, 115);
    StickCP2.Display.setTextColor(RED, BLACK);
    StickCP2.Display.println("Wcisnij [M5] aby wrocic");

    if (StickCP2.BtnA.wasPressed()) break;
    delay(200);
  }
}
