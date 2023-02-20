# ArdWeatherStation

This project is a LAN-based weather station using Arduino MCU (Adafruit Feather HUZZAH with ESP8266) clients and a server running on a Linux Docker container.

The goal is to have one outdoor MCU connected to a temperature and humidity sensor (Adafruit SHTC3), and another indoor MCU with an LCD screen to display the most recent sensor readings.  A server program running on a desktop will communicate with both via WiFi.  It will request readings from the sensor MCU at a given interval.  It will also send those readings to the indoor MCU with the LCD screen in a similar interval.

(client_sensor) <-- (server) --> (client_display)

## Notable Modules

- client_sensor
  - Contains the Arduino sketch for the temperature and humidity sensor MCU
- client_display
  - Contains the Arduino sketch for the MCU with display
- server
  - Contains the server program and Dockerfile

## Module Overview

### server

The server module utilizes the SocketServer class from the [SimpleSocket library](https://github.com/17livincent/SimpleSocket).  It will run two SocketServer objects.  One will interact directly with client_sensor and the other with the client_display.  TCP will be used for each connection.

The server will convert the temperature value Fahrenheit.  The values to be shown by the client_display will be sent as a string with the readable text.

### client_sensor

The client_sensor is connected to the SHTC3 temperature and relative humidity sensor.  It will take a measurement when a reading is requested by the server and send it back in a defined struct.

### client_display

The client_display is connected to an LCD display.  It will receive measurements from the server and write them to the LCD.

## Module Interaction

This section describes the steps of how readings from the client_sensor are displayed on the client_display, coordinated by the server.

### 1. server --> client_sensor

The server sends a measurement request with a byte-long message which is 0xB1.

### 2. client_sensor --> server

The temperature and relative humidity readings from the SHTC3 are of type float (32-bit).  The client_sensor will send these values to the server using the sensor_readings_t struct.

```
typedef struct sensor_readings_t {
    float temp;
    float rel_hum;
} sensor_readings_t;
```

### 3. server --> client_display

The LCD client_display will have 16 columns and 2 rows, so the server will send the displayed string in a 32-byte array.

```
char[32] display_text;
```

### 4. client_display --> server

Once the client_display receives the 32-byte array from the server, it will send byte 0xA2 to acknowledge.

## Client and Server Connection Procedures

Each module will know the IP and destination port of its neighbor.

The client modules will attempt to connect to the server on boot-up.  If a connection cannot be made or is lost, then they will try to reconnect at a short interval.

Similarly, the server will attempt to accept connections from both modules on boot-up, and try to reconnect if a connection is lost.