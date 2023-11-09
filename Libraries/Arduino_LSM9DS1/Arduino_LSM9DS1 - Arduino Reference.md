# Arduino_LSM9DS1

Allows you to read the accelerometer, magnetometer and gyroscope values from the LSM9DS1 IMU on your Arduino Nano 33 BLE Sense.  

This library is compatible with **all** architectures so you should be able to use it on all the Arduino boards.

To use this library, open the [Library Manager](https://www.arduino.cc/en/Guide/Libraries) in the Arduino IDE and install it from there.

- [1.1.1](https://downloads.arduino.cc/libraries/github.com/arduino-libraries/Arduino_LSM9DS1-1.1.1.zip) (latest)
- [1.1.0](https://downloads.arduino.cc/libraries/github.com/arduino-libraries/Arduino_LSM9DS1-1.1.0.zip)
- [1.0.0](https://downloads.arduino.cc/libraries/github.com/arduino-libraries/Arduino_LSM9DS1-1.0.0.zip)

### Usage

The ArduinoLSM9DS1 library allows you to use the inertial measurement unit (IMU) available on the Arduino® Nano 33 BLE board. The IMU is a [LSM9DS1](https://www.st.com/resource/en/datasheet/lsm9ds1.pdf), it is a 3-axis accelerometer, 3-axis gyroscope, and 3-axis magnetometer. 

The IMU is connected to the Nano 33 BLE board’s microcontroller through I2C. The values returned are signed floats.

To use this library:

```c
#include <Arduino_LSM9DS1.h>
```




[Source](https://www.arduino.cc/reference/en/libraries/arduino_lsm9ds1/)