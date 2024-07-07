#ifndef Arduino_CYPD3177_H
#define Arduino_CYPD3177_H

#include <Arduino.h>
#include <Wire.h>

// Read registers
#define CYP_DEVICE_MODE 0x0000
#define CYP_SILICON_ID 0x0002
#define CYP_INTERRUPT 0x0006
#define CYP_PD_STATUS 0x1008
#define CYP_TYPE_C_STATUS 0x100C
#define CYP_BUS_VOLTAGE 0x100D
#define CYP_CURRENT_PDO 0x1010
#define CYP_CURRENT_RDO 0x1014
#define CYP_SWAP_RESPONSE 0x1028
#define CYP_EVENT_STATUS 0x1044
#define CYP_READ_GPIO_LEVEL 0x0082
#define CYP_SAMPLE_GPIO 0x0083

// Command registers
#define CYP_RESET 0x0008
#define CYP_EVENT_MASK 0x1024
#define CYP_DM_CONTROL 0x1000
#define CYP_SELECT_SINK_PDO 0x1005
#define CYP_PD_CONTROL 0x1006
#define CYP_REQUEST 0x1050
#define CYP_SET_GPIO_MODE 0x0080
#define CYP_SET_GPIO_LEVEL 0x0081
#define CYP_DEV_RESPONSE 0x007E
#define CYP_PD_RESPONSE 0x1400

#define CYP_DEFAULT_ADDR 0x08

enum GPIOMode
{
    GPIO_ANALOG_INPUT = 0,
    GPIO_HIGH_IMPEDANCE,
    GPIO_PULL_UP,
    GPIO_PULL_DOWN,
    GPIO_OPEN_DRAIN_LOW,
    GPIO_OPEN_DRAIN_HIGH,
    GPIO_STRONG_DRIVE,
    GPIO_WEAK_DRIVE
};

struct PDStatus
{
    bool currentPortDataRole;
    bool currentPortPowerRole;
    bool contractState;
    bool sinkTxReadyStatus;
    bool policyEngineState;
    uint8_t pdSpecRevisionBCR;
    uint8_t partnerPDRevision;
};

struct TypeCStatus
{
    bool portPartnerConnected;
    bool ccPolarity;
    uint8_t attachedDeviceType;
    uint8_t typeCCurrentLevel;
};

struct PDObject
{
    uint8_t supplyType;
    bool dualRolePower;
    bool usbSuspendSupported;
    bool externallyPowered;
    bool usbCommunicationsCapable;
    bool dualRoleData;
    uint8_t peakCurrent;
    uint16_t voltage;
    uint16_t maxCurrent;
    uint16_t minVoltageBattery;
    uint16_t maxVoltageBattery;
    uint16_t maxPowerBattery;
    uint16_t minVoltageVariable;
    uint16_t maxVoltageVariable;
    uint16_t operationalCurrentVariable;
    bool progPowerSupplyAPDO;
    uint16_t maxVoltageAPDO;
    uint16_t minVoltageAPDO;
    uint16_t maxCurrentAPDO;
};

struct RDObject
{
    uint8_t objectPosition;
    uint8_t giveBackFlag;
    uint8_t capabilityMismatch;
    uint8_t usbCommunicationsCapable;
    uint8_t noUsbSuspend;
    uint8_t unchunkedExtendedMessagesSupported;
    uint16_t operatingCurrentOrPower;
    uint16_t maxOrMinOperatingCurrentOrPower;
    uint16_t operatingPower;
    uint16_t minOperatingPower;
    uint16_t outputVoltage;
    uint8_t operatingCurrent;
};

class Arduino_CYPD3177
{
public:
    Arduino_CYPD3177();

    void begin(uint8_t deviceAddress = CYP_DEFAULT_ADDR, uint32_t clockFrequency = 400000);
    void GPIOpinMode(GPIOMode mode);
    void GPIOdigitalWrite(bool value);
    bool GPIOdigitalRead();
    uint8_t GPIOanalogRead();
    uint8_t getDeviceMode();
    uint16_t getSiliconID();
    uint8_t getInterrupt();
    PDStatus getPDStatus();
    TypeCStatus getTypeCStatus();
    uint8_t getBusVoltage();
    PDObject getCurrentPDO();
    RDObject getCurrentRDO();
    void reset(bool deviceReset);
    void readRegister16(uint16_t reg, uint8_t *data, uint8_t length);
    void writeRegister16(uint16_t reg, uint8_t data);
    void writeRegister16(uint16_t reg, uint8_t *data, uint8_t length);

private:
    uint8_t deviceAddress;
};

#endif // Arduino_CYPD3177_H
