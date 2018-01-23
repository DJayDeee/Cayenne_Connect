# Cayenne_Connect

Inspired from kentaylor/WiFiManager (forked from tzapu/WiFiManager)


* TIMEOUT define is used to close the WiFi configuration portal after a numer of seconds.  This apply only if a SSID exist and if there is no connection to the portal.

* CONFIG_FILE is the file name where different parameter are stored in SPIFFS.
  Exemple of what is inside CONFIG_FILE :
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

* DHCPAddress is used to switch from static IP to DHCP address (0.0.0.0).  Does not work whit stable release 2.4.0 of https://github.com/esp8266/Arduino see https://github.com/esp8266/Arduino/pull/4145.

* staticAddress structure contain the IP, GATEWAY, SUBNET adress and hostname.

* MQTT_credential contain Cayenne authentication (username, password and clientID) obtained from the Cayenne Dashboard.

* PIN_LED is the output pin for debuging led.



