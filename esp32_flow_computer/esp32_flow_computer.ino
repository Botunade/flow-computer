// ESP32-S3 Flow Computer Main Controller
// Version 1.0 - Basic Timer Skeleton

// ==========================================
// 1. HARDWARE INITIALIZATION (SETUP)
// ==========================================

void setup() {
  // Start Serial/UART for debugging and Sunton HMI screen
  Serial.begin(115200);
  while (!Serial) {
    ; // Wait for serial port to connect
  }
  Serial.println("ESP32-S3 Flow Computer Booting...");

  // TODO: Start I2C Bus (ADS1115, DS3231)
  Serial.println("Init I2C Bus: [PENDING]");

  // TODO: Start SPI Bus (Micro SD Card)
  // If fails, instantly turn on Yellow System Fault LED
  Serial.println("Init SPI Bus & SD Card: [PENDING]");

  // TODO: Start Wi-Fi (AP Mode)
  Serial.println("Init Wi-Fi AP Mode: [PENDING]");

  Serial.println("System Ready. Starting Main Loop.");
}

// ==========================================
// NON-BLOCKING TIMER VARIABLES
// ==========================================
unsigned long previousMillisBlock2 = 0; // Data Acquisition
const long intervalBlock2 = 100;        // 100 ms

unsigned long previousMillisBlock4 = 0; // HMI Comm Packer
const long intervalBlock4 = 500;        // 500 ms

unsigned long previousMillisBlock5 = 0; // Datalogging & Web
const long intervalBlock5 = 5000;       // 5 seconds

// ==========================================
// THE MAIN SUPERVISOR LOOP
// ==========================================
void loop() {
  unsigned long currentMillis = millis();

  // Block 2: Data Acquisition (Every 100ms)
  if (currentMillis - previousMillisBlock2 >= intervalBlock2) {
    previousMillisBlock2 = currentMillis;

    // TODO: Read ADS1115 channels
    // TODO: Convert to Static Pressure, DP, and Gas Temp

    // Block 3: The Flow Math & AI Supervisor
    // (This logic happens immediately after getting new data)
    // TODO: Calculate flow, update totalizer
    // TODO: Run TinyML Inference
    // TODO: Check Alarm Logic
  }

  // Block 4: The HMI Communication Packer (Every 500ms)
  if (currentMillis - previousMillisBlock4 >= intervalBlock4) {
    previousMillisBlock4 = currentMillis;

    // TODO: Pack variables into string and send over UART to screen
    // Example: <FLOW:124.5, STAT:6.0, DP:0.5, TEMP:32.5, TOT:423180>
  }

  // Block 5: Datalogging & Web Server (Every 5000ms)
  if (currentMillis - previousMillisBlock5 >= intervalBlock5) {
    previousMillisBlock5 = currentMillis;

    // TODO: Grab timestamp from RTC
    // TODO: Append CSV row to SD card
  }

  // The Web Server listening logic goes here in the main loop so it responds quickly
  // TODO: Handle Web Server client requests
}
