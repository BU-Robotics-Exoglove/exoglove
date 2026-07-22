const int sensorPin = A0;   // pin connected to sensor
const float vRef = 5.0;     // reference voltage (5V for Uno WiFi Rev2)
const int adcResolution = 1023; // 10-bit ADC (0-1023)

void setup() {
  Serial.begin(9600);
}

void loop() {
  int rawValue = analogRead(sensorPin);          // raw 0-1023
  float voltage = (rawValue / (float)adcResolution) * vRef; // convert to volts

  Serial.print("Raw: ");
  Serial.print(rawValue);
  Serial.print("  |  Voltage: ");
  Serial.print(voltage, 3); // 3 decimal places
  Serial.println(" V");

  delay(250); // adjust delay as needed (ms)
}
