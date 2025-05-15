# USB Printer Library for Arduino-Compatible Boards

This library enables direct USB communication between Arduino-compatible boards and the thermal printer using a USB Host Shield.

---

## Hardware Requirements

- **Arduino-Compatible Board**: Such as Arduino Uno, ESP32, or others with SPI support.
- **USB Host Shield 2.0**: Based on the MAX3421E chip.
- **Thermal Printer**: Connected via USB cable.
- **Thermal Paper Rolls**: 56–58mm width, ≤30mm diameter.
- **Micro USB Cable**: For programming the microcontroller.
- **Optional**: External 5V power supply for the printer (recommended for consistent performance).

---

## Software Requirements

### Arduino Libraries

Ensure the following libraries are installed in your Arduino IDE:

1. **USB Host Shield Library 2.0**
   - Repository: [felis/USB_Host_Shield_2.0](https://github.com/felis/USB_Host_Shield_2.0)
   - Installation:
     - Open Arduino IDE.
     - Navigate to `Sketch` > `Include Library` > `Manage Libraries...`.
     - Search for "USB Host Shield Library 2.0".
     - Click "Install".
2. **ESC_POS_Printer**
   - Repository: [ESC_POS_Printer](https://github.com/gdsports/ESC_POS_Printer)
   - Installation:
     - download the zip.
     - extract in Arduino Library Folder (/Arduino/libraries).
---

## Hardware Connections

### Arduino Uno

- **D13**: SCK
- **D12**: MISO
- **D11**: MOSI
- **D10**: SS

### ESP32 (using VSPI)

- **GPIO23**: MOSI
- **GPIO19**: MISO
- **GPIO18**: SCK
- **GPIO5**: SS

**For ESP32 users, make sure `#include "usbhub.h"` is added to USBPrinter.h.**

Connect the thermal printer to the USB Host Shield's USB port using a USB cable.

---

## Usage

It is available in the Arduino library manager or you can download the zip to the local /Arduino/libraries path of your IDE, once installed you can see the example in the menu or open it from the examples folder.

---

## Troubleshooting

- **No Output on Printer**:
  - Ensure the printer is powered on and properly connected.
  - Verify that the USB Host Shield is functioning and recognized by the microcontroller.

- **Compilation Errors**:
  - Confirm that all required libraries are correctly installed.
  - For ESP32 users, make sure `#include "usbhub.h"` is added to USBPrinter.h.

- **Power Issues**:
  - The printer may require more power than the microcontroller can provide via USB. Consider using an external 5V power supply.

---

## If the library was useful to you
[![](https://img.shields.io/static/v1?label=Sponsor&message=%E2%9D%A4&logo=GitHub&color=%23fe8e86)](https://github.com/sponsors/userHarpreet)

---

## References

This has never been success without the work below.

- USB Host Shield Library 2.0: [felis/USB_Host_Shield_2.0](https://github.com/felis/USB_Host_Shield_2.0)
- USBPrinter_uhs2: [gdsports/USBPrinter_uhs2](https://github.com/gdsports/USBPrinter_uhs2) **just another port with minor standardizations**


---

Feel free to customize and expand upon this project to suit your specific needs, such as adding image printing capabilities or integrating with other peripherals.
