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

int	Cayenne_Connect::loop_delay = 1000;
bool	Cayenne_Connect::shouldSaveConfig = false;
void	saveConfigCallback(void) {	Cayenne_Connect::shouldSaveConfig = true;	}

//***************************************************************************************
// Constructor : Setup static IPAddress variable, generate default hostname and restore configuration.
//***************************************************************************************
Cayenne_Connect::Cayenne_Connect(void) {
	DEBUG_CC(F("#####|  STARTING Cayenne_Connect  |#####"), true);

	if(!readWiFiConfigFile(CONFIG_FILE)) {					// Restore configuration (WiFi,IP, HostName, Cayenne and debug).
		DEBUG_CC(F("!!===-->  CONTINUE WITHOUT RESTORING SETTINGS  <--===!!"));
	}
	Connect(STATIC_NOHOSTNAME);						// Try to connect to the last know wifi using a static IP address.
	OpenPortal();								// Open the configuration portal .
	Connect(DHCP_HOSTNAME);							//

	DEBUG_CC(F("######|  Cayenne_Connect DONE  |######"), true);
}


//***************************************************************************************
// Called to restore WiFi parameters from the SPIFF CONFIG_FILE.
// Return true if success.
//***************************************************************************************
bool Cayenne_Connect::readWiFiConfigFile(const char *filename ) {
	Serial.println(F("*CC: <--===| Restoring config file |===-->"));

	if(!SPIFFS.begin()){
		Serial.println(F("*CC: =================ERROR================\n*CC: Unable to mount SPI Flash File System."));
		return false;
	}
	if(!SPIFFS.exists(filename)) {
		Serial.println(F("*CC: =============ERROR===========\n*CC: Configuration file not found."));
		SPIFFS.end();
		return false;
	}
	File f = SPIFFS.open(filename, "r");
	if(!f) {
		Serial.println(F("*CC: ==================ERROR===============\n*CC: Unable to open the configuration file."));
		SPIFFS.end();
		return false;
	}
	size_t size = f.size();							// Allocate and store contents of CONFIG_FILE in a buffer.
	std::unique_ptr<char[]> buf(new char[size]);
	f.readBytes(buf.get(), size);
	f.close();								// Closing file and unmount SPIFlashFileSystem.
	SPIFFS.end();

	DynamicJsonBuffer jsonBuffer;						// Using dynamic JSON buffer.
	JsonObject& json = jsonBuffer.parseObject(buf.get());			// Parse JSON string.
	if(!json.success()) {
		Serial.println(F("*CC: ==========ERROR==========\n*CC: JSON parseObject() failed."));
		return false;
	}

	Serial.printf("*CC: Restoring config from file \"%s\" :\n", filename); // Print what will be restore from the CONFIG_FILE.
	json.prettyPrintTo(Serial);
	Serial.println("");

	char *buf2 = new char[16];						// Parse all parameters and override local variables.
#ifndef	_CAYENNEMQTTESP8266_h
	if(json.containsKey(String(F("username")))) {
		strcpy(Cayenne_credential.username, json[String(F("username"))]);
	}
	if(json.containsKey(String(F("password")))) {
		strcpy(Cayenne_credential.password, json[String(F("password"))]);
	}
	if(json.containsKey(String(F("clientID")))) {
		strcpy(Cayenne_credential.clientID, json[String(F("clientID"))]);
	}
	if(json.containsKey(String(F("loop_delay")))) {
		loop_delay = json[String(F("loop_delay"))];
	}
#endif
	if(json.containsKey(String(F("ip")))) {
		strcpy(buf2, json[String(F("ip"))]);
		staticAddress.ip.fromString(buf2);
	}
	if(json.containsKey(String(F("gateway")))) {
		strcpy(buf2, json[String(F("gateway"))]);
		staticAddress.gateway.fromString(buf2);
	}
	if(json.containsKey(String(F("subnet")))) {
		strcpy(buf2, json[String(F("subnet"))]);
		staticAddress.subnet.fromString(buf2);
	}
	if(json.containsKey(String(F("hostname")))) {
		strcpy(staticAddress.hostname, json[String(F("hostname"))]);
	}
	if(json.containsKey(String(F("debug")))) {
		debug = json[String(F("debug"))];
	}
	if(json.containsKey(String(F("timeout")))) {
		timeout = json[String(F("timeout"))];
	}
	DEBUG_CC("");
	return true;
}


//***************************************************************************************
// Called to save WiFi parameters to CONFIG_FILE in the FileSystem
// Return true if success.
//***************************************************************************************
bool Cayenne_Connect::writeWiFiConfigFile(const char *filename) {
	Serial.println(F("*CC: <--===| Saving config file |===-->"));

	if(!SPIFFS.begin()){
		Serial.println(F("*CC: =================ERROR================\n*CC: Unable to mount SPI Flash File System."));
		return false;
	}
	File f = SPIFFS.open(filename, "w");
	if(!f) {
		Serial.println(F("*CC: ================ERROR=================\n*CC: Failed to open config file for writing.\n"));
		SPIFFS.end();
		return false;
	}

	DynamicJsonBuffer jsonBuffer;						// Using dynamic JSON buffer which is not the recommended memory model, but anyway, See https://github.com/bblanchon/ArduinoJson/wiki/Memory%20model
	JsonObject& json	= jsonBuffer.createObject();			// Create JSON string.
#ifndef	_CAYENNEMQTTESP8266_h
	json[String(F("username"))]	= Cayenne_credential.username;		// JSONify local configuration parameters.
	json[String(F("password"))]	= Cayenne_credential.password;
	json[String(F("clientID"))]	= Cayenne_credential.clientID;
	json[String(F("loop_delay"))]	= loop_delay;
#endif
	json[String(F("ip"))]		= staticAddress.ip.toString();
	json[String(F("gateway"))]	= staticAddress.gateway.toString();
	json[String(F("subnet"))]	= staticAddress.subnet.toString();
	json[String(F("hostname"))]	= staticAddress.hostname;
	json[String(F("debug"))]	= debug;
	json[String(F("timeout"))]	= timeout;

	if(debug) {								// If debug is enable print what will be saved in the CONFIG_FILE.
		Serial.printf("*CC: Saving config to file \"%s\" :\n", filename);
		json.prettyPrintTo(Serial);
		Serial.println("");
	}

	json.printTo(f);							// Write data to file, close it and unmount SPIFlashFileSystem.
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
	WiFi.mode(WIFI_STA);							// Force to station mode because if device was switched off while in access point mode it will start up next time in access point mode.
	WiFi.setAutoReconnect(true);

	switch(option) {
		case STATIC_HOSTNAME:	WiFi.hostname(staticAddress.hostname);						// Use defined hostname
					WiFi.config(staticAddress.ip, staticAddress.gateway, staticAddress.subnet);	// and fixed IP adress.
					break;
		case STATIC_NOHOSTNAME:	WiFi.hostname("");								// Use blank hostname
					WiFi.config(staticAddress.ip, staticAddress.gateway, staticAddress.subnet);	// and fixed IP adress.
					break;
		case DHCP_HOSTNAME:	WiFi.hostname(staticAddress.hostname);						// Use defined hostname
					WiFi.config(DHCPAddress, DHCPAddress, DHCPAddress);				// and dynamic IP adress.
					break;
		case DHCP_NOHOSTNAME:	WiFi.hostname("");								// Use blank hostname
					WiFi.config(DHCPAddress, DHCPAddress, DHCPAddress);				// and dynamic IP adress.
					break;
		default:		DEBUG_CC(F("UNKNOW (Is it possible?)"));
					return false;
	}

	WiFi.reconnect();							// Reconnect to properly set the hostname.
	switch(WiFi.waitForConnectResult()) {
		case WL_CONNECTED:	DEBUG_CC(F("WL_CONNECTED (Successful connection is established)."));	break;
		case WL_IDLE_STATUS:	DEBUG_CC(F("WL_IDLE_STATUS (Changing between statuses)."));		return false;
		case WL_NO_SSID_AVAIL:	DEBUG_CC(F("WL_NO_SSID_AVAIL (Configured SSID cannot be reached)."));	return false;
		case WL_CONNECT_FAILED:	DEBUG_CC(F("WL_CONNECT_FAILED (Password is incorrect?)."));		return false;
		case WL_DISCONNECTED:	DEBUG_CC(F("WL_DISCONNECTED (Not configured in station mode)."));	return false;
		default:		DEBUG_CC(F("UNKNOW (Is it possible?)"));				return false;
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
#ifndef	_CAYENNEMQTTESP8266_h							// Should we get Cayenne credentials.
	WiFiManagerParameter p_username("MQTT_username",	"Cayenne username :",	Cayenne_credential.username,	48);
	WiFiManagerParameter p_password("MQTT_password",	"Cayenne password :",	Cayenne_credential.password,	48);
	WiFiManagerParameter p_clientID("MQTT_clientID",	"Cayenne clientID :",	Cayenne_credential.clientID,	48);
	char _loop_delay[6];
	sprintf(_loop_delay, "%d", loop_delay);
	WiFiManagerParameter p_loop_delay("loop_delay",		"Cayenne loop delay :",	_loop_delay,			6);
	wifiManager.addParameter(&p_username);
	wifiManager.addParameter(&p_password);
	wifiManager.addParameter(&p_clientID);
	wifiManager.addParameter(&p_loop_delay);
#endif
	WiFiManagerParameter p_hostname("hostname",		"Custom hostname :",	staticAddress.hostname,		32);
	wifiManager.addParameter(&p_hostname);

	char customhtml[24] = "type=\"checkbox\"";				// Create a checkbox for the debug boolean input field.
	char *_value = "T";
	if(debug) {
		strcat(customhtml, " checked");
	}
	WiFiManagerParameter p_debug("debug", "Debug serial printout", _value, 2, customhtml, WFM_LABEL_AFTER);
	wifiManager.addParameter(&p_debug);

	char _timeout[6];
	sprintf(_timeout, "%d", timeout);
	WiFiManagerParameter p_timeout("timeout_delay",		"Configuration portal timeout :",	_timeout,	6);
	wifiManager.addParameter(&p_timeout);

	wifiManager.setDebugOutput(debug);
	wifiManager.setSTAStaticIPConfig(staticAddress.ip, staticAddress.gateway, staticAddress.subnet);	// Field for STA fixed IP adress.
	if(WiFi.SSID() != "") wifiManager.setConfigPortalTimeout(timeout);	// If access point name exist set a timeout.
	wifiManager.setSaveConfigCallback(saveConfigCallback);			// Callback to set shouldSaveConfig flag only if you it save button in portal.

	wifiManager.startConfigPortal();					// Start the config portal.

	DEBUG_CC(F("<--===| Portal closed |===-->"));
	DEBUG_CC("");

	if(shouldSaveConfig){							// Gater each configuration parameters only if we should save it.
#ifndef	_CAYENNEMQTTESP8266_h
		strcpy(Cayenne_credential.username, const_cast<char*>(p_username.getValue()));
		strcpy(Cayenne_credential.password, const_cast<char*>(p_password.getValue()));
		strcpy(Cayenne_credential.clientID, const_cast<char*>(p_clientID.getValue()));
		loop_delay = atoi(p_loop_delay.getValue());
#endif
		staticAddress.ip 	= WiFi.localIP();			// Get the IP adress we are currently using.
		staticAddress.gateway	= WiFi.gatewayIP();
		staticAddress.subnet	= WiFi.subnetMask();
		strcpy(staticAddress.hostname, const_cast<char*>(p_hostname.getValue()));

		debug = (strncmp(p_debug.getValue(), "T", 1) == 0);		// Get advanced parameters.
		timeout = atoi(p_timeout.getValue());
		writeWiFiConfigFile(CONFIG_FILE);				// Save the confguration and reset his flag.
		shouldSaveConfig = false;
	}
}


//***************************************************************************************
// Channel to print the debug.
//***************************************************************************************
template <typename Generic>
void Cayenne_Connect::DEBUG_CC(Generic text, const bool force) const{
	if (debug || force) {
		Serial.print("*CC: ");
		Serial.println(text);
	}
}
