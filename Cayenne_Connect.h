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

#include	<FS.h>								//https://github.com/pellepl/spiffs
#include	<ArduinoJson.h>							//https://github.com/bblanchon/ArduinoJson
#include	<ESP8266WiFi.h>							//https://github.com/esp8266/Arduino
#include	<WiFiManager.h>							//https://github.com/kentaylor/WiFiManager
#include 	<stdlib.h>							//to use atoi()

#ifndef	CONFIG_FILE
#define	CONFIG_FILE	"/CC_config.json"					// CONFIG_FILE is the name of the Configuration file.
#endif
void	saveConfigCallback(void);						// Callback helper to set shouldSaveConfig flag.


// Option for Connect(option)
#define	STATIC_HOSTNAME		0						// Parameter to Connect() to static IP with hostname.
#define	STATIC_NOHOSTNAME	1						// Parameter to Connect() to static IP without hostname.
#define	DHCP_HOSTNAME		2						// Parameter to Connect() to dynamic IP with hostname.
#define	DHCP_NOHOSTNAME		3						// Parameter to Connect() to dynamic IP without hostname.

static	const	IPAddress	DHCPAddress = IPAddress(0,0,0,0);		// IP adress used to force DHCP in STA mode.  https://github.com/esp8266/Arduino/pull/4145
typedef struct _staticAddress {							// Container for the IP, GATEWAY, SUBNET address and HOSTNAME.
	IPAddress	ip	= IPAddress(192,168,0,3);
	IPAddress	gateway	= IPAddress(192,168,0,1);
	IPAddress	subnet	= IPAddress(255,255,255,0);
	char		hostname[32];
};

typedef struct _MQTT_credential {						// Cayenne authentication info container.
	char	username[48];
	char	password[48];
	char	clientID[48];
};

#define	PIN_LED		2							// Default led pin for debugging.
class LED {
  public:
	LED(const int _pin = PIN_LED)	{ led_pin = _pin;	pinMode(led_pin, OUTPUT);	digitalWrite(led_pin, HIGH);	}
	void	off(void)		{ digitalWrite(led_pin, HIGH);			}
	void	on(void)		{ digitalWrite(led_pin, LOW);			}
	void	toggle(void)		{ digitalWrite(led_pin, !digitalRead(led_pin));	}
  private:
	int	led_pin;
};

class Cayenne_Connect {
  public:
	Cayenne_Connect(void);							// Restore configuration, connect WiFi whit static IP, start WiFiManager, save configuration if needed and reconnect WiFi whit dynamic IP and hostname.
	void		setDebugOutput(const bool _debug) { debug = _debug; }	// Called to enable/disable the debug information over serial.
/*
	char*		getMQTTusername(void) {	return Cayenne_credential.username; }
	char*		getMQTTpassword(void) {	return Cayenne_credential.password; }
	char*		getMQTTclientID(void) {	return Cayenne_credential.clientID; }
*/
	_MQTT_credential	getCayenne_credential(void) {	return Cayenne_credential; }

	static	bool	shouldSaveConfig;					// Flag for saving data.

//	static _MQTT_credential	Cayenne_credential;
	static int		loop_delay;

  private:
	char			ssid[32];
	char			pass[32];
	_staticAddress		staticAddress;
	_MQTT_credential	Cayenne_credential;
	bool			debug = true;
	int			timeout = 180;

	bool		readWiFiConfigFile(void);				// Restore configuration from the FileSystem.
	bool		writeWiFiConfigFile(void);				// Save WiFi configuration to the FileSystem.

	bool		Connect(const int option = STATIC_HOSTNAME) const;	// Force WiFi connection, see #define for option.
	void		OpenPortal(void);					// Configure the WiFi via WiFiManager portal.

	template <typename Generic>						// Debug channel enable if debug is set.
	void		DEBUG_CC(Generic text, const bool force = false) const;

};

#endif // _CAYENNE_CONNECT_H_
