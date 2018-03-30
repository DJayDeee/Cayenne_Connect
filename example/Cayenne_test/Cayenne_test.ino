/****************************************************************************************
* File	:		Cayenne_test.ino
* Date	:		2018-Mar-30
* By	:		Jean-Daniel Lavoie
* Description :	Inspired from kentaylor/WiFiManager (forked from tzapu/WiFiManager)
*		Manage WiFi connection at every reboot for 60 seconds whit fixed IP.
*		Connect to cayenne.mydevices.com
****************************************************************************************/

#include <Cayenne_Connect.h>							//https://github.com/DJayDeee/Cayenne_Connect
_MQTT_credential Cayenne_credential;					// Cayenne authentication info. This should be obtained from the Cayenne Dashboard.

#define CAYENNE_DEBUG
#define CAYENNE_PRINT Serial
#include <CayenneMQTTESP8266.h>							//https://github.com/myDevicesIoT/Cayenne-MQTT-ESP

#define	RESTART_REQ_CH		0														// Cayenne channel for general commands.
#define	SEND_REQ_CH			1
#include <queue>																	// std::queue
typedef enum user_request {
	RESTART,																		// Reset event.
	SEND,																			// Send event.
} user_request;
std::queue<user_request> request_queue;

LED Debug_LED;

//***************************************************************************************
// setup()		Put your setup code here, to run once:
//***************************************************************************************
void setup(void) {
	Debug_LED.on();																	// Turn on LED during BOOT.

	Serial.begin(74880);															// Start serial for debug.
	Serial.println(F(" "));															// First print is garbage.

	Cayenne_Connect my_connection;													// Manage WiFi connection whit staticIP.
	if (WiFi.isConnected()) {														// Turn LED OFF if connected.
		Debug_LED.off();
	}

	Cayenne_credential = my_connection.getCayenne_credential();
	Cayenne.begin(Cayenne_credential.username, Cayenne_credential.password, Cayenne_credential.clientID);	// Start Cayenne MQTT client.
	
	request_queue.push(SEND);														// Update Cayenne Dashboard after bootup.
}


//***************************************************************************************
// loop()		Endless storie
//***************************************************************************************
void loop() {
	Debug_LED.toggle();
	Cayenne.loop();																	// Take time to do Cayenne and WiFi techy stuf.
	yield();

	if (!request_queue.empty()) {													// Process only if their is job to do.
		switch (request_queue.front()) {											// Get the next job.
			case SEND:																// Update Cayenne dashboard.
// Put your Cayenne.virtualWrite(...); here
				Cayenne.virtualWrite(SEND_REQ_CH, false);							// Update cayenne_send button.
				Serial.println(F("Cayenne dashboard updated."));
				break;

			case RESTART:															// Reset!!
				Cayenne.virtualWrite(RESTART_REQ_CH, false);						// Update cayenne_restart button.
				Serial.println(F("!!Restarting NOW!!"));
				ESP.restart();
				delay(5000);
				break;

			default:
				Serial.println(F("!!Unknow user_request!!"));
		}
		request_queue.pop();
	}
}


//***************************************************************************************
// Function for processing actuator commands from the Cayenne Dashboard.
//***************************************************************************************
CAYENNE_IN(RESTART_REQ_CH) {
	CAYENNE_LOG("Restart requested.");
	request_queue.push(RESTART);
}


CAYENNE_IN(SEND_REQ_CH) {
	CAYENNE_LOG("Cayenne dashboard update requested.");
	request_queue.push(SEND);
}

CAYENNE_IN_DEFAULT() {
	CAYENNE_LOG("CAYENNE_IN_DEFAULT(%u) - %s, %s", request.channel, getValue.getId(), getValue.asString());
	//Process message here. If there is an error set an error message using getValue.setError(), e.g getValue.setError("Error message");
}


