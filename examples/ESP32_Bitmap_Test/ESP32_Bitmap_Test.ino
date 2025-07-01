/*
 * ESP32 Thermal Printer Bitmap Test
 * Simple example showing how to print bitmaps/images
 */

#include "USBPrinter.h"
#include "ESC_POS_Printer.h"

// Simple 8x8 smiley face bitmap (1 bit per pixel, 8 bytes total)
const uint8_t smiley_bitmap[] PROGMEM = {
  0b00111100,  // Row 0:   ####
  0b01000010,  // Row 1:  #    #
  0b10100101,  // Row 2: # #  # #
  0b10000001,  // Row 3: #      #
  0b10100101,  // Row 4: # #  # #
  0b10011001,  // Row 5: #  ##  #
  0b01000010,  // Row 6:  #    #
  0b00111100   // Row 7:   ####
};

// 16x16 heart bitmap (32 bytes total)
const uint8_t heart_bitmap[] PROGMEM = {
  0x00, 0x00,  // ................
  0x0E, 0x70,  // ....###..###....
  0x1F, 0xF8,  // ...##########...
  0x3F, 0xFC,  // ..############..
  0x7F, 0xFE,  // .##############.
  0xFF, 0xFF,  // ################
  0xFF, 0xFF,  // ################
  0x7F, 0xFE,  // .##############.
  0x7F, 0xFE,  // .##############.
  0x3F, 0xFC,  // ..############..
  0x1F, 0xF8,  // ...##########...
  0x0F, 0xF0,  // ....########....
  0x07, 0xE0,  // .....######.....
  0x03, 0xC0,  // ......####......
  0x01, 0x80,  // .......##.......
  0x00, 0x00   // ................
};

class PrinterOper : public USBPrinterAsyncOper {
  public:
    uint8_t OnInit(USBPrinter *pPrinter);
};

uint8_t PrinterOper::OnInit(USBPrinter *pPrinter) {
  Serial.println(F("USB Printer initialized for bitmap test"));
  return 0;
}

USB myusb;
PrinterOper AsyncOper;
USBPrinter uprinter(&myusb, &AsyncOper);
ESC_POS_Printer printer(&uprinter);

bool testDone = false;

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000) delay(10);
  
  Serial.println(F("ESP32 Thermal Printer Bitmap Test"));
  Serial.println(F("=================================="));
  
  if (myusb.Init() == -1) {
    Serial.println(F("USB Host Shield init failed"));
    while(1) delay(1000);
  }
  
  Serial.println(F("USB Host initialized. Connect thermal printer..."));
}

void loop() {
  myusb.Task();
  
  if (uprinter.isReady() && !testDone) {
    runBitmapTest();
    testDone = true;
  }
}

void runBitmapTest() {
  Serial.println(F("Starting bitmap test..."));
  
  printer.begin();
  printer.reset();
  
  // Print header
  printer.justify('C');
  printer.boldOn();
  printer.println(F("BITMAP TEST"));
  printer.boldOff();
  printer.feed(2);
  
  // Print 8x8 smiley face
  printer.println(F("8x8 Smiley:"));
  printer.printBitmap(8, 8, smiley_bitmap);
  printer.feed(2);
  
  // Print 16x16 heart
  printer.println(F("16x16 Heart:"));
  printer.printBitmap(16, 16, heart_bitmap);
  printer.feed(2);
  
  // Print multiple small bitmaps in a row
  printer.println(F("Multiple smileys:"));
  for(int i = 0; i < 3; i++) {
    printer.printBitmap(8, 8, smiley_bitmap);
    printer.print(F("  ")); // Some spacing
  }
  printer.feed(3);
  
  Serial.println(F("Bitmap test completed!"));
}

/*
 * Bitmap Format Notes:
 * ===================
 * - Bitmaps are 1-bit per pixel (black/white)
 * - Data is organized row by row
 * - Each row must be byte-aligned (padded to nearest byte)
 * - For widths not divisible by 8, pad with zeros on the right
 * 
 * Creating Custom Bitmaps:
 * ========================
 * 1. Create image in graphics software (black/white, correct size)
 * 2. Convert to 1-bit bitmap format
 * 3. Use online tools or custom scripts to convert to C array
 * 4. Store in PROGMEM to save RAM
 * 
 * Example tools:
 * - LCD Image Converter
 * - Online bitmap to C array converters
 * - Python scripts with PIL/Pillow
 */
