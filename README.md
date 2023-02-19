# ArdWeatherStation

This project is a LAN-based weather station using Arduino MCU (Adafruit Feather HUZZAH with ESP8266) clients and a server running on a Linux Docker container.

The goal is to have one outdoor MCU connected to a temperature and humidity sensor (Adafruit SHTC3), and another indoor MCU with an LCD screen to display the most recent sensor readings.  A server program running on a desktop will communicate with both via WiFi.  It will request readings from the sensor MCU at a given interval.  It will also send those readings to the indoor MCU with the LCD screen in a similar interval.

                  (server)
                 /        \
                /          \
(client_sensor)              (client_display)

## Notable Modules

- client_sensor
    - Contains the Arduino sketch for the temperature and humidity sensor MCU
- client_display
    - Contains the Arduino sketch for the MCU with display
- server
    - Contains the server program and Dockerfile