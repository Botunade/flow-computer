#include <Arduino.h>
#include <Wire.h>               // For I2C (ADS1115, RTC)
#include <SPI.h>                // For SD Card
#include <Adafruit_ADS1X15.h>

// ==========================================
// 1. GLOBAL VARIABLES & SETTINGS
// ==========================================
// Physical Readings (Updated every 100ms)
float static_pressure = 0.0;
float diff_pressure = 0.0;
float gas_temp = 0.0;

// Flow Variables (Calculated)
float flow_rate = 0.0;
float totalizer = 0.0;

// Calibration Settings (Editable from HMI Screen)
float base_pressure = 1.01325;
float specific_gravity = 0.60;
float pipe_diameter = 4.026;  // inches
float orifice_diameter = 2.0; // inches

// Timer Variables for Non-Blocking Loops
unsigned long prev_sensor_time = 0;
unsigned long prev_hmi_time = 0;
unsigned long prev_log_time = 0;

// Hardware Objects
Adafruit_ADS1115 ads;

// ==========================================
// UTILITY FUNCTIONS
// ==========================================
// A custom function to map decimal (float) numbers, since the standard Arduino map() only does whole integers
float mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
  if (x < in_min) x = in_min; // Prevent negative readings if the sensor drops below 0.66V
  if (x > in_max) x = in_max; // Prevent overflow if the sensor spikes above 3.3V
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// ==========================================
// TASK FUNCTIONS
// ==========================================

void setupSensors() {
  // Initialize the ADS1115
  // GAIN_ONE sets the voltage reading limit to +/- 4.096V (Perfect for our 3.3V max)
  ads.setGain(GAIN_ONE);

  // Try to initialize I2C on standard pins (SDA=21, SCL=22 for typical ESP32, or default for S3)
  if (!ads.begin()) {
    Serial.println("Failed to initialize ADS1115. Check I2C wiring!");
    // Here you would trigger your yellow fault LED
  } else {
    Serial.println("ADS1115 Initialized Successfully.");
  }
}

void readSensors() {
  // 1. Read the raw 16-bit values from the ADS1115 channels
  int16_t adc0 = ads.readADC_SingleEnded(0); // Static Pressure
  int16_t adc1 = ads.readADC_SingleEnded(1); // Differential Pressure
  int16_t adc2 = ads.readADC_SingleEnded(2); // Temperature

  // 2. Convert the raw 16-bit ADC values into actual Volts
  // At GAIN_ONE, 1 bit = 0.125 mV (or 0.000125 Volts)
  float volts0 = adc0 * 0.000125;
  float volts1 = adc1 * 0.000125;
  float volts2 = adc2 * 0.000125;

  // 3. Map the Voltages (0.66V - 3.3V) to Physical Ranges
  // *Adjust the 'out_max' numbers below to match the max range printed on your actual physical transmitters*

  // Example: Static transmitter is 0 to 100 Bar
  static_pressure = mapFloat(volts0, 0.66, 3.3, 0.0, 100.0);

  // Example: DP transmitter is 0 to 1000 mBar (1.0 Bar)
  diff_pressure = mapFloat(volts1, 0.66, 3.3, 0.0, 1.0);

  // Example: Temp transmitter is 0 to 150 Celsius
  gas_temp = mapFloat(volts2, 0.66, 3.3, 0.0, 150.0);

  Serial.println("--- SENSORS ---");
  Serial.printf("Static Pressure: %.2f Bar\n", static_pressure);
  Serial.printf("Diff Pressure: %.2f Bar\n", diff_pressure);
  Serial.printf("Gas Temp: %.2f C\n", gas_temp);
}

void setupHMI() {
  Serial.println("Init HMI Serial Link on GPIO 16/17...");
  // Start Serial1 at 115200 baud rate.
  // We will use GPIO 16 (RX) and GPIO 17 (TX) to connect to the Sunton screen.
  Serial1.begin(115200, SERIAL_8N1, 16, 17);
}

void communicateWithHMI() {
  // ==========================================
  // PART 1: SEND DATA TO THE SCREEN
  // ==========================================

  // Create a character array to hold our formatted string (packet)
  char tx_buffer[100];

  // Pack the variables into a clean string wrapped in < > brackets.
  // %.2f means format the float to 2 decimal places.
  sprintf(tx_buffer, "<FLOW:%.2f,STAT:%.2f,DP:%.2f,TEMP:%.2f,TOT:%.0f>",
          flow_rate, static_pressure, diff_pressure, gas_temp, totalizer);

  // Send the string over the wires to the HMI
  Serial1.println(tx_buffer);

  // Print to your laptop for debugging so you can see what is being sent
  Serial.print("[TX to HMI]: ");
  Serial.println(tx_buffer);


  // ==========================================
  // PART 2: RECEIVE COMMANDS FROM THE SCREEN
  // ==========================================

  // Check if the screen sent any data back (like a calibration update)
  if (Serial1.available() > 0) {
    // Read the incoming text until the newline character
    String incomingMessage = Serial1.readStringUntil('\n');
    incomingMessage.trim(); // Remove any invisible spaces or return carriages

    Serial.print("[RX from HMI]: ");
    Serial.println(incomingMessage);

    // Example: If the operator hits the "Zero Totalizer" button, the screen sends "<ZERO_TOT>"
    if (incomingMessage == "<ZERO_TOT>") {
      totalizer = 0.0;
      Serial.println("[SYSTEM]: Totalizer reset by HMI command.");
    }

    // You can add more listeners here later for saving new Orifice or Pipe diameters
  }
}

void calculateFlow() {
  // Run the AGA-3 or basic volumetric math using current readings and calibration settings
  // Add to Totalizer
  // Pass variables to TinyML AI model to check for drift/anomalies
  Serial.println("--- FLOW MATH ---");
  Serial.printf("Calculated Flow: %.2f\n", flow_rate);
  Serial.printf("Updated Totalizer: %.0f\n", totalizer);
}

void logToSDCard() {
  // Get time from RTC
  // Append a new CSV line: Date, Time, Static, DP, Temp, Flow
  Serial.println("[SYSTEM]: Appending log row to SD Card...");
}


// ==========================================
// 2. HARDWARE SETUP
// ==========================================
void setup() {
  Serial.begin(115200);       // Debugging to your laptop
  while(!Serial) { ; }
  Serial.println("ESP32-S3 Flow Computer Booting...");

  setupHMI();                 // UART connection to Sunton HMI

  // TODO: Load saved Calibration Settings from ESP32 flash memory

  setupSensors();             // Start I2C Bus and check ADS1115

  // TODO: Start SPI Bus and check SD Card
  // TODO: Start Wi-Fi Access Point
}


// ==========================================
// 4. THE MAIN LOOP (The State Machine)
// ==========================================
void loop() {
  unsigned long current_time = millis();

  // Task 1: Read sensors & calculate flow every 100 milliseconds
  if (current_time - prev_sensor_time >= 100) {
    readSensors();
    calculateFlow();
    prev_sensor_time = current_time;
  }

  // Task 2: Update the 7-inch screen every 500 milliseconds
  if (current_time - prev_hmi_time >= 500) {
    communicateWithHMI();
    prev_hmi_time = current_time;
  }

  // Task 3: Save data to SD Card every 5 seconds
  if (current_time - prev_log_time >= 5000) {
    logToSDCard();
    prev_log_time = current_time;
  }

  // Background Tasks: Keep Wi-Fi web server alive
  // wifi_server.handleClient();
}