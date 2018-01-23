/****************************************************************************************
* File	:		HelloWorld.ino
* Date	:		2017-Dec-14
* By	:		Jean-Daniel Lavoie
* Description :	Inspired from kentaylor/WiFiManager (forked from tzapu/WiFiManager)
*		Manage WiFi connection at every reboot for 60 seconds whit fixed IP.
*		Connect to cayenne.mydevices.com
****************************************************************************************/

#include <Cayenne_Connect.h>							//https://github.com/DJayDeee/Cayenne_Connect

//#define CAYENNE_DEBUG
#define CAYENNE_PRINT Serial
#include <CayenneMQTTESP8266.h>							//https://github.com/myDevicesIoT/Cayenne-MQTT-ESP

char username[48];								// Cayenne authentication info. This should be obtained from the Cayenne Dashboard.
char password[48];
char clientID[48];

#include <queue>								// std::queue
typedef enum user_request {
	RESTART,								// Reset event.
	SEND,									// Send event.
} user_request;
std::queue<user_request> request_queue;

LED Debug_LED;

//***************************************************************************************
// setup()		Put your setup code here, to run once:
//***************************************************************************************
void setup(void) {
	Debug_LED.on();								// Turn on LED during BOOT.

	Serial.begin(74880);							// Start serial for debug.
	Serial.println(F(" "));							// First print is garbage.

	Cayenne_Connect my_connection;						// Manage WiFi connection whit staticIP.
	if (WiFi.isConnected()) {						// Turn LED OFF if connected.
		Debug_LED.off();
	}

	strcpy(username, my_connection.getMQTTusername());
	strcpy(password, my_connection.getMQTTpassword());
	strcpy(clientID, my_connection.getMQTTclientID());
	Cayenne.begin(username, password, clientID);				// Start Cayenne MQTT client.
}


//***************************************************************************************
// loop()		Endless storie
//***************************************************************************************
void loop() {
	Cayenne.loop();
	yield();

	if (!request_queue.empty()) {
		switch (request_queue.front()) {
			case SEND:						// Update Cayenne dashboard.
				Debug_LED.toggle();
// Put your Cayenne.virtualWrite(...); here
				Cayenne.virtualWrite(1, false);			// Update cayenne_send button.
				Serial.println(F("Cayenne server updated."));
				break;

			case RESTART:						// Reset!!
				Cayenne.virtualWrite(0, false);			// Update cayenne_restart button.
				Serial.println(F("!!Restarting NOW!!"));
				ESP.restart();
				delay(5000);
				break;

			default:
				CAYENNE_LOG("Unknow user_request");
		}
		request_queue.pop();
	}
}


//***************************************************************************************
// Function for processing actuator commands from the Cayenne Dashboard.
//***************************************************************************************
CAYENNE_IN(0) {
	request_queue.push(RESTART);
	CAYENNE_LOG("Restart requested.");
}


CAYENNE_IN(1) {
	request_queue.push(SEND);
	CAYENNE_LOG("Cayenne write requested.");
}


CAYENNE_IN_DEFAULT() {
	CAYENNE_LOG("CAYENNE_IN_DEFAULT(%u) - %s, %s", request.channel, getValue.getId(), getValue.asString());
	//Process message here. If there is an error set an error message using getValue.setError(), e.g getValue.setError("Error message");
}
