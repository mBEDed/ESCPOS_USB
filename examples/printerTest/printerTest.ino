/*------------------------------------------------------------------------
  Example sketch for Adafruit Thermal Printer library for Arduino.
  Demonstrates a few text styles & layouts, bitmap printing, etc.

  IMPORTANT: DECLARATIONS DIFFER FROM PRIOR VERSIONS OF THIS LIBRARY.
  This is to support newer & more board types, especially ones that don't
  support SoftwareSerial (e.g. Arduino Due).  You can pass any Stream
  (e.g. Serial1) to the printer constructor.  See notes below.

  You may need to edit the PRINTER_FIRMWARE value in Adafruit_Thermal.h
  to match your printer (hold feed button on powerup for test page).
  ------------------------------------------------------------------------*/

#include <SPI.h>
#include "USBPrinter.h"
#include "ESC_POS_Printer.h"
#include "qrcode.h"

// Explicit ESP32 pin mapping for MAX3421E-based USB Host Shield
// Adjust these if your wiring differs. Avoid ESP32 strapping pins (0, 2, 12, 15).
#if defined(ESP32)
// VSPI default pins on most ESP32 dev boards
static const int USB_SCK = 18;
static const int USB_MISO = 19;
static const int USB_MOSI = 23;
// Control pins to the shield (wire these accordingly)
static const int USB_SS = 5;     // MAX3421E SS (CS)
static const int USB_INT = 17;   // MAX3421E INT (to ESP32 input)
static const int USB_GPX = 16;   // MAX3421E GPX (optional, can be -1)
static const int USB_RST = 4;    // MAX3421E RESET (optional but recommended)
#endif

class PrinterOper : public USBPrinterAsyncOper
{
  public:
    uint8_t OnInit(USBPrinter *pPrinter);
};

uint8_t PrinterOper::OnInit(USBPrinter *pPrinter)
{
  Serial.println(F("USB Printer OnInit"));
  Serial.print(F("Bidirectional="));
  Serial.println(pPrinter->isBidirectional());
  return 0;
}

USB myusb;
PrinterOper AsyncOper;
USBPrinter uprinter(&myusb, &AsyncOper);
ESC_POS_Printer printer(&uprinter);

static bool initUSBHost()
{
#if defined(ESP32)
  // Bring up VSPI with explicit pins using default SPI instance
  SPI.begin(USB_SCK, USB_MISO, USB_MOSI, USB_SS);
  pinMode(USB_SS, OUTPUT);
  digitalWrite(USB_SS, HIGH);
  if (USB_RST >= 0) {
    pinMode(USB_RST, OUTPUT);
    // Pulse reset low->high to ensure MAX3421E comes up cleanly
    digitalWrite(USB_RST, LOW);
    delay(10);
    digitalWrite(USB_RST, HIGH); // deassert reset
  }
  if (USB_INT >= 0) pinMode(USB_INT, INPUT_PULLUP); // MAX3421E INT is active-low, open-drain
  if (USB_GPX >= 0) pinMode(USB_GPX, INPUT_PULLUP);
  Serial.println(F("ESP32 VSPI initialized for USB Host Shield"));
#endif
  delay(50); // allow pins and regulator to settle

  // Optional: bring VBUS on early
  myusb.vbusPower(vbus_on);
  delay(10);

  // Try up to 3 attempts with long oscillator settle time
  for (int attempt = 1; attempt <= 3; ++attempt) {
    int8_t rc = myusb.Init(1000); // 1s timeout for OSCOK
    if (rc == 0) {
      Serial.print(F("USB Host init OK on attempt ")); Serial.println(attempt);
      return true;
    }
    Serial.print(F("USB init attempt ")); Serial.print(attempt); Serial.println(F(" failed"));
    // Toggle RESET and retry
#if defined(ESP32)
    if (USB_RST >= 0) {
      digitalWrite(USB_RST, LOW);
      delay(20);
      digitalWrite(USB_RST, HIGH);
    }
#endif
    delay(100);
  }
  return false;
}

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000) delay(1);
  if (!initUSBHost()) {
    Serial.println(F("USB host failed to initialize"));
  }
}

void printer_test()
{
  printer.reset();
  printer.setDefault(); // Restore printer to defaults

  // Test inverse on & off
  printer.inverseOn();
  printer.println(F("Inverse ON"));
  printer.inverseOff();

  // Test character double-height on & off
  printer.doubleHeightOn();
  printer.println(F("Double Height ON"));
  printer.doubleHeightOff();

  // Set text justification (right, center, left) -- accepts 'L', 'C', 'R'
  printer.justify('R');
  printer.println(F("Right justified"));
  printer.justify('C');
  printer.println(F("Center justified"));
  printer.justify('L');
  printer.println(F("Left justified"));

  // Test more styles
  printer.boldOn();
  printer.println(F("Bold text"));
  printer.boldOff();

  printer.underlineOn();
  printer.println(F("Underlined text"));
  printer.underlineOff();

  printer.setSize('L');        // Set type size, accepts 'S', 'M', 'L'
  printer.println(F("Large"));
  printer.setSize('M');
  printer.println(F("Medium"));
  printer.setSize('S');
  printer.println(F("Small"));

  printer.justify('C');
  printer.println(F("normal\nline\nspacing"));
  printer.setLineHeight(50);
  printer.println(F("Taller\nline\nspacing"));
  printer.setLineHeight(); // Reset to default
  printer.justify('L');

  // Print the QR code in graphics mode
  // See the ./bitmap/mkqrcode program to generate qrcode.h
  Serial.print(F("qrcode_data size = ")); Serial.println(sizeof(qrcode_data));
#if defined(ESP8266) || defined(AVR)
  printer.printBitmap_P(qrcode_width, qrcode_height, qrcode_data, qrcode_density);
#else
  printer.printBitmap(qrcode_width, qrcode_height, qrcode_data, qrcode_density);
#endif
  printer.println(F("ESC_POS_Printer"));

  printer.feed(2);
}

void loop() {
  myusb.Task();

  // Make sure USB printer found and ready
  if (uprinter.isReady()) {
    printer.begin();
    Serial.println(F("Init ESC POS printer"));

    printer_test();
    // Do this one time to avoid wasting paper
    while (1) delay(1);
  }
}
