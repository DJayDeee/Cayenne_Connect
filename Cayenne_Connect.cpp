/****************************************************************************************
* File :		Cayenne_Connect.cpp
* Date :		2018-Jan-21
* By :			Jean-Daniel Lavoie
* Description :	Inspired from kentaylor/WiFiManager (forked from tzapu/WiFiManager)
*				Manage WiFi connection at every reboot for TIMEOUT seconds whit fixed IP.
*
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
****************************************************************************************/

#include "Cayenne_Connect.h"

bool	Cayenne_Connect::shouldSaveConfig = false;
void	saveConfigCallback(void) {	Cayenne_Connect::shouldSaveConfig = true;	}

//***************************************************************************************
// Constructor : Setup static IPAddress variable, generate default hostname and restore configuration.
//***************************************************************************************
Cayenne_Connect::Cayenne_Connect(void) {
	DEBUG_CC(F("########################################"));
	DEBUG_CC(F("#####|  STARTING Cayenne_Connect  |#####"));
	DEBUG_CC("");
	
	if(!readWiFiConfigFile()) {										// Restore configuration (IP, HostName and debug).
		DEBUG_CC(F("!!===-->  CONTINUE WITHOUT RESTORING SETTINGS  <--===!!"));
	}
	Connect(STATIC_NOHOSTNAME);	
	OpenPortal();													// Open the configuration portal.
	Connect(DHCP_HOSTNAME);
	
	DEBUG_CC(F("#######  Cayenne_Connect DONE  #######"));
	DEBUG_CC(F("######################################\n\n"));
}


//***************************************************************************************
// Called to restore WiFi parameters from the SPIFF CONFIG_FILE.
// Return true if success.
//***************************************************************************************
bool Cayenne_Connect::readWiFiConfigFile(void) {
//	Serial.println(F("*CC: <--===| Restoring config file |===-->"));

	if(!SPIFFS.begin()){											// Exit if unable to mount SPIFlashFileSystem.
		Serial.println(F("*CC: =================ERROR================\n*CC: Unable to mount SPI Flash File System."));
		return false;
	}
	if(!SPIFFS.exists(CONFIG_FILE)) {								// Unmount SPIFF and exit if the confguration file does not exist.
		Serial.println(F("*CC: =============ERROR===========\n*CC: Configuration file not found."));
		SPIFFS.end();
		return false;
	}
	File f = SPIFFS.open(CONFIG_FILE, "r");
	if(!f) {														// Unmount SPIFF and exit if unable to open the configuration file.
		Serial.println(F("*CC: ==================ERROR===============\n*CC: Unable to open the configuration file."));
		SPIFFS.end();
		return false;
	}
	size_t size = f.size();											// Allocate a buffer to store contents of the file.
	std::unique_ptr<char[]> buf(new char[size]);
	f.readBytes(buf.get(), size);
	f.close();														// Closing file and unmount SPIFlashFileSystem.
	SPIFFS.end();

	DynamicJsonBuffer jsonBuffer;									// Using dynamic JSON buffer.
	JsonObject& json = jsonBuffer.parseObject(buf.get());			// Parse JSON string.
	if(!json.success()) {
		Serial.println(F("*CC: ==========ERROR==========\n*CC: JSON parseObject() failed."));
		return false;
	}

	Serial.printf("*CC: Restoring config from file \"%s\" :\n", CONFIG_FILE); // Print what will be restore from the CONFIG_FILE.
	json.prettyPrintTo(Serial);
	Serial.println("");

	char *buf2 = new char[16];										// Parse all parameters and, if required, override local variables with parsed values.
	if(json.containsKey("ssid")) {
		strcpy(ssid, json["ssid"]);
	}
	if(json.containsKey("pass")) {
		strcpy(pass, json["pass"]);
	}
	if(json.containsKey("username")) {
		strcpy(MQTT_credential.username, json["username"]);
	}
	if(json.containsKey("password")) {
		strcpy(MQTT_credential.password, json["password"]);
	}
	if(json.containsKey("clientID")) {
		strcpy(MQTT_credential.clientID, json["clientID"]);
	}
	if(json.containsKey("ip")) {
		strcpy(buf2, json["ip"]);
		staticAddress.ip.fromString(buf2);
	}
	if(json.containsKey("gateway")) {
		strcpy(buf2, json["gateway"]);
		staticAddress.gateway.fromString(buf2);
	}
	if(json.containsKey("subnet")) {
		strcpy(buf2, json["subnet"]);
		staticAddress.subnet.fromString(buf2);
	}
	if(json.containsKey("hostname")) {
		strcpy(staticAddress.hostname, json["hostname"]);
	}
	if(json.containsKey("debug")) {
		debug = json["debug"];
	}
	DEBUG_CC("");
	return true;
}


//***************************************************************************************
// Called to save WiFi parameters to CONFIG_FILE in the FileSystem
// Return true if success.
//***************************************************************************************
bool Cayenne_Connect::writeWiFiConfigFile(void) {
//	Serial.println(F("*CC: <--===| Saving config file |===-->"));

	if(!SPIFFS.begin()){ 											// Exit if unable to mount SPIFlashFileSystem.
		Serial.println(F("*CC: =================ERROR================\n*CC: Unable to mount SPI Flash File System."));
		return false;
	}
	File f = SPIFFS.open(CONFIG_FILE, "w");
	if(!f) {														// Unmount SPIFF and exit if unable to open the configuration file.
		Serial.println(F("*CC: ================ERROR=================\n*CC: Failed to open config file for writing.\n"));
		SPIFFS.end();
		return false;
	}

	DynamicJsonBuffer jsonBuffer;									// Using dynamic JSON buffer which is not the recommended memory model, but anyway, See https://github.com/bblanchon/ArduinoJson/wiki/Memory%20model
	JsonObject& json	= jsonBuffer.createObject();				// Create JSON string.
	json["ssid"]		= ssid;
	json["pass"]		= pass;
	json["username"]	= MQTT_credential.username;					// JSONify local configuration parameters.
	json["password"]	= MQTT_credential.password;
	json["clientID"]	= MQTT_credential.clientID;
	json["ip"]			= staticAddress.ip.toString();
	json["gateway"]		= staticAddress.gateway.toString();
	json["subnet"]		= staticAddress.subnet.toString();
	json["hostname"]	= staticAddress.hostname;
	json["debug"]		= debug;
	
	if(debug) {														// If debug is enable print what will be saved in the CONFIG_FILE.
		Serial.printf("*CC: Saving config to file \"%s\" :\n", CONFIG_FILE);
		json.prettyPrintTo(Serial);
		Serial.println("");
	}
	
	json.printTo(f);												// Write data to file, close it and unmount SPIFlashFileSystem.
	f.close();
	SPIFFS.end();
	return true;
}


//***************************************************************************************
// Called to force WiFi reconnection to a fixed IPAddress and hostname.
// Parameter : bool firstconnect = false	If firstconnect no hostname.
// Return true if success.
//***************************************************************************************
bool Cayenne_Connect::Connect(const int option) const {
	DEBUG_CC(F("<--===| Trying to connect to WiFi NOW |===-->"));
	WiFi.mode(WIFI_STA);											// Force to station mode because if device was switched off while in access point mode it will start up next time in access point mode.
	WiFi.setAutoReconnect(true);
	
	switch(option) {
		case STATIC_HOSTNAME:	WiFi.hostname(staticAddress.hostname);										// Use defined hostname
								WiFi.config(staticAddress.ip, staticAddress.gateway, staticAddress.subnet);	// and fixed IP adress.
								break;
		case STATIC_NOHOSTNAME:	WiFi.hostname("");															// Use blank hostname
								WiFi.config(staticAddress.ip, staticAddress.gateway, staticAddress.subnet);	// and fixed IP adress.
								break;
		case DHCP_HOSTNAME:		WiFi.hostname(staticAddress.hostname);										// Use defined hostname
								WiFi.config(DHCPAddress, DHCPAddress, DHCPAddress);							// and dynamic IP adress.
								break;
		case DHCP_NOHOSTNAME:	WiFi.hostname("");															// Use blank hostname
								WiFi.config(DHCPAddress, DHCPAddress, DHCPAddress);							// and dynamic IP adress.
								break;
		default:				DEBUG_CC(F("UNKNOW (Is it possible?)"));
								return false;
	}
	
	WiFi.reconnect();												// Reconnect if needed to properly set the hostname.
	switch(WiFi.waitForConnectResult()) {
		case WL_CONNECTED:		DEBUG_CC(F("WL_CONNECTED (Successful connection is established)."));	break;
		case WL_IDLE_STATUS:	DEBUG_CC(F("WL_IDLE_STATUS (Changing between statuses)."));				return false;
		case WL_NO_SSID_AVAIL:	DEBUG_CC(F("WL_NO_SSID_AVAIL (Configured SSID cannot be reached)."));	return false;
		case WL_CONNECT_FAILED:	DEBUG_CC(F("WL_CONNECT_FAILED (Password is incorrect?)."));				return false;
		case WL_DISCONNECTED:	DEBUG_CC(F("WL_DISCONNECTED (Not configured in station mode)."));		return false;
		default:				DEBUG_CC(F("UNKNOW (Is it possible?)"));								return false;
	}
	DEBUG_CC("Local HostName is " + WiFi.hostname());
	DEBUG_CC("Local IP is " + WiFi.localIP().toString());
	return true;
}


//***************************************************************************************
// Called to configure and save the WiFi credentials and debug selector.
//		Set class variables whit parameters if required by shouldSaveConfig.
//***************************************************************************************
void Cayenne_Connect::OpenPortal(void) {
	DEBUG_CC(F("<--===| Starting configuration portal |===-->"));
	WiFiManager wifiManager;

	// Format: <ID> <Placeholder text> <default value> <length> <custom HTML> <label placement>.
	WiFiManagerParameter p_username("MQTT_username",	"Cayenne username :",	MQTT_credential.username,	48);
	WiFiManagerParameter p_password("MQTT_password",	"Cayenne password :",	MQTT_credential.password,	48);
	WiFiManagerParameter p_clientID("MQTT_clientID",	"Cayenne clientID :",	MQTT_credential.clientID,	48);
	WiFiManagerParameter p_hostname("hostname",			"Custom hostname :",	staticAddress.hostname,	32);// Create hostname input field.
	char customhtml[24] = "type=\"checkbox\"";						// Create a checkbox for the debug boolean input field.
	char *_value = "T";
	if(debug) {
		strcat(customhtml, " checked");
	}
	WiFiManagerParameter p_debug("debug", "Debug serial printout", _value, 2, customhtml, WFM_LABEL_AFTER);

	wifiManager.addParameter(&p_username);
	wifiManager.addParameter(&p_password);
	wifiManager.addParameter(&p_clientID);
	wifiManager.addParameter(&p_hostname);
	wifiManager.addParameter(&p_debug);
	wifiManager.setDebugOutput(debug);
	wifiManager.setSTAStaticIPConfig(staticAddress.ip, staticAddress.gateway, staticAddress.subnet);// Use STA fixed IP adress.
	if(WiFi.SSID() !="") wifiManager.setConfigPortalTimeout(TIMEOUT); // If access point name exist set a timeout.
	wifiManager.setSaveConfigCallback(saveConfigCallback);

	wifiManager.startConfigPortal();								// Start the config portal.

	DEBUG_CC(F("<--===| Portal closed |===-->"));
	DEBUG_CC("");
	
	if(shouldSaveConfig){
		strcpy(ssid, WiFi.SSID().c_str());
		strcpy(pass, WiFi.psk().c_str());
		strcpy(MQTT_credential.username, const_cast<char*>(p_username.getValue())); // Get the value of each parameters.
		strcpy(MQTT_credential.password, const_cast<char*>(p_password.getValue()));
		strcpy(MQTT_credential.clientID, const_cast<char*>(p_clientID.getValue()));
		staticAddress.ip 			= WiFi.localIP();
		staticAddress.gateway 		= WiFi.gatewayIP();
		staticAddress.subnet 		= WiFi.subnetMask();
		strcpy(staticAddress.hostname, const_cast<char*>(p_hostname.getValue()));
		debug = (strncmp(p_debug.getValue(), "T", 1) == 0);
		writeWiFiConfigFile();										// Save the confguration.
		shouldSaveConfig = false;
	}
}


//***************************************************************************************
// Channel to print the debug.
//***************************************************************************************
template <typename Generic>
void Cayenne_Connect::DEBUG_CC(Generic text) const{
	if (debug) {
		Serial.print("*CC: ");
		Serial.println(text);
	}
}
