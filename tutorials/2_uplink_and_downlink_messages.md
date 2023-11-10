# Uplink and Downlink messages

[TOC]

## Introduction

In LoRaWAN networks, Class A end nodes are low-power IoT devices that can send uplink messages and receive Downlink messages.

To optimize power consumption and enable long-range communication in Class A  devices, Uplink and downlink messages follow a specific pattern:

**Uplink Messages for Class A End Nodes:**

- the End Nodes send asynchronously the Uplink messages
- after the transmission, the end node listens for potential downlink messages during short, specific time windows defined by the network.

**Downlink Messages for Class A End Nodes:**

- The downlink messages may contain commands (such as device configurations or control) or data sent from the server or network to the end node.
- It is not possible to autonomously send downlink messages to Class A end nodes except in response to a previous uplink message.

This tutorial explains how an Arduino sketch works for sending uplink messages to a TTN3 application and receiving commands from the TTN3 application to the node through downlink messages.

## Load the Arduino sketch to the lorawan node

![img](./img/uplink_and_downlink_messages/lu137324la6e_tmp_21734edc32ac424b.png)

![img](./img/uplink_and_downlink_messages/lu137324la6e_tmp_624b29b1acda9225.png)

Open sketch: ttn3e5_uplink_count_dwnlink_LEDcolor.ino

Select board:

![img](./img/uplink_and_downlink_messages/lu137324la6e_tmp_9ccfbecd6d40fc72.png)

This is on windows OS,

for MAC and Linux it is slightly different

Upload

![img](./img/uplink_and_downlink_messages/lu137324la6e_tmp_527a42c2af7c0b6b.png)

compile ...

![img](./img/uplink_and_downlink_messages/lu137324la6e_tmp_cd133653c535c5a0.png)

![img](./img/uplink_and_downlink_messages/lu137324la6e_tmp_9c771d297f9295ad.png)

Select Serial monitor

![img](./img/uplink_and_downlink_messages/lu137324la6e_tmp_5bf8ae88eac06ec8.png)

The program runs, executing these steps:

1. initialization of lorawan parameters
2. loop:
   - send to the TTN3 application an uplink message composed of 5  bytes:
      - bytes 0..3: 32 bit counter incremented each loop
      - byte 4: color code for RGB LED of Nano 33 BLE
         - 0x00: RGB LED OFF
         - 0x01: RGB LED RED
         - 0x02: RGB LED BLUE
         - 0x33: RGB LED GREEN
1. check if there is a downlink message from the TTN application. In this case, set the RGB LED to the color set in the downlink message
2. increment the 32 bit counter
3. Wait 30 seconds
4. repeat loop

![img](./img/uplink_and_downlink_messages/lu137324la6e_tmp_9839a2edff073ede.png)

Open the things network site

https://www.thethingsnetwork.org/

![img](./img/uplink_and_downlink_messages/lu137324la6e_tmp_f453e5f670c7a718.png)

Select Login

![img](./img/uplink_and_downlink_messages/lu137324la6e_tmp_1eacbe81d5ae7e75.png)

Go down and select Login to The Things Network

![img](./img/uplink_and_downlink_messages/lu137324la6e_tmp_270ddf1c0c1a790.png)

After Login, select Console

![img](./img/uplink_and_downlink_messages/lu137324la6e_tmp_e7f1656913c5d947.png)

select “Go to applications”

https://eu1.cloud.thethings.network/console/

![img](./img/uplink_and_downlink_messages/lu137324la6e_tmp_3fa0d8058928a5ec.png)

https://eu1.cloud.thethings.network/console/applications

In the applications page, select the application where the end node is registered

(in this case, tst-nano33)

![img](./img/uplink_and_downlink_messages/lu137324la6e_tmp_e10a4ac74c12dc91.png)

![img](./img/uplink_and_downlink_messages/lu137324la6e_tmp_e1114635f3c0e9b4.png)

On the left menu, select ‘Live data’

https://eu1.cloud.thethings.network/console/applications/tst-nano33/data

this page is updated with the uplink messages sent by the end devices registered in the application

![img](./img/uplink_and_downlink_messages/lu137324la6e_tmp_4ef5f719a08dccb5.png)

Click on a row of data message to see the details

![img](./img/uplink_and_downlink_messages/lu137324la6e_tmp_2b39a94e671832d9.png)

## Uplink payload formatter

On the left menu, select Payload formatters – Uplink

![img](./img/uplink_and_downlink_messages/lu137324la6e_tmp_35c38e0f333be4fd.png)

https://eu1.cloud.thethings.network/console/applications/tst-nano33/payload-formatters/uplink

As ‘Formatter type’ select ‘Custom Javascript formatter’

![img](./img/uplink_and_downlink_messages/lu137324la6e_tmp_d8576827c9ca4f71.png)

Insert the downlink formatter to decode the uplink messages sent by the arduino sketch ttn3e5_uplink_count_dwnlink_LEDcolor.ino

![img](./img/uplink_and_downlink_messages/lu137324la6e_tmp_10a8632c6c43f373.png)

### Javascript uplink decoder

```javascript
// uplink decoder ttn v3 for ttn3e5_uplink_count_dwnlink_LEDcolor.ino

var pos = 0;
var bindata = "";

var ConvertBase = function (num) {
    return {
        from : function (baseFrom) {
            return {
                to : function (baseTo) {
                    return parseInt(num, baseFrom).toString(baseTo);
                }
            };
        }
    };
};

function pad(num) {
    var s = "0000000" + num;
    return s.slice(-8);
}

ConvertBase.dec2bin = function (num) {
    return pad(ConvertBase(num).from(10).to(2));
};

ConvertBase.bin2dec = function (num) {
    return ConvertBase(num).from(2).to(10);
};

function data2bits(data) {
    var binary = "";
    for(var i=0; i<data.length; i++) {
        binary += ConvertBase.dec2bin(data[i]);
    }
    return binary;
}

function bitShift(bits) {
    var num = ConvertBase.bin2dec(bindata.substr(pos, bits));
    pos += bits;
    return Number(num);
}

function precisionRound(number, precision) {
  var factor = Math.pow(10, precision);
  return Math.round(number * factor) / factor;
}

function decodeUplink(input) {
	// https://www.thethingsindustries.com/docs/integrations/payload-formatters/javascript/
	var data = {};

	bindata = data2bits(input.bytes);

	counter = bitShift(32);
	rgb_color = bitShift(8);
	rgb_color = rgb_color & 0x03;
	switch(rgb_color) {
		case 0	:							// RGB LED OFF
			RGBcolor = "OFF";
			break;
		case 1	:							// RGB LED RED
			RGBcolor = "RED";
			break;
		case 2	:							// RGB LED BLUE
			RGBcolor = "BLUE";
			break;
		case 3	:							// RGB LED GREEN
			RGBcolor = "GREEN";
			break;
	}
	data.decoded = {
		"counter": counter,
		"RGBcolor": RGBcolor
	};

	return {
		data: data,
		warnings: [],
		errors: []
	};
}
```

Inserted the javascript uplink code, press Save changes

![img](./img/uplink_and_downlink_messages/lu137324la6e_tmp_17978aebd0709f4a.png)

![img](./img/uplink_and_downlink_messages/lu137324la6e_tmp_ad4956dce7b5e004.png)

Reopen the Live data page

![img](./img/uplink_and_downlink_messages/lu137324la6e_tmp_4a3f770337e7e6d.png)

Now the uplink decoder function is active.

In every uplink message row a decoded field is shown

![img](./img/uplink_and_downlink_messages/lu137324la6e_tmp_e554b7cdcd1a10d4.png)

Click on an uplink message. In ‘Event details’ there is ‘decoded_payload’ json field name with “decoded” that stores two elements:

1. RGBcolor: actual color of Nano 33 LED RGB (in this case: the LED is actually OFF)
2. counter: the actual value of  the counter sent by Nano 33 (in this case: 190)

![img](./img/uplink_and_downlink_messages/lu137324la6e_tmp_a86dd9b919296622.png)

## Send a downlink message to change RGB LED color

From the left menu select: End devices

![img](./img/uplink_and_downlink_messages/lu137324la6e_tmp_1d3b8b83a48de19a.png)

https://eu1.cloud.thethings.network/console/applications/tst-nano33/devices

Select the end device registered in the TTN applications

![img](./img/uplink_and_downlink_messages/lu137324la6e_tmp_9fb82f31460f936b.png)

https://eu1.cloud.thethings.network/console/applications/tst-nano33/devices/eui-2cf7f12042004b8a

This will open the General Information tab.

![img](./img/uplink_and_downlink_messages/lu137324la6e_tmp_728585b25e381ede.png)

Press the Messaging tab

![img](./img/uplink_and_downlink_messages/lu137324la6e_tmp_78cb53a106fdf303.png)

Select Downlink tab

![img](./img/uplink_and_downlink_messages/lu137324la6e_tmp_c181b586f1939320.png)

On Payload type, select ‘Bytes’

Insert the byte code to modify the RGB LED color

| RGB LED                                                      | code | color       |
| ------------------------------------------------------------ | ---- | ----------- |
| ![img](./img/uplink_and_downlink_messages/lu137324la6e_tmp_36c5638689952eef.png) | 0x00 | RGB LED OFF |
| ![img](./img/uplink_and_downlink_messages/lu137324la6e_tmp_683e585a4b46988e.png) | 0x01 | RED color   |
| ![img](./img/uplink_and_downlink_messages/lu137324la6e_tmp_18aa90609e42303e.png) | 0x02 | BLUE        |
| ![img](./img/uplink_and_downlink_messages/lu137324la6e_tmp_f718cafdcb157eee.png) | 0x03 | GREEN       |

![img](./img/uplink_and_downlink_messages/lu137324la6e_tmp_1cf0a60b100449b5.png)

Press Schedule downlink

![img](./img/uplink_and_downlink_messages/lu137324la6e_tmp_d03e8aae53ff8fe2.png)

After receiving  a new uplink message, the TTN application sends automatically a downlink message to the end node.

The uplink message includes the code that change the color on the RGB LED.

![img](./img/uplink_and_downlink_messages/lu137324la6e_tmp_a3608f6fd0396d97.png)

```
AT+CMSGHEX="0000013A01"
+CMSGHEX: ACK Received
+CMSGHEX: PORT: 1; RX: "01"
+CMSGHEX: RXWIN1, RSSI -87, SNR 13.0
+CMSGHEX: Done
+CMSGHEX: Start
+CMSGHEX: Wait ACK
Downlink msg: [01]
rssi:-87
snr :13
RGB LED color: RED
Wait for 30 seconds …
```

## How to create a downlink decoder to send downlink messages in json format

In the  left menu, select Payload formatters – Downlink

![img](./img/uplink_and_downlink_messages/lu137324la6e_tmp_b4ad85b16f0139e9.png)

As Formatter, select “Custom javascript formatter”

![img](./img/uplink_and_downlink_messages/lu137324la6e_tmp_33c8f52c2137c36.png)

### Javascript downlink formatter

```javascript
// uplink decoder ttn v3 for ttn3e5_uplink_count_dwnlink_LEDcolor.ino
//
// based on LED color example of:
// https://www.thethingsindustries.com/docs/integrations/payload-formatters/javascript/downlink/
//
var colors = ["OFF", "RED", "BLUE", "GREEN"];

function encodeDownlink(input) {
  return {
    bytes: [colors.indexOf(input.data.color)],
    fPort: 1,
  };
}

function decodeDownlink(input) {
  switch (input.fPort) {
  case 1:
    return {
      data: {
        color: colors[input.bytes[0]]
      }
    }
  default:
    return {
      errors: ["unknown FPort"]
    }
  }
}
```

Press Save changes

![img](./img/uplink_and_downlink_messages/lu137324la6e_tmp_77ce065b3b78dfce.png)

![img](./img/uplink_and_downlink_messages/lu137324la6e_tmp_c43f675cb021a60a.png)

## Send a downlink message to change RGB LED color using a json payload

From the left menu select: End devices

![img](./img/uplink_and_downlink_messages/lu137324la6e_tmp_1d3b8b83a48de19a.png)

https://eu1.cloud.thethings.network/console/applications/tst-nano33/devices

Select the end device registered in the TTN applications

![img](./img/uplink_and_downlink_messages/lu137324la6e_tmp_9fb82f31460f936b.png)

https://eu1.cloud.thethings.network/console/applications/tst-nano33/devices/eui-2cf7f12042004b8a

Will open the General Information tab.

![img](./img/uplink_and_downlink_messages/lu137324la6e_tmp_728585b25e381ede.png)

Press Messaging tab

![img](./img/uplink_and_downlink_messages/lu137324la6e_tmp_78cb53a106fdf303.png)

Select Downlink tab

![img](./img/uplink_and_downlink_messages/lu137324la6e_tmp_c181b586f1939320.png)

On Payload type, select ‘JSON’

in Payload insert:

```
{
  "color": "BLUE"
}
```

![img](./img/uplink_and_downlink_messages/lu137324la6e_tmp_e85d02b6f36539ed.png)

Press Schedule downlink

![img](./img/uplink_and_downlink_messages/lu137324la6e_tmp_7cd8003c8ae4df8e.png)

![img](./img/uplink_and_downlink_messages/lu137324la6e_tmp_43ef1ef9de6f147.png)

After  receiving a new uplink message, the TTN application sends automatically a downlink message to the end node.

The uplink message includes the code that sets the BLUE color on the RGB LED.

![img](./img/uplink_and_downlink_messages/lu137324la6e_tmp_18aa90609e42303e.png)