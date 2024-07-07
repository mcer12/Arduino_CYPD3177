#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

#include "Arduino_CYPD3177.h"

#ifndef STASSID
#define STASSID "NAME"
#define STAPSK  "PASSWORD"
#endif

const char* ssid = STASSID;
const char* password = STAPSK;

ESP8266WebServer server(80);

Arduino_CYPD3177 cypd;

void setup(void) {
  cypd.begin(); // Uses default address CYP_DEFAULT_ADDR

  /*
    GPIO_ANALOG_INPUT,
    GPIO_HIGH_IMPEDANCE,
    GPIO_PULL_UP,
    GPIO_PULL_DOWN,
    GPIO_OPEN_DRAIN_LOW,
    GPIO_OPEN_DRAIN_HIGH,
    GPIO_STRONG_DRIVE,
    GPIO_WEAK_DRIVE
  */
  cypd.GPIOpinMode(GPIO_ANALOG_INPUT); // Set GPIO mode
  //cypd.GPIOdigitalWrite(1);

  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
  MDNS.update();
}

void handleRoot() {
  String output = "";

  // Read DEVICE_MODE
  uint8_t deviceMode = cypd.getDeviceMode();
  output += "DEVICE_MODE: 0x" + String(deviceMode, HEX) + "\n";

  // Read SILICON_ID
  uint16_t siliconID = cypd.getSiliconID();
  output += "SILICON_ID: 0x" + String(siliconID, HEX) + "\n";

  uint8_t analogReadGPIO = cypd.GPIOanalogRead();
  output += "GPIO ANALOG READ: " + String(analogReadGPIO) + "\n";

  // Read BUS_VOLTAGE
  uint8_t busVoltage = cypd.getBusVoltage();
  output += "BUS_VOLTAGE: " + String(busVoltage * 100) + "mV\n";


  // Read PD_STATUS
  output += "--------------------------- \n";
  output += "PD STATUS ----------------- \n";
  output += "--------------------------- \n";
  PDStatus pd_status = cypd.getPDStatus();

  output += "Current Port Data Role: ";
  output += (pd_status.currentPortDataRole ? "DFP (Downstream Facing Port)" : "UFP (Upstream Facing Port)");
  output += "\nCurrent Port Power Role: ";
  output += (pd_status.currentPortPowerRole ? "Source" : "Sink");
  output += "\nContract State: ";
  output += (pd_status.contractState ? "Explicit PD contract exists" : "No contract exists");
  output += "\nSink Tx Ready Status (Rp state): ";
  output += (pd_status.sinkTxReadyStatus ? "SinkTxNG (Sink shouldn't send messages)" : "SinkTxOk (Sink can send messages to Source)");
  output += "\nPolicy Engine State: ";
  output += (pd_status.policyEngineState ? "PE_SNK_Ready state" : "Not in PE_SNK_Ready state");
  output += "\nPD Spec Revision supported by BCR: ";
  switch (pd_status.pdSpecRevisionBCR) {
    case 0:
      output += "PD 2.0";
      break;
    case 1:
      output += "PD 3.0";
      break;
    default:
      output += "Reserved";
      break;
  }
  output += "\nPartner (Attached device) PD Spec revision: ";
  output += (pd_status.partnerPDRevision ? "PD 3.0 device" : "PD 2.0 device");

  output += "\n";


  // Read TYPE_C_STATUS
  output += "--------------------------- \n";
  output += "TYPE C STATUS ------------- \n";
  output += "--------------------------- \n";
  TypeCStatus typec_status = cypd.getTypeCStatus();

  output += "Port Partner Connected: ";
  output += (typec_status.portPartnerConnected ? "Yes" : "No");
  output += "\nCC Polarity: ";
  output += (typec_status.ccPolarity ? "CC2" : "CC1");
  output += "\nAttached Device Type: ";
  switch (typec_status.attachedDeviceType) {
    case 0:
      output += "Nothing attached";
      break;
    case 2:
      output += "Source attached";
      break;
    case 3:
      output += "Debug Accessory attached";
      break;
    default:
      output += "Reserved";
      break;
  }
  output += "\nType-C Current Level: ";
  switch (typec_status.typeCCurrentLevel) {
    case 0:
      output += "Default (900mA)";
      break;
    case 1:
      output += "1.5A";
      break;
    case 2:
      output += "3A";
      break;
    default:
      output += "Reserved";
      break;
  }

  output += "\n";


  // Read CURRENT_PDO
  output += "--------------------------- \n";
  output += "CURRENT PDO --------------- \n";
  output += "--------------------------- \n";

  if (pd_status.contractState) {

    PDObject pdoData = cypd.getCurrentPDO(); // Power Data Object (PDO)
    switch (pdoData.supplyType) {
      case 0:
        output += "Supply Type: Fixed supply (Vmin = Vmax)\n";
        output += "Dual-Role Power: " + String(pdoData.dualRolePower ? "Yes" : "No") + "\n";
        output += "USB Suspend Supported: " + String(pdoData.usbSuspendSupported ? "Yes" : "No") + "\n";
        output += "Externally Powered: " + String(pdoData.externallyPowered ? "Yes" : "No") + "\n";
        output += "USB Communications Capable: " + String(pdoData.usbCommunicationsCapable ? "Yes" : "No") + "\n";
        output += "Dual-Role Data: " + String(pdoData.dualRoleData ? "Yes" : "No") + "\n";
        output += "Peak Current: ";
        switch (pdoData.peakCurrent) {
          case 0:
            output += "IOC (default)\n";
            break;
          case 1:
            output += "150% IOC for 1ms @ 5% duty cycle (low current equals 97% IOC for 19ms)\n";
            break;
          case 2:
            output += "125% IOC for 2ms @ 10% duty cycle (low current equals 97% IOC for 18ms)\n";
            break;
          case 3:
            output += "110% IOC for 10ms @ 50% duty cycle (low current equals 90% IOC for 10ms)\n";
            break;
          default:
            output += "Reserved\n";
            break;
        }
        output += "Voltage: " + String(pdoData.voltage * 50) + " mV\n";
        output += "Maximum Current: " + String(pdoData.maxCurrent * 10) + " mA\n";
        break;
      case 1:
        output += "Supply Type: Battery\n";
        output += "Maximum Voltage: " + String(pdoData.maxVoltageBattery * 50) + " mV\n";
        output += "Minimum Voltage: " + String(pdoData.minVoltageBattery * 50) + " mV\n";
        output += "Maximum Allowable Power: " + String(pdoData.maxPowerBattery) + "\n";
        break;
      case 2:
        output += "Supply Type: Variable Supply (non-Battery)\n";
        output += "Maximum Voltage: " + String(pdoData.maxVoltageVariable * 50) + " mV\n";
        output += "Minimum Voltage: " + String(pdoData.minVoltageVariable * 50) + " mV\n";
        output += "Operational Current: " + String(pdoData.operationalCurrentVariable * 10) + " mA\n";
        break;
      case 3:
        output += "Supply Type: Augmented Power Data Object (APDO)\n";
        output += "Programmable Power Supply: " + String(pdoData.progPowerSupplyAPDO ? "Yes" : "No") + "\n";
        output += "Maximum Voltage: " + String(pdoData.maxVoltageAPDO * 100) + " mV\n";
        output += "Minimum Voltage: " + String(pdoData.minVoltageAPDO * 100) + " mV\n";
        output += "Maximum Current: " + String(pdoData.maxCurrentAPDO * 50) + " mA\n";
        break;
      default:
        output += "Unknown supply type\n";
        break;
    }
  }
  else {
    output += "Unavailable - PD not negotiated \n";
  }


  // Read CURRENT_RDO
  output += "--------------------------- \n";
  output += "CURRENT RDO --------------- \n";
  output += "--------------------------- \n";

  if (pd_status.contractState) {
    RDObject rdo = cypd.getCurrentRDO(); // Request Data Object (PDO)

    output += "Object Position: " + String(rdo.objectPosition) + "\n";
    output += "Give Back Flag: " + String(rdo.giveBackFlag ? "Yes" : "No") + "\n";
    output += "Capability Mismatch: " + String(rdo.capabilityMismatch ? "Yes" : "No") + "\n";
    output += "USB Communications Capable: " + String(rdo.usbCommunicationsCapable ? "Yes" : "No") + "\n";
    output += "No USB Suspend: " + String(rdo.noUsbSuspend ? "Yes" : "No") + "\n";
    output += "Unchunked Extended Messages Supported: " + String(rdo.unchunkedExtendedMessagesSupported ? "Yes" : "No") + "\n";
    output += "Operating Current or Power (10mA units): " + String(rdo.operatingCurrentOrPower * 10) + "\n";
    output += "Max or Min Operating Current or Power (10mA units): " + String(rdo.maxOrMinOperatingCurrentOrPower * 10) + "\n";

    if (rdo.giveBackFlag == 1) {
      output += "Operating Power (250mW units): " + String(rdo.operatingPower * 250) + " mW\n";
      output += "Min Operating Power (250mW units): " + String(rdo.minOperatingPower * 250) + " mW\n";
    } else if (rdo.objectPosition == 3) {
      output += "Output Voltage (20mV units): " + String(rdo.outputVoltage * 20) + " mV\n";
      output += "Operating Current (50mA units): " + String(rdo.operatingCurrent * 50) + " mA\n";
    }

  }
  else {
    output += "Unavailable - PD not negotiated \n";
  }

  output += "\n";

  server.send(200, "text/plain", output.c_str());
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}
