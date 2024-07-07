#include "Arduino_CYPD3177.h"

Arduino_CYPD3177::Arduino_CYPD3177()
{
    deviceAddress = CYP_DEFAULT_ADDR;
}

void Arduino_CYPD3177::begin(uint8_t deviceAddress, uint32_t clockFrequency)
{
    Wire.begin();
    Wire.setClock(clockFrequency);
    this->deviceAddress = deviceAddress;
}

void Arduino_CYPD3177::GPIOpinMode(GPIOMode mode)
{
    uint8_t gpioMode;
    gpioMode = static_cast<uint8_t>(mode);
    writeRegister16(CYP_SET_GPIO_MODE, gpioMode);
}

void Arduino_CYPD3177::GPIOdigitalWrite(bool value)
{
    if (value == true)
    {
        writeRegister16(CYP_SET_GPIO_LEVEL, 1);
    }
    else
    {
        writeRegister16(CYP_SET_GPIO_LEVEL, 0);
    }
}

bool Arduino_CYPD3177::GPIOdigitalRead()
{
    uint8_t dataBuffer[1];
    readRegister16(CYP_READ_GPIO_LEVEL, dataBuffer, 1);

    if (dataBuffer[0] == 1)
    {
        return true;
    }
    else
    {
        return false;
    }
}

uint8_t Arduino_CYPD3177::GPIOanalogRead()
{
    uint8_t dataBuffer[1];
    readRegister16(CYP_SAMPLE_GPIO, dataBuffer, 1);
    return dataBuffer[0];
}

uint8_t Arduino_CYPD3177::getDeviceMode()
{
    uint8_t dataBuffer[1];
    readRegister16(CYP_DEVICE_MODE, dataBuffer, 1);
    return dataBuffer[0];
}

uint16_t Arduino_CYPD3177::getSiliconID()
{
    uint8_t dataBuffer[2];
    readRegister16(CYP_SILICON_ID, dataBuffer, 2);
    uint16_t data = (dataBuffer[0]) | (dataBuffer[1] << 8);
    return data;
}

uint8_t Arduino_CYPD3177::getInterrupt()
{
    uint8_t dataBuffer[1];
    readRegister16(CYP_INTERRUPT, dataBuffer, 1);
    return dataBuffer[0];
}

PDStatus Arduino_CYPD3177::getPDStatus()
{
    PDStatus status;
    uint8_t dataBuffer[2];

    readRegister16(CYP_PD_STATUS, dataBuffer, 2);

    uint16_t data = (dataBuffer[0]) | (dataBuffer[1] << 8);

    status.currentPortDataRole = (data >> 6) & 0x01;
    status.currentPortPowerRole = (data >> 8) & 0x01;
    status.contractState = (data >> 10) & 0x01;
    status.sinkTxReadyStatus = (data >> 14) & 0x01;
    status.policyEngineState = (data >> 15) & 0x01;
    status.pdSpecRevisionBCR = (data >> 16) & 0x03;
    status.partnerPDRevision = (data >> 18) & 0x01;

    return status;
}

TypeCStatus Arduino_CYPD3177::getTypeCStatus()
{
    uint8_t dataBuffer[4];
    readRegister16(CYP_TYPE_C_STATUS, dataBuffer, 4);
    uint32_t data = (dataBuffer[0]) | (dataBuffer[1] << 8) | (dataBuffer[2] << 16) | (dataBuffer[3] << 24);

    TypeCStatus status;

    status.portPartnerConnected = (data & 0x01) != 0; // Bit 0
    status.ccPolarity = (data & 0x02) != 0;           // Bit 1
    status.attachedDeviceType = (data >> 2) & 0x07;   // Bits 4-2
    status.typeCCurrentLevel = (data >> 6) & 0x03;    // Bits 7-6

    return status;
}

uint8_t Arduino_CYPD3177::getBusVoltage()
{
    uint8_t dataBuffer[1];
    readRegister16(CYP_BUS_VOLTAGE, dataBuffer, 1);
    return dataBuffer[0];
}

PDObject Arduino_CYPD3177::getCurrentPDO()
{
    PDObject pdoData;

    uint8_t dataBuffer[4];
    readRegister16(CYP_CURRENT_PDO, dataBuffer, 4);

    uint32_t pdo = dataBuffer[0] | (dataBuffer[1] << 8) | (dataBuffer[2] << 16) | (dataBuffer[3] << 24);

    pdoData.supplyType = (pdo >> 30) & 0x03; // Bits 30-31

    switch (pdoData.supplyType)
    {
    case 0:                                                    // Fixed supply (Vmin = Vmax)
        pdoData.dualRolePower = (pdo >> 29) & 0x01;            // Bit 29
        pdoData.usbSuspendSupported = (pdo >> 28) & 0x01;      // Bit 28
        pdoData.externallyPowered = (pdo >> 27) & 0x01;        // Bit 27
        pdoData.usbCommunicationsCapable = (pdo >> 26) & 0x01; // Bit 26
        pdoData.dualRoleData = (pdo >> 25) & 0x01;             // Bit 25
        pdoData.peakCurrent = (pdo >> 20) & 0x03;              // Bits 20-21

        pdoData.voltage = (pdo >> 10) & 0x3FF; // Bits 10-19
        pdoData.maxCurrent = pdo & 0x3FF;      // Bits 0-9
        break;

    case 1:                                              // Battery
        pdoData.maxVoltageBattery = (pdo >> 20) & 0x3FF; // Bits 20-29
        pdoData.minVoltageBattery = (pdo >> 10) & 0x3FF; // Bits 10-19
        pdoData.maxPowerBattery = pdo & 0x3FF;           // Bits 0-9
        break;

    case 2:                                               // Variable Supply (non-Battery)
        pdoData.maxVoltageVariable = (pdo >> 20) & 0x3FF; // Bits 20-29
        pdoData.minVoltageVariable = (pdo >> 10) & 0x3FF; // Bits 10-19
        pdoData.operationalCurrentVariable = pdo & 0x3FF; // Bits 0-9
        break;

    case 3:                                               // Augmented Power Data Object (APDO)
        pdoData.progPowerSupplyAPDO = (pdo >> 28) & 0x01; // Bit 28
        pdoData.maxVoltageAPDO = (pdo >> 17) & 0xFF;      // Bits 17-24
        pdoData.minVoltageAPDO = (pdo >> 8) & 0xFF;       // Bits 8-15
        pdoData.maxCurrentAPDO = pdo & 0x7F;              // Bits 0-6
        break;

    default:
        // Unknown supply type
        pdoData.supplyType = 0xFF; // Set to some invalid value
        break;
    }

    return pdoData;
}

RDObject Arduino_CYPD3177::getCurrentRDO()
{
    uint8_t dataBuffer[4];
    readRegister16(CYP_CURRENT_RDO, dataBuffer, 4);
    uint32_t rdo = dataBuffer[0] | (dataBuffer[1] << 8) | (dataBuffer[2] << 16) | (dataBuffer[3] << 24);

    RDObject rdoObject;
    rdoObject.objectPosition = (rdo >> 28) & 0x07;
    rdoObject.giveBackFlag = (rdo >> 27) & 0x01;
    rdoObject.capabilityMismatch = (rdo >> 26) & 0x01;
    rdoObject.usbCommunicationsCapable = (rdo >> 25) & 0x01;
    rdoObject.noUsbSuspend = (rdo >> 24) & 0x01;
    rdoObject.unchunkedExtendedMessagesSupported = (rdo >> 23) & 0x01;
    rdoObject.operatingCurrentOrPower = (rdo >> 10) & 0x3FF;
    rdoObject.maxOrMinOperatingCurrentOrPower = rdo & 0x3FF;

    if (rdoObject.giveBackFlag)
    {
        rdoObject.operatingPower = (rdo >> 10) & 0x3FF;
        rdoObject.minOperatingPower = rdo & 0x3FF;
    }
    else if (rdoObject.objectPosition == 3)
    {
        rdoObject.outputVoltage = (rdo >> 9) & 0x7FF;
        rdoObject.operatingCurrent = rdo & 0x7F;
    }

    return rdoObject;
}

void Arduino_CYPD3177::reset(bool deviceReset)
{
    uint8_t resetData[2];
    resetData[0] = 0x52;                      // Signature byte 'R'
    resetData[1] = deviceReset ? 0x01 : 0x00; // Reset type
    writeRegister16(CYP_RESET, resetData, 2);
}

void Arduino_CYPD3177::readRegister16(uint16_t reg, uint8_t *data, uint8_t length)
{
    Wire.beginTransmission(deviceAddress);
    Wire.write(lowByte(reg));
    Wire.write(highByte(reg));
    Wire.endTransmission(false);

    Wire.requestFrom(deviceAddress, length);
    uint8_t index = 0;
    while (Wire.available() && index < length)
    {
        data[index++] = Wire.read();
    }
    Wire.endTransmission();
}

void Arduino_CYPD3177::writeRegister16(uint16_t reg, uint8_t data)
{
    Wire.beginTransmission(deviceAddress);
    Wire.write(lowByte(reg));
    Wire.write(highByte(reg));
    Wire.write(data);
    Wire.endTransmission();
}

void Arduino_CYPD3177::writeRegister16(uint16_t reg, uint8_t *data, uint8_t length)
{
    Wire.beginTransmission(deviceAddress);
    Wire.write(lowByte(reg));
    Wire.write(highByte(reg));
    for (uint8_t i = 0; i < length; i++)
    {
        Wire.write(data[i]);
    }
    Wire.endTransmission();
}