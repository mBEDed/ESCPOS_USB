#include "Usb.h"
#include "usbhub.h"

USB Usb;
USBHub Hub1(&Usb);

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000) delay(1);
  
  Serial.println("=== USB Descriptor Dump ===");
  
  if (Usb.Init() == -1) {
    Serial.println("ERROR: USB Host Shield not found!");
    while(1) { delay(1); }
  }
  
  Serial.println("USB Host initialized.");
}

void loop() {
  Usb.Task();
  
  static unsigned long lastCheck = 0;
  static bool descriptorDumped = false;
  
  if (millis() - lastCheck > 2000) { // Check every 2 seconds
    lastCheck = millis();
    
    uint8_t usbState = Usb.getUsbTaskState();
    Serial.print("USB State: 0x");
    Serial.println(usbState, HEX);
    
    if (usbState == USB_STATE_RUNNING && !descriptorDumped) {
      Serial.println("USB is running, attempting to dump descriptors...");
      
      // Try to get device descriptor for address 1 (first device)
      USB_DEVICE_DESCRIPTOR desc;
      uint8_t rcode = Usb.getDevDescr(1, 0, sizeof(USB_DEVICE_DESCRIPTOR), (uint8_t*)&desc);
      
      if (rcode == 0) {
        Serial.println("=== Device Descriptor Found ===");
        Serial.print("bLength: ");
        Serial.println(desc.bLength);
        Serial.print("bDescriptorType: ");
        Serial.println(desc.bDescriptorType);
        Serial.print("bcdUSB: 0x");
        Serial.println(desc.bcdUSB, HEX);
        Serial.print("bDeviceClass: ");
        Serial.println(desc.bDeviceClass);
        Serial.print("bDeviceSubClass: ");
        Serial.println(desc.bDeviceSubClass);
        Serial.print("bDeviceProtocol: ");
        Serial.println(desc.bDeviceProtocol);
        Serial.print("bMaxPacketSize0: ");
        Serial.println(desc.bMaxPacketSize0);
        Serial.print("idVendor: 0x");
        Serial.println(desc.idVendor, HEX);
        Serial.print("idProduct: 0x");
        Serial.println(desc.idProduct, HEX);
        Serial.print("bcdDevice: 0x");
        Serial.println(desc.bcdDevice, HEX);
        Serial.print("iManufacturer: ");
        Serial.println(desc.iManufacturer);
        Serial.print("iProduct: ");
        Serial.println(desc.iProduct);
        Serial.print("iSerialNumber: ");
        Serial.println(desc.iSerialNumber);
        Serial.print("bNumConfigurations: ");
        Serial.println(desc.bNumConfigurations);
        
        // Check if it's a printer (Class 7)
        if (desc.bDeviceClass == 7) {
          Serial.println("*** PRINTER DEVICE DETECTED! ***");
        }
        
        descriptorDumped = true;
      } else {
        Serial.print("Failed to get device descriptor, error code: ");
        Serial.println(rcode);
      }
    } else if (usbState != USB_STATE_RUNNING) {
      descriptorDumped = false; // Reset flag when device disconnects
    }
  }
}
