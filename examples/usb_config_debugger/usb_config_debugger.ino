/*
 * USB Configuration Debugger
 * Tracks state transitions and configuration attempts in detail
 */

#include "Usb.h"
#include "usbhub.h"

USB Usb;
USBHub Hub1(&Usb);

// State tracking
uint8_t lastState = 0xFF;
unsigned long stateChangeTime = 0;
int stateCount[32] = {0}; // Count occurrences of each state
unsigned long lastCheck = 0;

const char* getUSBStateName(uint8_t state) {
  switch(state) {
    case 0x00: return "DETACHED_INIT";
    case 0x01: return "DETACHED_RESET_DEVICE";
    case 0x02: return "DETACHED_WAIT_RESET";
    case 0x03: return "DETACHED_WAIT_SOF";
    case 0x04: return "ATTACHED_SETTLE";
    case 0x05: return "ATTACHED_RESET_DEVICE";
    case 0x06: return "ATTACHED_WAIT_RESET";
    case 0x07: return "ATTACHED_WAIT_SOF";
    case 0x08: return "ATTACHED_WAIT_RESET2";
    case 0x09: return "ATTACHED_GET_DESC_SIZE";
    case 0x0A: return "ADDRESSING";
    case 0x0B: return "CONFIGURING";
    case 0x0C: return "WAIT_FOR_DEVICE";
    case 0x0D: return "RUNNING";
    case 0x0E: return "ERROR";
    case 0x10: return "ATTACHED_GET_DESCRIPTOR";
    case 0x11: return "ATTACHED_SET_ADDR_RECOVERY";
    case 0x12: return "ATTACHED_WAIT_SET_ADDR";
    case 0x13: return "ATTACHED_GET_CONFIG_DESC";
    case 0x14: return "CONFIGURING_ALT";
    default: return "UNKNOWN";
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000) delay(10);
  
  Serial.println(F("=============================="));
  Serial.println(F("USB Configuration Debugger"));
  Serial.println(F("=============================="));
  Serial.println();
  
  if (Usb.Init() == -1) {
    Serial.println(F("❌ USB Host Shield init FAILED!"));
    Serial.println(F("Hardware check:"));
    Serial.println(F("1. SPI connections correct?"));
    Serial.println(F("2. Power supply adequate?"));
    Serial.println(F("3. Shield compatibility with ESP32?"));
    while(1) delay(1000);
  }
  
  Serial.println(F("✓ USB Host Shield initialized"));
  Serial.println(F("Monitoring USB state transitions..."));
  Serial.println(F("Format: [Time] State: 0xXX (NAME) - Duration: XXXms"));
  Serial.println();
}

void loop() {
  Usb.Task();
  
  if (millis() - lastCheck > 100) { // Check every 100ms for faster response
    lastCheck = millis();
    
    uint8_t currentState = Usb.getUsbTaskState();
    
    // Track state changes
    if (currentState != lastState) {
      unsigned long duration = millis() - stateChangeTime;
      
      // Print previous state duration if it wasn't the first state
      if (lastState != 0xFF) {
        Serial.print(F("["));
        Serial.print(stateChangeTime);
        Serial.print(F("ms] State: 0x"));
        Serial.print(lastState, HEX);
        Serial.print(F(" ("));
        Serial.print(getUSBStateName(lastState));
        Serial.print(F(") - Duration: "));
        Serial.print(duration);
        Serial.println(F("ms"));
        
        // Track problem patterns
        if (lastState == 0x0B && duration > 5000) {
          Serial.println(F("  ⚠️  Long configuration time - possible device issue"));
        }
        if (lastState == 0x14 && currentState != 0x0D) {
          Serial.println(F("  ⚠️  Configuration failed, not reaching RUNNING state"));
        }
      }
      
      // Update tracking
      lastState = currentState;
      stateChangeTime = millis();
      if (currentState < 32) stateCount[currentState]++;
      
      // Special handling for specific states
      switch(currentState) {
        case 0x0C: // WAIT_FOR_DEVICE
          Serial.println(F("  📱 Waiting for device connection"));
          break;
        case 0x0B: // CONFIGURING
          Serial.println(F("  ⚙️  Attempting device configuration..."));
          break;
        case 0x0D: // RUNNING
          Serial.println(F("  🎉 Device successfully configured!"));
          attemptDescriptorRead();
          break;
        case 0x0E: // ERROR
          Serial.println(F("  ❌ USB Error state reached"));
          break;
        case 0x14: // State 20 (0x14)
          Serial.println(F("  ❓ State 0x14 - Alternative configuration attempt"));
          break;
      }
    }
    
    // Print statistics every 30 seconds
    static unsigned long lastStats = 0;
    if (millis() - lastStats > 30000) {
      lastStats = millis();
      printStatistics();
    }
  }
}

void attemptDescriptorRead() {
  Serial.println(F("\n--- Attempting Device Descriptor Read ---"));
  
  USB_DEVICE_DESCRIPTOR desc;
  uint8_t rcode = Usb.getDevDescr(1, 0, sizeof(USB_DEVICE_DESCRIPTOR), (uint8_t*)&desc);
  
  if (rcode == 0) {
    Serial.println(F("✓ Device descriptor read successfully!"));
    Serial.print(F("Vendor ID: 0x"));
    Serial.println(desc.idVendor, HEX);
    Serial.print(F("Product ID: 0x"));
    Serial.println(desc.idProduct, HEX);
    Serial.print(F("Device Class: "));
    Serial.print(desc.bDeviceClass);
    if (desc.bDeviceClass == 7) {
      Serial.println(F(" (PRINTER - Perfect!)"));
    } else {
      Serial.println(F(" (Not a printer class)"));
    }
    Serial.print(F("USB Version: 0x"));
    Serial.println(desc.bcdUSB, HEX);
    
    // Try to read configuration descriptor
    Serial.println(F("\n--- Configuration Descriptor ---"));
    uint8_t confDesc[9];
    rcode = Usb.getConfDescr(1, 0, 9, 0, confDesc);
    if (rcode == 0) {
      Serial.println(F("✓ Configuration descriptor read successfully!"));
      Serial.print(F("Configuration length: "));
      Serial.println(confDesc[2] | (confDesc[3] << 8));
      Serial.print(F("Number of interfaces: "));
      Serial.println(confDesc[4]);
    } else {
      Serial.print(F("❌ Configuration descriptor read failed, code: "));
      Serial.println(rcode);
    }
  } else {
    Serial.print(F("❌ Device descriptor read failed, error code: "));
    Serial.println(rcode);
    
    // Try to diagnose the error
    switch(rcode) {
      case 0x01: Serial.println(F("  → USB_ERROR_TIMEOUT")); break;
      case 0x02: Serial.println(F("  → USB_ERROR_STALL")); break;
      case 0x03: Serial.println(F("  → USB_ERROR_NAK_TIMEOUT")); break;
      case 0x04: Serial.println(F("  → USB_ERROR_BUS")); break;
      default: Serial.println(F("  → Unknown error")); break;
    }
  }
  Serial.println(F("----------------------------------------\n"));
}

void printStatistics() {
  Serial.println(F("\n=== USB State Statistics ==="));
  for (int i = 0; i < 32; i++) {
    if (stateCount[i] > 0) {
      Serial.print(F("State 0x"));
      if (i < 16) Serial.print(F("0"));
      Serial.print(i, HEX);
      Serial.print(F(" ("));
      Serial.print(getUSBStateName(i));
      Serial.print(F("): "));
      Serial.print(stateCount[i]);
      Serial.println(F(" times"));
    }
  }
  
  // Analysis
  if (stateCount[0x0B] > 10 && stateCount[0x0D] == 0) {
    Serial.println(F("\n⚠️  ANALYSIS: Device stuck in configuration loop"));
    Serial.println(F("   Suggestions:"));
    Serial.println(F("   1. Try different USB cable"));
    Serial.println(F("   2. Check printer power supply"));
    Serial.println(F("   3. Test with different thermal printer"));
    Serial.println(F("   4. Verify ESP32-MAX3421E connections"));
  }
  
  if (stateCount[0x14] > 5) {
    Serial.println(F("\n⚠️  ANALYSIS: Frequent state 0x14 transitions"));
    Serial.println(F("   This might indicate:"));
    Serial.println(F("   1. USB timing issues"));
    Serial.println(F("   2. Power instability"));
    Serial.println(F("   3. Incompatible device"));
  }
  
  Serial.println(F("===========================\n"));
}
