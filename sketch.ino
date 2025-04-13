// -----------------------------
// Shift Register Pin Definitions
// -----------------------------
const int datapin = 18;    // SER (serial data input) on 74HC595
const int latchpin = 19;   // RCLK (storage register latch)
const int clockpin = 17;   // SRCLK (shift register clock)

// -----------------------------
// Matrix Row Pins (Input Lines)
// -----------------------------
// These GPIOs read from the matrix rows.
// Each row pin is connected to a key row through a diode.
const int rowPins[9] = {34, 35, 32, 33, 25, 26, 27, 13, 4};

// -----------------------------
// Shift Register Output Signals (1 active column at a time)
// -----------------------------
// These values are shifted into the 74HC595 to activate one column line at a time.
// Only one bit is HIGH per value.
const uint8_t columnSignals[8] = {
  0b00000001,  // Column 0
  0b00000010,  // Column 1
  0b00000100,  // Column 2
  0b00001000,  // Column 3
  0b00010000,  // Column 4
  0b00100000,  // Column 5
  0b01000000,  // Column 6
  0b10000000   // Column 7
};

// -----------------------------
// Matrix State: matrix[column][row]
// -----------------------------
// Stores the state (0 or 1) for each key position
int matrix[8][9];

// -----------------------------
// Timing Variables for Printing
// -----------------------------
unsigned long lastPrintTime = 0;
const unsigned long printInterval = 1000;  // 1 second

// -----------------------------
// Setup Function
// -----------------------------
void setup() {
  Serial.begin(115200);  // Start serial communication at 115200 baud

  // Set shift register pins as OUTPUT
  pinMode(datapin, OUTPUT);
  pinMode(clockpin, OUTPUT);
  pinMode(latchpin, OUTPUT);

  // Configure row input pins with internal pull-down resistors
  // Ensures unconnected inputs stay LOW (0) unless driven HIGH by key press
  for (int i = 0; i < 9; i++) {
    pinMode(rowPins[i], INPUT_PULLDOWN);
  }

  // Clear all column outputs initially
  clearAllColumns();
}

// -----------------------------
// Clear All Columns (Deactivate All)
// -----------------------------
void clearAllColumns() {
  // Send 0 to shift register to deactivate all column lines
  digitalWrite(latchpin, LOW);
  shiftOut(datapin, clockpin, LSBFIRST, 0b00000000);
  digitalWrite(latchpin, HIGH);
}

// -----------------------------
// Activate One Column
// -----------------------------
// Only one column should be active (HIGH) at a time.
void activateColumn(uint8_t colValue) {
  digitalWrite(latchpin, LOW);
  shiftOut(datapin, clockpin, LSBFIRST, colValue);
  digitalWrite(latchpin, HIGH);
}

// -----------------------------
// Main Loop
// -----------------------------
void loop() {
  // -----------------------------
  // Scan the Entire Matrix
  // -----------------------------
  for (int col = 0; col < 8; col++) {
    // Turn on only one column
    activateColumn(columnSignals[col]);

    // Allow time for signals to settle
    delayMicroseconds(30);

    // Read all 9 row pins for this column
    for (int row = 0; row < 9; row++) {
      matrix[col][row] = digitalRead(rowPins[row]);
    }
  }

  // After scanning, turn off all columns
  clearAllColumns();

  // -----------------------------
  // Print Matrix Once Every 1 Second
  // -----------------------------
  if (millis() - lastPrintTime >= printInterval) {
    lastPrintTime = millis();

    Serial.println("Matrix (Rows Vertically, Cols Horizontally):");

    // Print column headers
    Serial.print("     ");
    for (int col = 0; col < 8; col++) {
      Serial.print("C"); Serial.print(col); Serial.print(" ");
    }
    Serial.println();

    // Print each row with values for every column
    for (int row = 0; row < 9; row++) {
      Serial.print("R"); Serial.print(row); Serial.print(":  ");
      for (int col = 0; col < 8; col++) {
        Serial.print(" ");
        Serial.print(matrix[col][row]);  // 1 = key pressed, 0 = no press
        Serial.print(" ");
      }
      Serial.println();
    }

    Serial.println("------------------------");
  }
}
