// **********************************************************************************
// ttn3e5_acq_SI1145_grove.ino
//             v1 2023-10-25, ICTP Wireless Lab
// Programmer: Marco Rainone - ICTP Wireless Lab
// 
// TTN3 application to read the Si1151 Grove Sunlight Sensor
// doc:
// https://wiki.seeedstudio.com/Grove-Digital_Light_Sensor/
// https://github.com/Seeed-Studio/Grove_Digital_Light_Sensor
// 
// Version for Nano 33 BLE Sense and Lite.
//
// This code is licensed under the terms of the MIT license. 
// You can find the complete file here: https://opensource.org/license/mit/
// **********************************************************************************

#include <Arduino.h>
#include <string.h>
#include <Wire.h>
#include "SI114X.h"

SI114X SI1145 = SI114X();

// Hardware used:
// Grove LoraE5 module connected to Tiny Machine Learning Kit (https://store.arduino.cc/products/arduino-tiny-machine-learning-kit)
//
// grove pin on lora e5 module (https://wiki.seeedstudio.com/Grove_LoRa_E5_New_Version/)
// pin           Function
// pin1 Yellow   RX  Serial Receive
// pin2 White    TX  Serial Transmit
// pin3 Red      VCC Power for Grove Module, 5V/3.3V
// pin4 Black    GND Ground

// ----------------------------------------------------------------------------------
// Functions to control the RGB Led, Power Led and Builtin Led of the Nano 33 BLE boards.

// https://arduino.stackexchange.com/questions/80323/how-do-i-control-the-onboard-leds-of-my-arduino-nano-33-ble-sense

// digital output leds nano 33
#define DO_LBLTIN		(13u)
#define DO_LRED 		(22u)
#define DO_LGREEN 		(23u)
#define DO_LBLUE 		(24u)
#define DO_LPOW 		(25u)

// rgb led color
#define RED 		1
#define BLUE 		2
#define GREEN 		3
// other colors are equivalent off

// The power LED and the built-in LED are active-high, i.e. you need to set their pin to HIGH to turn these LEDs on.
// The three LEDs in the RGB LED, however, are active-low.
// see schematics
// https://content.arduino.cc/assets/NANO33BLE_V2.0_sch.pdf
// loraE5 state values
enum id_led_nano33 {
	LED_BLTIN,				// led builtin
	LED_POW,				// led power
	LED_RGB					// led RED, BLUE, GREEN
};

void initLeds(void)
{
	// initialize the digital Pin as an output
	pinMode(DO_LRED, OUTPUT);
	pinMode(DO_LBLUE, OUTPUT);
	pinMode(DO_LGREEN, OUTPUT);
	pinMode(DO_LPOW, OUTPUT);
	pinMode(DO_LBLTIN, OUTPUT);
}

void setLed(id_led_nano33 idLed, int state)
{
	switch(idLed)
	{
	case LED_BLTIN		:
		if(state==HIGH)
			digitalWrite(DO_LBLTIN, HIGH);
		else
			digitalWrite(DO_LBLTIN, LOW);
		break;
	case LED_POW        :
		if(state==HIGH)
			digitalWrite(DO_LPOW, HIGH);
		else
			digitalWrite(DO_LPOW, LOW);
		break;
	case LED_RGB		:					// led RED, BLUE, GREEN
		switch(state)
		{
		case RED 		:
			digitalWrite(DO_LRED, LOW); 	// turn the LED RED ON by making the voltage LOW
			digitalWrite(DO_LGREEN, HIGH); 	// turn the LED GREEN off by making the voltage HIGH
			digitalWrite(DO_LBLUE, HIGH); 	// turn the LED BLUE off by making the voltage HIGH
			break;
		case BLUE 		:
			digitalWrite(DO_LRED, HIGH); 	// turn the LED RED off by making the voltage HIGH
			digitalWrite(DO_LGREEN, HIGH); 	// turn the LED GREEN off by making the voltage HIGH
			digitalWrite(DO_LBLUE, LOW); 	// turn the LED BLUE ON by making the voltage LOW
			break;
		case GREEN 		:
			digitalWrite(DO_LRED, HIGH); 	// turn the LED RED off by making the voltage HIGH
			digitalWrite(DO_LGREEN, LOW); 	// turn the LED GREEN ON by making the voltage LOW
			digitalWrite(DO_LBLUE, HIGH); 	// turn the LED BLUE off by making the voltage HIGH
			break;
		default				:
			digitalWrite(DO_LRED, HIGH); 	// turn the LED RED off by making the voltage HIGH
			digitalWrite(DO_LGREEN, HIGH); 	// turn the LED GREEN off by making the voltage HIGH
			digitalWrite(DO_LBLUE, HIGH); 	// turn the LED BLUE off by making the voltage HIGH
		}
	}
}

// ----------------------------------------------------------------------------------
//
// loraE5 state values
enum state_loraE5 {
    INITIALIZE_LORAE5,
	JOIN_LORAWAN_NETWORK,
	READ_SENSOR,
    SEND_LORAWAN_MSG,
	WAIT_BEFORE_NEW_SEND
};
state_loraE5 state;


// Initialize LoRa module
UART LoraSerial(A7, A6); //A7 A6 using TinyMl Kit

char AppKEY[33] = "0960B8B2A99CA43B2CD981E8948028D8";	// 128 bit Application Key
static char msg[256];									// generic buffer for msg
static char tx_buff[256];								// tx buffer
static char rx_buff[512];								// rx buffer

static int rssi = 0;
static int snr = 0;

static int DelayMsgSend = 30;							// delay between send message in seconds
static int msec_start_txmsg;							// time value in msec start send msg

// sensor values
int VisibleLux;											// Light value
int ir;
int uv;

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

static bool RxDownlinkMsg(char *p_msg)
{
	bool fRxMsgDown = false;				// true if received TTN downlink message
    if (p_msg == NULL)
    {
        return fRxMsgDown;
    }
	char *p_start = NULL;
    int data = 0;

    p_start = strstr(p_msg, "RX");
    if (p_start && (sscanf(p_start, "RX: \"%255[^\"]", msg) == 1))
    {
        // remove last '\"' from the extracted string
        size_t len = strlen(msg);
        if (len > 0 && msg[len - 1] == '"') {
            msg[len - 1] = '\0';
        }
        Serial.print("Downlink msg: [");
        Serial.print(msg);
        Serial.println("]");
		fRxMsgDown = true;				// received TTN downlink message
    }

    p_start = strstr(p_msg, "RSSI");
    if (p_start && (1 == sscanf(p_start, "RSSI %d,", &rssi)))
    {
        Serial.print("rssi:");
        Serial.println(rssi);
    }
    p_start = strstr(p_msg, "SNR");
    if (p_start && (1 == sscanf(p_start, "SNR %d", &snr)))
    {
        Serial.print("snr :");
        Serial.println(snr);
    }
    return fRxMsgDown;
}

void setup() {
    Wire.begin();

    Serial.begin(115200);
	initLeds();									// initialize the LED digital Pins as an output
    
    LoraSerial.begin(9600);				// initialize LoraE5 serial communication

    Serial.println("Beginning Si1145!");

    while (!SI1145.Begin()) {
        Serial.println("Si1145 is not ready!");
        delay(1000);
    }
    Serial.println("Si1145 is ready!");
    Serial.println("Send Sunlight Sensor values to TTN application\r\n");
	setLed(LED_BLTIN, LOW);						// led builtin OFF
	setLed(LED_RGB, 0);							// set the RGB color OFF
	state = INITIALIZE_LORAE5;					// initialize state
}

void loop() {
	switch(state)
	{
	case INITIALIZE_LORAE5:
		if (cmdLoraE5("AT", "+AT: OK", 100))
		{
			Serial.println("LoRa E5 configuration");
			cmdLoraE5("AT+ID", "+ID", 1000);
			cmdLoraE5("AT+MODE=LWOTAA", "+MODE: LWOTAA", 1000);
			cmdLoraE5("AT+DR=EU868", "+DR: EU868", 1000);
			cmdLoraE5("AT+DR=1", "+DR:", 1000);
			cmdLoraE5("AT+CH=NUM,0-2", "+CH: NUM", 1000);
			snprintf(msg, sizeof(msg), "AT+KEY=APPKEY,\"%s\"", AppKEY);
			cmdLoraE5(msg, "+KEY: APPKEY", 1000);
			cmdLoraE5("AT+CLASS=A", "+CLASS: C", 1000);
			cmdLoraE5("AT+PORT=8", "+PORT: 8", 1000);
			state = JOIN_LORAWAN_NETWORK;
		}
		else
		{
			Serial.println("Communication error with Lora E5 module...");
            delay(5000);
			// remain in this state
		}
		break;
	case JOIN_LORAWAN_NETWORK:
		if(cmdLoraE5("AT+JOIN", "+JOIN: Network joined", 12000))
		{
			state = READ_SENSOR;		// after joining, start to acquire data
		}
		else
		{
			// network not joined
			Serial.println("JOIN failed!\r\n");
			// reread DevAddr, DevEui, AppEui
			cmdLoraE5("AT+ID", "+ID: AppEui", 1000);
			delay(5000);
			state = INITIALIZE_LORAE5;
		}
		break;
	case READ_SENSOR:					// read pressure and temperature
		// read all the sensor values
		Serial.print("The Light value is: ");
		VisibleLux = SI1145.ReadVisible();
		ir = SI1145.ReadIR();
		//the real UV value must be div 100 from the reg value , datasheet for more information.
		uv = int((float)SI1145.ReadUV() / 100.0);

		Serial.print("Vis: "); Serial.println(VisibleLux);
		Serial.print("IR: ");  Serial.println(ir);
		Serial.print("UV: ");  Serial.println(uv);
		Serial.println(VisibleLux);

		// print an empty line
		Serial.println();

		state = SEND_LORAWAN_MSG;		// after sensor acquire, start to send msg
		break;
	case SEND_LORAWAN_MSG:	
		// Send msg
		msec_start_txmsg = millis();			// get time start send msg
		setLed(LED_BLTIN, HIGH);				// send message to ttn: set Led builtin ON
		// prepare msg with sensor data
		
		snprintf(msg, sizeof(msg), "AT+CMSGHEX=\"%04X%04X%04X\"", VisibleLux, ir, uv);
		if(cmdLoraE5(msg, "Done", 5000))
		{
			RxDownlinkMsg  (rx_buff);
		}
		else
		{
			Serial.println("Send lorawan msg failed!\r\n");
		}
		state = WAIT_BEFORE_NEW_SEND;
		break;
	case WAIT_BEFORE_NEW_SEND:
		// wait for DelayMsgSend seconds
		int deltaMsec, waitMsec;

		Serial.print("Wait for ");
		Serial.print(DelayMsgSend);
		Serial.print(" seconds ...");
		setLed(LED_BLTIN, LOW);					// led builtin OFF

		// adjust time delay
		deltaMsec = millis() - msec_start_txmsg;
		waitMsec = (DelayMsgSend * 1000) - deltaMsec;
		delay(waitMsec);
		// acquire new sensor values
		state = READ_SENSOR;
		break;
	default:
		state = INITIALIZE_LORAE5;
    }
    digitalWrite(LED_BUILTIN, LOW);
}


