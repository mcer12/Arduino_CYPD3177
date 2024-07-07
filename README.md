# Arduino_CYPD3177 Library

The `Arduino_CYPD3177` library provides user friendly interface to interact with the CYPD3177 USB Type-C controller via I2C.

## Features

- Read current USB voltage
- Retrieve USB PD and Type-C status
- Access current PDO and RDO information
- Set built-in GPIO mode and state
- Read built-in GPIO state and analog value

## Installation

1. Download the zip archive of this repository.
2. Unzip the downloaded file and move it to your Arduino libraries directory, typically located in `Documents/Arduino/libraries`.
3. Rename the folder to `Arduino_CYPD3177` if it isn't already named that.

## References
- CYPD3177 Product detail and datasheet
https://www.infineon.com/cms/en/product/universal-serial-bus/usb-c-charging-port-controllers/ez-pd-barrel-connector-replacement-bcr/cypd3177-24lqxq/  
- CYPD3177 I2C interface specification
https://www.infineon.com/dgdl/Infineon-EZ-PD_BCR_Host_Processor_Interface_Specification-Software-v01_00-EN.pdf?fileId=8ac78c8c7d0d8da4017d0f8c4313766b&da=t
- USB PD specification (download button located on the right)  
https://usb.org/document-library/usb-power-delivery  

## List of methods
```cpp
SimpleCYPD3177(); // Constructor

void begin(uint8_t deviceAddress = CYP_DEFAULT_ADDR, uint32_t clockFrequency = 400000); // Initialize the library

void GPIOpinMode(GPIOMode mode); // Set GPIO pin mode

void GPIOdigitalWrite(bool value); // Write digital value to GPIO pin

bool GPIOdigitalRead(); // Read digital value from GPIO pin

uint8_t GPIOanalogRead(); // Read analog value from GPIO pin

uint8_t getDeviceMode(); // Get device mode

uint16_t getSiliconID(); // Get Silicon ID

uint8_t getInterrupt(); // Get interrupt status

PDStatus getPDStatus(); // Get Power Delivery (PD) status

TypeCStatus getTypeCStatus(); // Get Type-C status

uint8_t getBusVoltage(); // Get bus voltage

PDObject getCurrentPDO(); // Get current Power Delivery Object (PDO)

RDObject getCurrentRDO(); // Get current Request Data Object (RDO)

void reset(bool deviceReset); // Reset the device

String uint32ToBinaryString(uint32_t value); // Convert uint32_t value to binary string representation

void readRegister16(uint16_t reg, uint8_t *data, uint8_t length); // Read data from register

void writeRegister16(uint16_t reg, uint8_t data); // Write data to register

void writeRegister16(uint16_t reg, uint8_t *data, uint8_t length); // Write data array to register
```

## Usage

### Include the Library

Include the `Arduino_CYPD3177` library at the beginning of your sketch:

```cpp
#include "Arduino_CYPD3177.h"
```

### Initialize the Library

Create an instance of the `Arduino_CYPD3177` class and initialize it in the `setup` function:

```cpp
Arduino_CYPD3177 cypd;

void setup() {
    Wire.begin();
    Serial.begin(9600);
    cypd.begin();
}
```

### Set GPIO Mode

Set the mode of the GPIO pin:

```cpp
cypd.GPIOpinMode(GPIO_ANALOG_INPUT);        // Analog Input
cypd.GPIOpinMode(GPIO_HIGH_IMPEDANCE);      // High-Impedance Digital Input
cypd.GPIOpinMode(GPIO_PULL_UP);             // Resistive Pull-up
cypd.GPIOpinMode(GPIO_PULL_DOWN);           // Resistive Pull-down
cypd.GPIOpinMode(GPIO_OPEN_DRAIN_LOW);      // Open-drain Drive Low
cypd.GPIOpinMode(GPIO_OPEN_DRAIN_HIGH);     // Open-drain Drive High
cypd.GPIOpinMode(GPIO_STRONG_DRIVE);        // Strong drive
cypd.GPIOpinMode(GPIO_WEAK_DRIVE);          // Resistive drive
```

### Write to GPIO Pin

Set the state of the GPIO pin:

```cpp
cypd.GPIOdigitalWrite(HIGH);  // Set GPIO pin to HIGH
```

### Read GPIO Pin

Read the state of the GPIO pin:

```cpp
bool gpioState = cypd.GPIOdigitalRead();
Serial.println(gpioState ? "HIGH" : "LOW");
```

### Read Analog Value from GPIO Pin

Read the analog value from the GPIO pin:

```cpp
uint8_t analogValue = cypd.GPIOanalogRead();
Serial.println(analogValue);
```

### Get PD Status

Retrieve the current PD status:

```cpp
PDStatus pdStatus = cypd.getPDStatus();
Serial.println("Current Port Data Role: " + String(pdStatus.currentPortDataRole ? "UFP" : "DFP"));
```

### Get Type-C Status

Retrieve the current Type-C status:

```cpp
TypeCStatus typeCStatus = cypd.getTypeCStatus();
Serial.println("Port Partner Connected: " + String(typeCStatus.portPartnerConnected ? "Yes" : "No"));
```

### Get Current PDO

Retrieve the current PDO:

```cpp
    PDObject pdo = cypd.getCurrentPDO();
    String output = "Current PDO:\n";
    output += "Supply Type: " + String(pdo.supplyType) + "\n";
    output += "Dual-Role Power: " + String(pdo.dualRolePower ? "Yes" : "No") + "\n";
    output += "USB Suspend Supported: " + String(pdo.usbSuspendSupported ? "Yes" : "No") + "\n";
    output += "Externally Powered: " + String(pdo.externallyPowered ? "Yes" : "No") + "\n";
    output += "USB Communications Capable: " + String(pdo.usbCommunicationsCapable ? "Yes" : "No") + "\n";
    output += "Dual-Role Data: " + String(pdo.dualRoleData ? "Yes" : "No") + "\n";
    output += "Peak Current: " + String(pdo.peakCurrent) + "\n";
    output += "Voltage: " + String(pdo.voltage * 50) + " mV\n";
    output += "Max Current: " + String(pdo.maxCurrent * 10) + " mA\n";
    Serial.println(output);
```

### Get Current RDO

Retrieve the current RDO:

```cpp
RDObject rdo = cypd.getCurrentRDO();
String output = "";

output += "Object Position: " + String(rdo.objectPosition) + "\n";
output += "Give Back Flag: " + String(rdo.giveBackFlag ? "Yes" : "No") + "\n";
output += "Capability Mismatch: " + String(rdo.capabilityMismatch ? "Yes" : "No") + "\n";
output += "USB Communications Capable: " + String(rdo.usbCommunicationsCapable ? "Yes" : "No") + "\n";
output += "No USB Suspend: " + String(rdo.noUsbSuspend ? "Yes" : "No") + "\n";
output += "Unchunked Extended Messages Supported: " + String(rdo.unchunkedExtendedMessagesSupported ? "Yes" : "No") + "\n";
output += "Operating Current or Power (10mA units): " + String(rdo.operatingCurrentOrPower) + "\n";
output += "Max or Min Operating Current or Power (10mA units): " + String(rdo.maxOrMinOperatingCurrentOrPower) + "\n";

if (rdo.giveBackFlag == 1) {
    output += "Operating Power (250mW units): " + String(rdo.operatingPower) + "\n";
    output += "Min Operating Power (250mW units): " + String(rdo.minOperatingPower) + "\n";
} else if (rdo.objectPosition == 3) {
    output += "Output Voltage (20mV units): " + String(rdo.outputVoltage) + "\n";
    output += "Operating Current (50mA units): " + String(rdo.operatingCurrent) + "\n";
}

Serial.println(output);
```

### Reset the Device

Reset the device:

```cpp
cypd.reset(true);  // true for device reset, false for I2C reset
```

## License

This library is open-source and is licensed under the MIT License.
