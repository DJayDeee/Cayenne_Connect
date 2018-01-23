/****************************************************************************************
* File :		Cayenne_Connect.h
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

#ifndef		_CAYENNE_CONNECT_H_
#define		_CAYENNE_CONNECT_H_

#include	<FS.h>
#include	<ArduinoJson.h>												//https://github.com/bblanchon/ArduinoJson
#include	<ESP8266WiFi.h>												//https://github.com/esp8266/Arduino
#include	<WiFiManager.h>												//https://github.com/kentaylor/WiFiManager

#define	TIMEOUT			180												// TIMEOUT to close the WiFi configuration portal if a SSID exist.
#define	CONFIG_FILE		"/CC_config.json"								// CONFIG_FILE is the name of the Configuration file.
void	saveConfigCallback(void);


// Option for Connect(option)
#define	STATIC_HOSTNAME		0											// Parameter to Connect() to static IP with hostname.
#define	STATIC_NOHOSTNAME	1											// Parameter to Connect() to static IP without hostname.
#define	DHCP_HOSTNAME		2											// Parameter to Connect() to dynamic IP with hostname.
#define	DHCP_NOHOSTNAME		3											// Parameter to Connect() to dynamic IP without hostname.

static	const	IPAddress	DHCPAddress	= IPAddress(0,0,0,0);					// IP adress used to force DHCP in STA mode.
typedef struct _staticAddress {											// Container for the IP, GATEWAY and SUBNET adress.
	IPAddress	ip		= IPAddress(192,168,0,3);
	IPAddress	gateway	= IPAddress(192,168,0,1);
	IPAddress	subnet	= IPAddress(255,255,255,0);
	char		hostname[32];
};

typedef struct _MQTT_credential {	
	char username[48];													// Cayenne authentication info. This should be obtained from the Cayenne Dashboard.
	char password[48];
	char clientID[48];
};

#define	PIN_LED			2												// PIN_LED is to know if we are in configuration mode.
class LED {
  public:
	LED(const int _pin = PIN_LED) {	led_pin = _pin;	pinMode(led_pin, OUTPUT);	digitalWrite(led_pin, HIGH);	}
	void	off(void)		{	digitalWrite(led_pin, HIGH);							}
	void	on(void)		{	digitalWrite(led_pin, LOW);								}
	void	toggle(void)	{	digitalWrite(led_pin, !digitalRead(led_pin));			}	
  private:
	int		led_pin;
};

class Cayenne_Connect {
  public:
	Cayenne_Connect(void);												// Restore configuration, connect WiFi, start WiFiManager, save configuration and reconnect whit hostname, return true if connected.
	void		setDebugOutput(const bool _debug) {	debug = _debug;	}		// Called to print or not the debug information over serial.
	
	char*		getMQTTusername(void) {	return MQTT_credential.username;	}
	char*		getMQTTpassword(void) {	return MQTT_credential.password;	}
	char*		getMQTTclientID(void) {	return MQTT_credential.clientID;	}
	
	static	bool		shouldSaveConfig;								// Flag for saving data.
	
  private:
	char				ssid[32];
	char				pass[32];
	_staticAddress		staticAddress;
	_MQTT_credential	MQTT_credential;
	bool				debug = true;									// Print debuging informations by default.
	
	bool		readWiFiConfigFile(void);								// Restore configuration from the FileSystem.
	bool		writeWiFiConfigFile(void);								// Save WiFi configuration (IP, HostName and debug) to the FileSystem.
	
	bool		Connect(const int option = STATIC_HOSTNAME) const;		// Force WiFi connection, see #define for option.
	void		OpenPortal(void);										// Configure the WiFi via WiFiManager portal.

	template <typename Generic>											// Debug channel actif if debug is set.
	void		DEBUG_CC(Generic text) const;

};

#endif // _CAYENNE_CONNECT_H_

