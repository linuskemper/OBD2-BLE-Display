#include <Wire.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEClient.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Display Parameter
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// BLE-UUIDs ([UUID ist eine eindeutige Identifikationsnummer])
static BLEUUID serviceUUID("FFF0");
static BLEUUID writeUUID("FFF2");
static BLEUUID notifyUUID("FFF1");

// Name des OBD2-Steckers
static const char* obdDeviceName = "VEEPEAK";

// Globale Pointer für Charakteristiken
BLEClient*               pClient    = nullptr;
BLERemoteCharacteristic* pWriteChar = nullptr;
BLERemoteCharacteristic* pNotifyChar= nullptr;

// BLE-Status
static bool deviceConnected = false;

// Letzte empfangene Nachricht
static std::string lastBLEMessage = "";

// Temperaturwerte
float oilTemp = 0.0;
float waterTemp = 0.0;

/******************************************************************************
 * NOTIFY-CALLBACK-FUNKTION (statt Klassen-Callback)
 *****************************************************************************/
void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify
) {
  std::string rxData((char*)pData, length);
  lastBLEMessage += rxData;

  Serial.print("[NOTIFY] Empfangene Daten: ");
  Serial.println(lastBLEMessage.c_str());

  if (lastBLEMessage.find("\r>") != std::string::npos) {
    Serial.println("[DEBUG] Nachricht vollständig empfangen!");

    // Bereinige die Nachricht
    lastBLEMessage = cleanMessage(lastBLEMessage);

    Serial.print("[DEBUG] Bereinigte Nachricht: ");
    Serial.println(lastBLEMessage.c_str());
  }
}

/******************************************************************************
 * Funktion zum Senden eines Befehls an den OBD2-Adapter
 *****************************************************************************/
void sendOBDCommand(String cmd) {
  if (pWriteChar == nullptr) {
    Serial.println("[ERROR] pWriteChar == NULL, kann Befehl nicht senden!");
    return;
  }
  Serial.print("[INFO] Sende Befehl: ");
  Serial.println(cmd);

  pWriteChar->writeValue(cmd.c_str(), cmd.length());
}

/******************************************************************************
 * Funktion zum Aufbau der BLE-Verbindung
 *****************************************************************************/
bool connectToOBD() {
  Serial.println("[DEBUG] Starte BLE Initialisierung...");
  BLEDevice::init("");

  // Scanner holen
  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setActiveScan(true);

  // 5 Sekunden scannen
  BLEScanResults* pFoundDevices = pBLEScan->start(5); 
  if (pFoundDevices == nullptr) {
    Serial.println("[ERROR] BLEScan lieferte kein Ergebnis!");
    return false;
  }

  int count = pFoundDevices->getCount();
  Serial.print("[DEBUG] Gefundene BLE-Geräte: ");
  Serial.println(count);

  // Durchsuchen aller gefundenen Geräte
  for (int i = 0; i < count; i++) {
    BLEAdvertisedDevice d = pFoundDevices->getDevice(i);

    // Debug: Name jedes gefundenen Gerätes ausgeben
    if (d.haveName()) {
      Serial.print("[DEBUG] Gefunden: ");
      Serial.println(d.getName().c_str());
    }

    // Wenn Name == "VEEPEAK", dann versuchen zu verbinden
    if (d.haveName() && d.getName() == obdDeviceName) {
      Serial.println("[DEBUG] VEEPEAK gefunden. Verbinde...");
      pClient = BLEDevice::createClient();
      if (!pClient->connect(&d)) {
        Serial.println("[ERROR] Konnte nicht mit VEEPEAK verbinden!");
        return false;
      }
      Serial.println("[DEBUG] Verbindung hergestellt.");

      // Service mit UUID "FFF0" suchen
      BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
      if (pRemoteService == nullptr) {
        Serial.println("[ERROR] Service FFF0 nicht gefunden!");
        return false;
      }

      // Write-Charakteristik (FFF2)
      pWriteChar = pRemoteService->getCharacteristic(writeUUID);
      // Notify-Charakteristik (FFF1)
      pNotifyChar = pRemoteService->getCharacteristic(notifyUUID);

      if (pWriteChar == nullptr || pNotifyChar == nullptr) {
        Serial.println("[ERROR] Write/Notify-Characteristic nicht gefunden!");
        return false;
      }

      // Notify einschalten
      if (pNotifyChar->canNotify()) {
        pNotifyChar->registerForNotify(notifyCallback);
        Serial.println("[DEBUG] Notify Callback registriert.");
      }

      deviceConnected = true;
      return true; // Erfolg
    }
  }

  // Wenn kein Gerät mit Namen "VEEPEAK" gefunden
  Serial.println("[ERROR] Kein VEEPEAK-Gerät gefunden!");
  return false;
}

// Funktion zur Ausgabe aller Zeichen und ihrer ASCII-Werte
void printMessageDetails(const std::string& message) {
  Serial.println("[DEBUG] Nachricht Details:");
  for (size_t i = 0; i < message.length(); ++i) {
    char c = message[i];
    Serial.print("Zeichen: '");
    if (isprint(c)) { // Wenn das Zeichen druckbar ist
      Serial.print(c);
    } else {
      Serial.print("<nicht druckbar>");
    }
    Serial.print("' ASCII: ");
    Serial.println((int)c); // ASCII-Wert anzeigen
  }
}

std::string cleanMessage(const std::string& message) {
  std::string cleaned;
  for (char c : message) {
    if (isprint(c) && c != ' ' && c != '>') { // Nur druckbare Zeichen, keine Leerzeichen oder `>`
      cleaned += c;
    }
  }
  return cleaned;
}

std::string extractLastByte(const std::string& message) {
  if (message.length() >= 2) { // Prüfe, ob die Nachricht mindestens 2 Zeichen hat
    return message.substr(message.length() - 2); // Die letzten zwei Zeichen extrahieren
  }
  return ""; // Kein gültiger Byte gefunden
}

/******************************************************************************
 * setup()
 *****************************************************************************/
void setup() {
  Serial.begin(115200);
  delay(2000); // kleiner Delay fürs Hochfahren

  // Display initialisieren
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("[ERROR] SSD1306 nicht gefunden!");
    for (;;); // hängen bleiben
  }

  // Begrüßung auf dem Display
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Verbinde mit VEEPEAK...");
  display.display();

  // BLE-Verbindung herstellen
  if (!connectToOBD()) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Verbindung fehlgeschl.");
    display.display();
    return;
  }

  // AT-Befehle senden
  sendOBDCommand("ATZ\r");
  delay(1000);
  sendOBDCommand("ATSP6\r");
  delay(1000);
  sendOBDCommand("ATD\r");
  delay(1000);

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Verbunden. Init...");
  display.display();
}

/******************************************************************************
 * loop()
 *****************************************************************************/
void loop() {
  if (!deviceConnected) {
    Serial.println("[INFO] Gerät nicht verbunden. Warte...");
    delay(2000);
    return;
  }

  // Abfrage: Kühlmitteltemperatur
  sendOBDCommand("0105\r");
  delay(1000); // Warten auf Antwort
  if (!lastBLEMessage.empty()) {
    std::string lastByte = extractLastByte(lastBLEMessage);
    if (!lastByte.empty()) {
      int temp = (int)strtol(lastByte.c_str(), NULL, 16) - 40; // HEX -> Dezimal, dann -40
      waterTemp = temp;
      Serial.print("[DEBUG] Kühlmitteltemperatur: ");
      Serial.print(lastByte.c_str());
      Serial.print(" (HEX) -> ");
      Serial.print(temp);
      Serial.println(" °C");
    } else {
      Serial.println("[ERROR] Kein gültiger HEX-Wert für Kühlmitteltemperatur gefunden!");
    }

    // Nachricht nach Verarbeitung zurücksetzen
    lastBLEMessage.clear();
  }

  // Abfrage: Öltemperatur
  sendOBDCommand("015C\r");
  delay(1000); // Warten auf Antwort
  if (!lastBLEMessage.empty()) {
    std::string lastByte = extractLastByte(lastBLEMessage);
    if (!lastByte.empty()) {
      int temp = (int)strtol(lastByte.c_str(), NULL, 16) - 40; // HEX -> Dezimal, dann -40
      oilTemp = temp;
      Serial.print("[DEBUG] Öltemperatur: ");
      Serial.print(lastByte.c_str());
      Serial.print(" (HEX) -> ");
      Serial.print(temp);
      Serial.println(" °C");
    } else {
      Serial.println("[ERROR] Kein gültiger HEX-Wert für Öltemperatur gefunden!");
    }

    // Nachricht nach Verarbeitung zurücksetzen
    lastBLEMessage.clear();
  }

  // Display aktualisieren
  display.clearDisplay();

  // Überschriften
  display.setTextSize(1);
  display.setCursor(0, 2);
  display.println("Oil");
  display.setCursor(68, 2);
  display.println("Water");

  // Temperaturen
  display.setTextSize(2);
  display.setCursor(0, 18);
  display.printf("%.0f", oilTemp);
  display.setCursor(68, 18);
  display.printf("%.0f", waterTemp);

  display.display();

  delay(2000); // Zeit für nächste Abfrage
}