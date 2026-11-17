                              **SmartPark: IoT-Based Vehicle Presence Detection System Using ESP32 and Ultrasonic Ranging**


Author: Eng. Paul Musyoka

⚠️ Public demo note: This ThingSpeak channel (ID 3525895) is a public demo. The API key is included in sketch.ino for reproducibility, which means anyone who runs the code can write to this channel. The data is not tamper-proof and should be treated as a demonstration only — not as a reliable monitoring source.

Overview
SmartPark is an IoT-based smart parking system that detects whether a parking slot is occupied or free, displays the status locally, and logs the data to a live cloud dashboard. The system is built on an ESP32 DevKit-C V4 and simulated entirely in Wokwi.

An HC-SR04 ultrasonic sensor measures the distance to a vehicle every 800 ms. The distance is calculated as distance = (duration × 0.034) / 2. Based on the reading, the system applies three behaviour zones:

Distance	Meaning	Output
0–15 cm	Vehicle too close	Red LED + continuous rapid beeping
16–100 cm	Slot occupied	Red LED ON, silent
101+ cm	Slot available	Green LED ON, silent
A 16×2 I2C LCD shows the live distance, WiFi status, and slot state (OCCUPIED / AVAILABLE). A piezo buzzer gives audio feedback when a car enters or leaves. Every 20 seconds, the ESP32 connects to ThingSpeak over WiFi and uploads the distance (field1) and occupancy flag (field2) to a public cloud dashboard — proving end-to-end IoT capability.

Live Cloud Dashboard (ThingSpeak)
Link: https://thingspeak.mathworks.com/channels/3525895
Channel ID: 3525895 — public access, shows real-time distance and occupancy graphs streamed from the ESP32.

Components
ESP32 DevKit-C V4 (microcontroller)

HC-SR04 ultrasonic distance sensor

16×2 I2C LCD display

Green LED + 220Ω resistor (available indicator)

Red LED + 220Ω resistor (occupied indicator)

Piezo buzzer (state-change and proximity alert)

Breadboard

Pin Mapping
Component	Pin	ESP32
HC-SR04	VCC / GND	5V / GND
HC-SR04	TRIG	D5
HC-SR04	ECHO	D15
Green LED	Anode	D2
Red LED	Anode	D4
Buzzer	+	D23
LCD	SDA / SCL	D21 / D22
How to Test in Wokwi
Click ▶ Play.

Open the Serial Monitor at 115200 baud.

Click the HC-SR04 sensor → a distance slider appears.

Drag the slider:

Below 15 cm → red LED + rapid continuous beeping.

16–100 cm → red LED only, silent.

Above 100 cm → green LED only, silent.

Watch the LCD update locally and the ThingSpeak dashboard update every 20 seconds.

Files
sketch.ino — main firmware (WiFi + ThingSpeak + sensor logic)

diagram.json — Wokwi circuit

libraries.txt — required library (LiquidCrystal I2C)

Links
Simulation: https://wokwi.com/projects/477273563827573761

Dashboard: https://thingspeak.mathworks.com/channels/3525895

