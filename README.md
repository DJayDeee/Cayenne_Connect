/****************************************************************************************
 * File :         README.md
 * Date :         2017-Dec-18
 * By :           Jean-Daniel Lavoie
 * Description :  Inspired from kentaylor/WiFiManager (forked from tzapu/WiFiManager)
 *
 ****************************************************************************************/

TIMEOUT define is used to close the WiFi configuration portal if a SSID exist.  Set to 180 seconds.

CONFIG_FILE is the name of the Configuration file stored in SPIFF.  Set to "/CC_config.json".
Exemple of what is inside CONFIG_FILE :
	{	"username":"xxx",
		"password":"yyy",
		"clientID":"zzz",
		"ip":"192.168.0.3",
		"gateway":"192.168.0.1",
		"subnet":"255.255.255.0",
		"hostname":"ESP1234567",
		"debug":1
	}

NO_HOSTNAME is the parameter used to Connect() withhout hostname.  Set to true.

staticAddress contain the IP, GATEWAY, SUBNET adress and hostname.
typedef struct staticAddress {
	IPAddress	ip = IPAddress(192,168,0,3);
	IPAddress	gateway = IPAddress(192,168,0,1);
	IPAddress	subnet = IPAddress(255,255,255,0);
	char		hostname[32];
};

MQTT_credential contain Cayenne authentication info. This should be obtained from the Cayenne Dashboard.
typedef struct MQTT_credential {	
	char username[48];
	char password[48];
	char clientID[48];
};

PIN_LED is to debug.  Set to 2.

LED_Init() initialise the PIN_LED in output mode and turn it off.
LED_ON() turn the PIN_LED high.
LED_OFF() turn the PIN_LED low.
LED_Toggle() invert the PIN_LED state.


Cayenne_Connect() restore configuration from CONFIG_FILE, connect WiFi, start WiFiManager, save configuration to CONFIG_FILE and reconnect whit hostname.
setDebugOutput(boolean debug) is called to activate the debug channel over serial if parameter is set to true.  ALSO SET IN THE WiFiManager PORTAL.
	
char*	getMQTTusername() return Cayenne username.
char*	getMQTTpassword() return Cayenne password.
char*	getMQTTclientID() return Cayenne clientID.

