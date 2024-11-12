#include <WiFi.h>
#include <ThingSpeak.h>
#include <DHT.h>

// WiFi credentials
const char* ssid = "Chivi";
const char* password = "Pass4123";

// ThingSpeak credentials
WiFiClient client;
unsigned long myChannelNumber = 2737423;  // Replace with your ThingSpeak channel number
const char* myWriteAPIKey = "4JZZ5C1TE3YLFJGV";  // Replace with your ThingSpeak Write API key

// Soil Moisture Sensor pin
const int moistureSensorPin = 35; // Analog pin on ESP32

// DHT11 Sensor settings
#define DHTPIN 4    // GPIO pin connected to DHT11
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// PIR Sensor settings
const int pirPin = 12; // PIR sensor pin
const int ledPin = 13; // LED pin for indication

// MQ-7 Sensor settings
const int MQ7_PIN = 32; // Pin connected to MQ-7 analog output (A0)

// Variables
int moistureLevel;
bool motionDetected = false;
unsigned long startTime, endTime;

void setup() {
  Serial.begin(115200);  // Start Serial Monitor for debugging
  pinMode(pirPin, INPUT); // PIR sensor
  pinMode(ledPin, OUTPUT); // LED
  pinMode(MQ7_PIN, INPUT); // MQ-7 analog pin
  dht.begin();             // Initialize DHT sensor

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");

  // Initialize ThingSpeak
  ThingSpeak.begin(client);
}

void loop() {
  // Soil Moisture Sensor reading
  int rawMoistureValue = analogRead(moistureSensorPin);
  moistureLevel = map(rawMoistureValue, 4000, 800, 0, 100); // Adjust range if needed
  moistureLevel = constrain(moistureLevel, 0, 100);         // Constrain to 0-100%

  // DHT11 Sensor readings
  float temperature = dht.readTemperature(); // Celsius
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
  }

  // PIR Sensor motion detection
  int pirState = digitalRead(pirPin);
  if (pirState == HIGH && !motionDetected) {
    startTime = millis();
    motionDetected = true;
    digitalWrite(ledPin, HIGH); // Turn on LED
    Serial.println("Person detected!");
  } else if (pirState == LOW && motionDetected) {
    endTime = millis();
    motionDetected = false;
    digitalWrite(ledPin, LOW); // Turn off LED
    unsigned long timeSpent = endTime - startTime;
    Serial.print("Person left. Time spent: ");
    Serial.print(timeSpent / 1000.0);
    Serial.println(" seconds");
  }

  // MQ-7 Gas Sensor reading
  int mq7Value = analogRead(MQ7_PIN); // Read analog value from MQ-7 sensor

  // Debug output
  Serial.print("Raw Moisture Value: ");
  Serial.println(rawMoistureValue);
  Serial.print("Mapped Moisture Level (%): ");
  Serial.println(moistureLevel);
  Serial.print("Temperature (C): ");
  Serial.println(temperature);
  Serial.print("Humidity (%): ");
  Serial.println(humidity);
  Serial.print("MQ-7 Raw Analog Value: ");
  Serial.println(mq7Value);

  // Send data to ThingSpeak
  ThingSpeak.setField(1, moistureLevel); // Field 1 for soil moisture
  ThingSpeak.setField(2, temperature);   // Field 2 for temperature
  ThingSpeak.setField(3, humidity);      // Field 3 for humidity
  ThingSpeak.setField(4, mq7Value);      // Field 4 for gas sensor (MQ-7)

  int response = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey); // Write data to ThingSpeak

  if (response == 200) {
    Serial.println("Data successfully sent to ThingSpeak.");
  } else {
    Serial.print("Problem sending data. HTTP error code: ");
    Serial.println(response);
  }

  delay(15000); // ThingSpeak rate limit (15 seconds minimum for free accounts)
}