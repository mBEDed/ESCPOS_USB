#include "Usb.h"
#include "usbhub.h"

USB Usb;
USBHub Hub1(&Usb);

// USB State definitions for reference
const char* getUSBStateName(uint8_t state) {
  switch(state) {
    case 0x00: return "USB_DETACHED_SUBSTATE_INITIALIZE";
    case 0x01: return "USB_DETACHED_SUBSTATE_RESET_DEVICE";
    case 0x02: return "USB_DETACHED_SUBSTATE_WAIT_RESET_COMPLETE";
    case 0x03: return "USB_DETACHED_SUBSTATE_WAIT_SOF";
    case 0x04: return "USB_ATTACHED_SUBSTATE_SETTLE";
    case 0x05: return "USB_ATTACHED_SUBSTATE_RESET_DEVICE";
    case 0x06: return "USB_ATTACHED_SUBSTATE_WAIT_RESET_COMPLETE";
    case 0x07: return "USB_ATTACHED_SUBSTATE_WAIT_SOF";
    case 0x08: return "USB_ATTACHED_SUBSTATE_WAIT_RESET";
    case 0x09: return "USB_ATTACHED_SUBSTATE_GET_DEVICE_DESCRIPTOR_SIZE";
    case 0x0A: return "USB_STATE_ADDRESSING";
    case 0x0B: return "USB_STATE_CONFIGURING";
    case 0x0C: return "USB_DETACHED_SUBSTATE_WAIT_FOR_DEVICE";
    case 0x0D: return "USB_STATE_RUNNING";
    case 0x0E: return "USB_STATE_ERROR";
    case 0x10: return "USB_ATTACHED_SUBSTATE_GET_DEVICE_DESCRIPTOR";
    case 0x11: return "USB_ATTACHED_SUBSTATE_SET_ADDRESS_RECOVERY";
    case 0x12: return "USB_ATTACHED_SUBSTATE_WAIT_SET_ADDRESS_COMPLETE";
    case 0x13: return "USB_ATTACHED_SUBSTATE_GET_FULL_CONFIG_DESCRIPTOR";
    case 0x14: return "USB_STATE_CONFIGURING (0x14)";
    default: return "UNKNOWN_STATE";
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000) delay(1);
  
  Serial.println("=== USB Host Shield Diagnostic ===");
  Serial.println("Testing MAX3421E communication...");
  
  // Test SPI communication with MAX3421E
  if (Usb.Init() == -1) {
    Serial.println("ERROR: USB Host Shield not found or SPI connection failed!");
    Serial.println("Check your wiring:");
    Serial.println("  ESP32 GPIO23 -> MOSI");
    Serial.println("  ESP32 GPIO19 -> MISO"); 
    Serial.println("  ESP32 GPIO18 -> SCK");
    Serial.println("  ESP32 GPIO5  -> SS/CS");
    Serial.println("  3.3V -> VCC");
    Serial.println("  GND -> GND");
    while(1) { delay(1); }
  }
  
  Serial.println("SUCCESS: USB Host Shield initialized!");
  Serial.println("Scanning for USB devices...");
  delay(1000);
}

void loop() {
  Usb.Task();
  
  static unsigned long lastScan = 0;
  if (millis() - lastScan > 2000) { // Check every 2 seconds
    lastScan = millis();
    
    Serial.print("USB State: ");
    Serial.println(Usb.getUsbTaskState(), HEX);
    
    // Simple check - just report if USB task state indicates device connection
    uint8_t usbState = Usb.getUsbTaskState();
    if (usbState == USB_STATE_RUNNING) {
      Serial.println("USB in RUNNING state - device may be connected");
    } else if (usbState == USB_STATE_CONFIGURING) {
      Serial.println("USB CONFIGURING - device being set up");
    } else if (usbState == USB_DETACHED_SUBSTATE_WAIT_FOR_DEVICE) {
      Serial.println("USB waiting for device connection");
    } else {
      Serial.print("USB State code: 0x");
      Serial.println(usbState, HEX);
    }
  }
}
