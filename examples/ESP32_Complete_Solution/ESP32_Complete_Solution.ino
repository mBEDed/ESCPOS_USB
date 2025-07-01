/*
 * ESP32-WROOM + MAX3421E + ESC/POS Thermal Printer - Complete Solution
 * 
 * Hardware Connections:
 * ====================
 * ESP32 WROOM (VSPI) → MAX3421E Shield
 * GPIO23 (MOSI)      → MOSI
 * GPIO19 (MISO)      → MISO  
 * GPIO18 (SCK)       → SCK
 * GPIO5  (CS/SS)     → CS/SS
 * 3.3V               → VCC (or 5V if shield requires it)
 * GND                → GND
 * 
 * MAX3421E Shield → Thermal Printer
 * USB Host Port     → USB Cable → Thermal Printer
 * 
 * Power Requirements:
 * ==================
 * - ESP32: 3.3V (via USB or external)
 * - MAX3421E Shield: 3.3V or 5V (check your shield)
 * - Thermal Printer: Usually USB powered, but external 5V recommended
 * 
 * Required Libraries:
 * ==================
 * 1. USB Host Shield Library 2.0 (by felis)
 * 2. This ESCPOS_USB library
 * 
 * Installation:
 * =============
 * Arduino IDE → Library Manager → Search "USB Host Shield Library 2.0" → Install
 * This library should be in your libraries folder already
 */

#include "USBPrinter.h"
#include "ESC_POS_Printer.h"

// USB Host and Printer setup
class PrinterOper : public USBPrinterAsyncOper {
  public:
    uint8_t OnInit(USBPrinter *pPrinter);
};

uint8_t PrinterOper::OnInit(USBPrinter *pPrinter) {
  Serial.println(F("=== USB Printer Detected ==="));
  Serial.print(F("Bidirectional: "));
  Serial.println(pPrinter->isBidirectional() ? "Yes" : "No");
  Serial.println(F("Printer initialization complete!"));
  return 0;
}

USB myusb;
PrinterOper AsyncOper;
USBPrinter uprinter(&myusb, &AsyncOper);
ESC_POS_Printer printer(&uprinter);

// Test variables
bool printerTestDone = false;
unsigned long lastStatusCheck = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 5000) delay(10); // Wait up to 5 seconds for serial
  
  Serial.println(F("===================================="));
  Serial.println(F("ESP32 + MAX3421E + Thermal Printer"));
  Serial.println(F("===================================="));
  Serial.println();
  
  // Hardware connection verification
  Serial.println(F("Hardware Setup:"));
  Serial.println(F("ESP32 VSPI → MAX3421E Shield"));
  Serial.println(F("  GPIO23 → MOSI"));
  Serial.println(F("  GPIO19 → MISO"));
  Serial.println(F("  GPIO18 → SCK"));
  Serial.println(F("  GPIO5  → CS/SS"));
  Serial.println(F("  3.3V   → VCC"));
  Serial.println(F("  GND    → GND"));
  Serial.println();
  
  // Initialize USB Host
  Serial.println(F("Initializing USB Host Shield..."));
  if (myusb.Init() == -1) {
    Serial.println(F("❌ USB Host Shield initialization FAILED!"));
    Serial.println(F("Check connections:"));
    Serial.println(F("  - SPI wiring correct?"));
    Serial.println(F("  - Shield powered properly?"));
    Serial.println(F("  - Compatible MAX3421E shield?"));
    while(1) {
      delay(1000);
      Serial.println(F("System halted. Fix hardware and reset."));
    }
  }
  
  Serial.println(F("✓ USB Host Shield initialized successfully!"));
  Serial.println(F("Waiting for thermal printer connection..."));
  Serial.println(F("Please connect your ESC/POS thermal printer via USB."));
  Serial.println();
}

void loop() {
  myusb.Task(); // Essential: Handle USB communication
  
  // Check printer status every second
  if (millis() - lastStatusCheck > 1000) {
    lastStatusCheck = millis();
    checkPrinterStatus();
  }
  
  // Run printer test when ready
  if (uprinter.isReady() && !printerTestDone) {
    Serial.println(F("🎉 Printer connected and ready!"));
    delay(1000); // Give printer time to fully initialize
    runPrinterTest();
    printerTestDone = true;
  }
}

void checkPrinterStatus() {
  uint8_t usbState = myusb.getUsbTaskState();
  
  // Print status occasionally to show we're alive
  static int statusCounter = 0;
  if (++statusCounter >= 10) { // Every 10 seconds
    statusCounter = 0;
    Serial.print(F("USB State: 0x"));
    Serial.print(usbState, HEX);
    
    switch(usbState) {
      case 0x0C: Serial.println(F(" (Waiting for device)")); break;
      case 0x0B: Serial.println(F(" (Configuring device)")); break;
      case 0x0D: Serial.println(F(" (Running)")); break;
      case 0x0E: Serial.println(F(" (Error state)")); break;
      default: Serial.println(F(" (Other state)")); break;
    }
    
    if (usbState == 0x0C) {
      Serial.println(F("💡 Tip: Ensure thermal printer is connected and powered on"));
    }
  }
  
  // Check for printer readiness
  if (uprinter.isReady()) {
    if (statusCounter == 0) { // Don't spam this message
      Serial.println(F("✓ Printer is ready for commands"));
    }
  }
}

void runPrinterTest() {
  Serial.println(F("Starting comprehensive printer test..."));
  
  // Initialize printer
  printer.begin();
  printer.reset();
  printer.setDefault();
  
  Serial.println(F("Sending test print job..."));
  
  // Header
  printer.setSize('L');
  printer.boldOn();
  printer.justify('C');
  printer.println(F("ESP32 PRINTER TEST"));
  printer.boldOff();
  printer.setSize('M');
  printer.println(F("MAX3421E USB Host"));
  printer.setSize('S');
  printer.println(F("ESC/POS Thermal Printer"));
  printer.feed(2);
  
  // Connection info
  printer.justify('L');
  printer.println(F("Hardware: ESP32-WROOM"));
  printer.println(F("Shield: MAX3421E USB Host"));
  printer.println(F("Protocol: ESC/POS"));
  printer.feed(1);
  
  // Test different text styles
  printer.boldOn();
  printer.println(F("Bold Text Test"));
  printer.boldOff();
  
  printer.underlineOn();
  printer.println(F("Underlined Text Test"));
  printer.underlineOff();
  
  printer.inverseOn();
  printer.println(F("Inverse Text Test"));
  printer.inverseOff();
  
  // Test different sizes
  printer.setSize('S');
  printer.println(F("Small text size"));
  printer.setSize('M');
  printer.println(F("Medium text size"));
  printer.setSize('L');
  printer.println(F("Large text size"));
  printer.setSize('S'); // Reset to small
  
  // Test justification
  printer.justify('L');
  printer.println(F("Left justified text"));
  printer.justify('C');
  printer.println(F("Center justified"));
  printer.justify('R');
  printer.println(F("Right justified"));
  printer.justify('L'); // Reset to left
  
  // Test line spacing
  printer.println(F("Normal line spacing"));
  printer.setLineHeight(50);
  printer.println(F("Increased"));
  printer.println(F("line spacing"));
  printer.setLineHeight(); // Reset to default
  
  // Footer with timestamp
  printer.feed(1);
  printer.justify('C');
  printer.setSize('S');
  printer.print(F("Test completed: "));
  printer.print(millis() / 1000);
  printer.println(F(" seconds"));
  printer.feed(3); // Extra paper for tearing
  
  Serial.println(F("✓ Print test completed successfully!"));
  Serial.println(F("Check your thermal printer for output."));
  Serial.println();
  Serial.println(F("To run another test, reset the ESP32."));
}

// Additional utility function for manual commands
void sendManualCommand() {
  // This function can be called to send custom ESC/POS commands
  // Example: printer.write("\x1B\x64\x05"); // Feed 5 lines
}

/* 
 * Troubleshooting Tips:
 * ====================
 * 
 * 1. No printer detected:
 *    - Check SPI wiring (especially CS pin on GPIO5)
 *    - Verify shield power (3.3V or 5V as required)
 *    - Try different USB cable
 *    - Ensure printer is ESC/POS compatible
 * 
 * 2. Printer detected but no output:
 *    - Check printer power (use external 5V if needed)
 *    - Verify paper is loaded correctly
 *    - Try printer self-test (usually hold feed button on power-up)
 *    - Check if printer is in correct mode
 * 
 * 3. Garbled output:
 *    - Check baud rate settings
 *    - Verify ESC/POS command compatibility
 *    - Try different character encoding
 * 
 * 4. USB State stuck at configuring:
 *    - Power cycle both ESP32 and printer
 *    - Try different MAX3421E shield
 *    - Check for USB Host Shield library version compatibility
 * 
 * 5. For debugging, enable DEBUG_USB_HOST in your USB Host Shield library
 */
