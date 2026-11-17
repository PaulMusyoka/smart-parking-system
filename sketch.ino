// ==========================================================
//   SMARTPARK — Smart Parking System
//   0-15 cm  → RED LED + continuous beeping
//   16-100 cm → RED LED, silent
//   101+ cm  → GREEN LED, silent
// ==========================================================

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <HTTPClient.h>

const char* TS_API_KEY  = "HUNAN60OBQINM7EP";
const unsigned long TS_INTERVAL = 20000;

const char* ssid     = "Wokwi-GUEST";
const char* password = "";

const int trigPin     = 5;
const int echoPin     = 15;
const int greenLedPin = 2;
const int redLedPin   = 4;
const int buzzerPin   = 23;

LiquidCrystal_I2C lcd(0x27, 16, 2);

const int parkingThreshold = 100;
const int proximityLimit   = 15;
const int maxValidDistance = 400;

long duration;
int  distance = 400;
bool occupied = false;
unsigned long lastSensorRead = 0;
unsigned long lastTSUpload  = 0;

String cloudMsg = "Cloud: Idle";
unsigned long cloudMsgUntil = 0;

void showCloudMessage(String msg, int durationMs = 3000) {
  cloudMsg = msg;
  cloudMsgUntil = millis() + durationMs;
}

void uploadToThingSpeak() {
  if (WiFi.status() != WL_CONNECTED) {
    showCloudMessage("Cloud: No WiFi", 3000);
    return;
  }
  showCloudMessage("Cloud: Sending...", 2000);

  String url = "http://api.thingspeak.com/update?api_key=";
  url += TS_API_KEY;
  url += "&field1=" + String(distance);
  url += "&field2=" + String(occupied ? 1 : 0);

  HTTPClient http;
  http.begin(url);
  http.setTimeout(10000);
  int code = http.GET();
  String response = http.getString();
  http.end();

  if (code == 200 && response.toInt() > 0) {
    showCloudMessage("Cloud OK #" + response, 4000);
  } else if (response == "0") {
    showCloudMessage("Cloud: RateLimit", 4000);
  } else {
    showCloudMessage("Cloud FAIL " + String(code), 4000);
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(trigPin,     OUTPUT);
  pinMode(echoPin,     INPUT);
  pinMode(greenLedPin, OUTPUT);
  pinMode(redLedPin,   OUTPUT);
  pinMode(buzzerPin,   OUTPUT);

  Wire.begin(21, 22);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0); lcd.print("SMART PARKING");
  lcd.setCursor(0, 1); lcd.print("WiFi setup...");

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password, 6);

  int tries = 0;
  while (WiFi.status() != WL_CONNECTED && tries < 60) {
    delay(500); tries++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    lcd.clear();
    lcd.setCursor(0, 0); lcd.print("WiFi OK");
    lcd.setCursor(0, 1); lcd.print(WiFi.localIP());
    delay(2000);
  } else {
    lcd.clear();
    lcd.setCursor(0, 0); lcd.print("WiFi FAILED");
    lcd.setCursor(0, 1); lcd.print("Offline mode");
    delay(2000);
  }

  tone(buzzerPin, 1000, 150); delay(200);
  tone(buzzerPin, 1500, 150);
  lcd.clear();
  lastTSUpload = millis() - TS_INTERVAL;
}

void loop() {
  if (millis() - lastSensorRead >= 800) {
    lastSensorRead = millis();

    digitalWrite(trigPin, LOW);  delayMicroseconds(2);
    digitalWrite(trigPin, HIGH); delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    duration = pulseIn(echoPin, HIGH, 30000);
    distance = duration * 0.034 / 2;
    if (distance <= 0 || distance > maxValidDistance) distance = maxValidDistance;

    // ---- Occupancy decision ----
    bool newOccupied = (distance < parkingThreshold);
    if (newOccupied != occupied) {
      if (newOccupied) {
        tone(buzzerPin, 1200, 200); delay(220);
        tone(buzzerPin, 800,  200); delay(220);
      } else {
        tone(buzzerPin, 800,  200); delay(220);
        tone(buzzerPin, 1200, 200); delay(220);
      }
      occupied = newOccupied;
    }

    // ---- LEDs ----
    digitalWrite(redLedPin,   occupied ? HIGH : LOW);
    digitalWrite(greenLedPin, occupied ? LOW  : HIGH);

    // ---- LCD ----
    lcd.setCursor(0, 0);
    lcd.print("Dist:"); lcd.print(distance); lcd.print("cm ");
    lcd.print(WiFi.status() == WL_CONNECTED ? "WiFi" : "OFF ");

    lcd.setCursor(0, 1);
    if (millis() < cloudMsgUntil) {
      String line = cloudMsg;
      while (line.length() < 16) line += " ";
      lcd.print(line.substring(0, 16));
    } else {
      lcd.print(occupied ? "OCCUPIED  [XX]" : "AVAILABLE [  ]");
    }

    Serial.print(distance);
    Serial.print(" cm | ");
    Serial.println(occupied ? "OCCUPIED" : "AVAILABLE");
  }

  // ============================================================
  //   CONTINUOUS BEEPING when distance < 15 cm
  //   Runs outside the 800ms block so beeps are rapid
  // ============================================================
  if (distance > 0 && distance < proximityLimit) {
    tone(buzzerPin, 2000, 80);   // 80 ms beep
    delay(120);                  // 120 ms gap
  } else {
    noTone(buzzerPin);           // silent otherwise
  }

  // ---- ThingSpeak upload ----
  if (millis() - lastTSUpload >= TS_INTERVAL) {
    lastTSUpload = millis();
    uploadToThingSpeak();
  }
}