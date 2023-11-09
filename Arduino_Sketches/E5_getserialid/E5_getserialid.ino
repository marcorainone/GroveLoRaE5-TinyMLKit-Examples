// **********************************************************************************
// serialid.ino
//             v1 2023-10-25, ICTP Wireless Lab
// Version for Nano 33 BLE Sense
// Programmer: Marco Rainone - ICTP Wireless Lab
// LoraE5 connected to Tiny Machine Learning Kit (https://store.arduino.cc/products/arduino-tiny-machine-learning-kit)
//
// This sketch read DevAddr, DevEUI, AppEUI stored in LoraE5
// LoRaWAN knows a number of identifiers for devices, applications and gateways.
// DevAddr : 32 bit device address (non-unique)
// DevEUI  : 64 bit end-device identifier, EUI-64 (unique)
// AppEUI (JOINEUI) : 64 bit application identifier, EUI-64 (unique)
//
// commands sent:
// 1) AT		ANSWER: +AT: OK
// 2) AT+ID
// ANSWER EXAMPLE:
// +ID: DevAddr, 25:1B:2E:11
// +ID: DevEui, 2C:F7:F1:21:42:60:90:CE
// +ID: AppEui, 80:00:00:00:00:00:00:06
// **********************************************************************************

//--------------------------------------------------
#include <Arduino.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

// lora e5 module pins on grove connector (https://wiki.seeedstudio.com/Grove_LoRa_E5_New_Version/)
// pin           Function
// pin1 Yellow   RX  Serial Receive
// pin2 White    TX  Serial Transmit
// pin3 Red      VCC Power for Grove Module, 5V/3.3V
// pin4 Black    GND Ground

// loraE5 state values
enum state_loraE5 {
    SEND_AT,
    GET_IDENTIFIERS,
	COMMUNICATION_ERROR
};
state_loraE5 state;

// Initialize LoRa module
UART LoraSerial(A7, A6); 								// A7 is tx, A6 is rx

static char tx_buff[255];								// tx buffer
static char rx_buff[512];								// rx buffer

static int cmdLoraE5(char *p_cmd, char *p_ack, int timeout_ms)
{
	int ch;
	int num = 0;
	int index = 0;
	int startMillis;
	va_list args;

	memset(rx_buff, 0, sizeof(rx_buff));								// clear rx buffer
	strncpy(tx_buff, p_cmd, sizeof(tx_buff));
	strncat(tx_buff, "\r\n", sizeof(tx_buff) - strlen(tx_buff));		// append end of command /r/n

	Serial.println(tx_buff);
	LoraSerial.println(tx_buff);
	delay(200);
	if (p_ack == NULL)
	{
		return 0;
	}
	startMillis = millis();
	do
	{
		while (LoraSerial.available() > 0)
		{
			// chars available
			ch = LoraSerial.read();
			rx_buff[index++] = ch;
			Serial.print((char)ch);
			delay(2);
		}
		if (strstr(rx_buff, p_ack) != NULL)
		{
			// msg received. Wait 200msec before to send another msg
			delay(200);
			return (1);
		}
	} while (millis() - startMillis < timeout_ms);
	return 0;
}

void setup() {
	Serial.begin(115200);
	pinMode(LED_BUILTIN, OUTPUT);

	LoraSerial.begin(9600);
	Serial.println("LoraE5 get DevAddr, DevEUI, AppEUI");
	// initialize state
	state = SEND_AT;
}

void loop() {
	digitalWrite(LED_BUILTIN, HIGH);
	switch(state)
	{
	case SEND_AT				:			// send AT msg: check if lora E5 is connected
		if(!cmdLoraE5("AT", "+AT: OK", 100))
		{
			state = COMMUNICATION_ERROR;
			break;
		}
		state = GET_IDENTIFIERS;			// the lora E5 answers. Pass to next state
		break;
	case GET_IDENTIFIERS		:			// read DevAddr, DevEUI, AppEUI stored in LoraE5
		if(!cmdLoraE5("AT+ID", "+ID: AppEui", 1000))
		{
			state = COMMUNICATION_ERROR;
			break;
		}
		// remains in this state.
		break;
	default						:
		state = SEND_AT;					// restart state machine
		break;
	}
	if(state == COMMUNICATION_ERROR)
	{
		Serial.println("LoraE5 module not connected ...");
		state = SEND_AT;					// restart state machine
	}
	digitalWrite(LED_BUILTIN, LOW);
	delay(2000);
}
