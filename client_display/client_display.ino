/**
 * @file client_display.ino
 * @author Vincent Li (li.vincent0@gmail.com)
 * @brief Code for the display client.
 */

#include <LiquidCrystal.h>
#include <ESP8266WiFi.h>

#include "common.h"
#include "network_creds.h"

// WiFi client object
WiFiClient client;

const char MSG_ACK = 0xA2;

// LCD pins
#define PIN_RS 12
#define PIN_EN 13
#define PIN_D4 15
#define PIN_D5 0
#define PIN_D6 16
#define PIN_D7 2
LiquidCrystal lcd(PIN_RS, PIN_EN, PIN_D4, PIN_D5, PIN_D6, PIN_D7);

// Send and recv buffers
#define RECV_BUFFER_MAX_LEN 64
#define SEND_BUFFER_MAX_LEN 64
uint16_t recv_buffer_len = 0;
uint16_t send_buffer_len = 0;
char recv_buffer[RECV_BUFFER_MAX_LEN];
char send_buffer[SEND_BUFFER_MAX_LEN];

// recv_buffer in a string
String recv_str;

void setup() {
    Serial.begin(9600);

    lcd.begin(16, 2);
    lcd.setCursor(0, 0);
    lcd.print("Initializing...");

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
        Serial.println("Connecting to WiFi");
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Connecting to   ");
        lcd.setCursor(0, 1);
        lcd.print("WiFi...");
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
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Connecting to   ");
            lcd.setCursor(0, 1);
            lcd.print("server...");
            while(!client.connect(SERVER_IP, PORT_SERVER_DISPLAY)) {
                delay(1000);
                Serial.println(".");
            }
            Serial.println("Connected to server");
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Connected to    ");
            lcd.setCursor(0, 1);
            lcd.print("server...");

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
                recv_buffer_len = client.read(recv_buffer, LCD_DISPLAY_TEXT_LEN);
                // Check if received full display text size
                if(recv_buffer_len == LCD_DISPLAY_TEXT_LEN) {
                    // Display given text

                    // Write to LCD
                    recv_str = recv_buffer;
                    Serial.print("Received: ");
                    Serial.println(recv_str);
                    lcd.clear();
                    lcd.setCursor(0, 0);
                    lcd.print(recv_str.substring(0, 16));
                    lcd.setCursor(0, 1);
                    lcd.print(recv_str.substring(16, 32));

                    // Send ACK to server
                    Serial.println("Sending ack");
                    send_buffer_len = MSG_ACK_LEN;
                    send_buffer[0] = MSG_ACK;
                    client.write(send_buffer, send_buffer_len);
                    Serial.println("Sent ack");
                }
            }
        }
    }
}
