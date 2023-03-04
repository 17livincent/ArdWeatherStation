/**
 * @file client_sensor.ino
 * @author Vincent Li (li.vincent0@gmail.com)
 * @brief Code for the sensor client.
 */

#include "Adafruit_SHTC3.h"
#include <ESP8266WiFi.h>

#include "common.h"
#include "network_creds.h"

// WiFi client object
WiFiClient client;

const char MSG_ACK = 0xA2;

// SHTC3 sensor
Adafruit_SHTC3 shtc3 = Adafruit_SHTC3();

// Sensor values
sensors_event_t humidity;
sensors_event_t temp;
sensor_readings_t latest_readings;

// Use millis() to trigger measure
unsigned long millis_prev = 0;
unsigned long millis_curr = 0;
#define MEASURE_INTERVAL 5000   // 1000ms interval

#define PIN_LED 0

// Send and recv buffers
#define RECV_BUFFER_MAX_LEN 32
#define SEND_BUFFER_MAX_LEN 32
uint16_t recv_buffer_len = 0;
uint16_t send_buffer_len = 0;
char recv_buffer[RECV_BUFFER_MAX_LEN];
char send_buffer[SEND_BUFFER_MAX_LEN];

void setup() {
    Serial.begin(9600);

    // Set up LED
    pinMode(PIN_LED, OUTPUT);

    Serial.println("Bring-up SHTC3");
    if(!shtc3.begin()) {
        Serial.println("SHTC3: Failed to start");
    }
    else {
        Serial.println("SHTC3: Started");
    }

    // Clear buffers
    for(int i = 0; i < RECV_BUFFER_MAX_LEN; i++) {
        recv_buffer[i] = 0;
    }
    for(int i = 0; i < SEND_BUFFER_MAX_LEN; i++) {
        send_buffer[i] = 0;
    }
}

void loop() {
    if(WiFi.status() != WL_CONNECTED) {
        // If not connected to wifi, make a connection attempt
        delay(5000);

        // Connect to wifi
        WiFi.begin(NETWORK_NAME, NETWORK_PSWD);
        while(WiFi.status() != WL_CONNECTED) {
            delay(1000);
            Serial.println(".");
        }
        Serial.println("");
        Serial.print("WiFi connected: IP ");
        Serial.println(WiFi.localIP());
    }
    else {
        if(!client.connected()) {
            // If not connected to the server, make a connection attempt
            delay(5000);

            // Connect to server
            Serial.println("Connecting to server");
            while(!client.connect(SERVER_IP, PORT_SERVER_SENSOR)) {
                delay(1000);
                Serial.println(".");
            }
            Serial.println("Connected to server");

            // Wait for ACK from server
            Serial.println("Waiting for ACK from server");
            while(!client.available()) {
                delay(1000);
                Serial.println(".");
            }
            recv_buffer_len = client.read(recv_buffer, MSG_ACK_LEN);
            Serial.print("Received ACK from server ");
            Serial.println(recv_buffer_len);
        }
        else {
            // Connection established with server
            // Wait for request from server
            if(client.available()) {
                recv_buffer_len = client.read(recv_buffer, MSG_MEAS_REQ_LEN);
                // Check if received MSG_MEAS_REQ
                if(recv_buffer[0] == MSG_MEAS_REQ) {
                    // Take a measurement

                    // Turn LED on
                    digitalWrite(PIN_LED, 1);

                    // Take a measurement      
                    shtc3.getEvent(&humidity, &temp);
                    Serial.print("Temp: ");
                    Serial.print(temp.temperature * 1.8 + 32);
                    Serial.println(" degF");
                    Serial.print("Humidity: ");
                    Serial.print(humidity.relative_humidity);
                    Serial.println(" % rH");

                    // Set latest_readings
                    latest_readings.temp = temp.temperature;
                    latest_readings.rel_hum = humidity.relative_humidity;

                    // Send measurement to client
                    send_buffer_len = sizeof(latest_readings);
                    memcpy(send_buffer, (void*)&latest_readings, send_buffer_len);
                    client.write(send_buffer, send_buffer_len);
                    send_buffer_len = 0;

                    // Turn LED off
                    digitalWrite(PIN_LED, 0);
                }
            }
        }
    }
}