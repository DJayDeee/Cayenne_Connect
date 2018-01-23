# Cayenne_Connect
WiFi and Cayenne connection helper for ESP8266.  Open a WiFi configuration portal to enter Cayenne authentication info.
Used to set :
* WiFi SSID
* WiFi password
* hostname
* Cayenne username
* Cayenne password
* Cayenne clientID
* enable/disable serial debug informations
* static  IP address
* gateway IP address
* subnet  IP address

It restore configuration, connect WiFi whit static IP, start WiFiManager (STA + AP), save configuration if needed and reconnect WiFi whit dynamic IP and hostname.  **For an unfortunate reason Cayenne does not connect if we use static IP address.**

## Contents
- [Defines and structures](#defines-and-structures)  
- [Functions](#functions)
- [License and credits](#license-and-credits)

### Defines and structures
* TIMEOUT define is used to close the WiFi configuration portal after a numer of seconds.  This apply only if a SSID exist and if there is no connection to the portal.

* CONFIG_FILE is the file name where different parameter are stored in SPIFFS.
  Exemple of what is inside CONFIG_FILE :
```bash
	{
	  "ssid": "yourwifiname",
	  "pass": "yourwifipassword",
	  "username": "yourcayenneusername",
	  "password": "yourcayennepassword",
	  "clientID": "yourcayenneclientid",
	  "ip": "192.168.0.3",
	  "gateway": "192.168.0.1",
	  "subnet": "255.255.255.0",
	  "hostname": "youresp",
	  "debug": true
	}
```

* DHCPAddress is used to switch from static IP to DHCP address (0.0.0.0).  Does not work whit stable release 2.4.0 [Arduino for ESP8266](https://github.com/esp8266/Arduino) see issue [https://github.com/esp8266/Arduino/pull/4145](https://github.com/esp8266/Arduino/pull/4145).

* _staticAddress structure contain the IP, GATEWAY, SUBNET adress and hostname.

* _MQTT_credential  contain Cayenne authentication (username, password and clientID) obtained from the Cayenne Dashboard.

* PIN_LED is the default output pin for debuging led.

### Functions
* LED(const int _pin = PIN_LED) instanciate LED class and turn it off.
	* off()		Turn led off.
	* on()		Turn led off.
	* toggle()	Toggle led state.
	
* Cayenne_Connect() instanciate class, restore configuration, connect WiFi whit static IP, start WiFiManager, save configuration if needed and reconnect WiFi whit dynamic IP and hostname.
	* setDebugOutput(const bool _debug) to see debuging information via serial connection.
	* getMQTTusername() return Cayenne authentication parameters.
	* getMQTTpassword()
	* getMQTTclientID()

### License and credits
- Inspired from [kentaylor/WiFiManager](https://github.com/kentaylor/WiFiManager) (forked from [tzapu/WiFiManager](https://github.com/tzapu/WiFiManager))
- [Arduino for ESP8266](https://github.com/esp8266/Arduino)
- [SPI Flash File System (SPIFFS)](https://github.com/pellepl/spiffs)
- [C++ JSON library for IoT](https://github.com/bblanchon/ArduinoJson) [https://arduinojson.org](https://arduinojson.org)
- [Cayenne for ESP](https://github.com/myDevicesIoT/Cayenne-MQTT-ESP)
