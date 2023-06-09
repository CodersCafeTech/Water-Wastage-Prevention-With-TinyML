#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>

const char DEVICE_LOGIN_NAME[]  = "c48693d3-XXXX-XXXX-832d-d4385ba86811";

const char SSID[]               = "SSID";    // Network SSID (name)
const char PASS[]               = "PASSWORD";    // Network password (use for WPA, or use as key for WEP)
const char DEVICE_KEY[]  = "RZG6QYHXXXXAMXKPU2UV";    // Secret device password

void onWastageChange();

float flowRate;
CloudSwitch wastage;

void initProperties(){

  ArduinoCloud.setBoardId(DEVICE_LOGIN_NAME);
  ArduinoCloud.setSecretDeviceKey(DEVICE_KEY);
  ArduinoCloud.addProperty(flowRate, READ, 1 * SECONDS, NULL);
  ArduinoCloud.addProperty(wastage, READWRITE, ON_CHANGE, onWastageChange);

}

WiFiConnectionHandler ArduinoIoTPreferredConnection(SSID, PASS);
