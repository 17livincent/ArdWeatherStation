/**
 * @file common.h
 * @author Vincent Li (li.vincent0@gmail.com)
 * @brief Common structs and functions for servers and clients.
 */

#ifndef COMMON_
#define COMMON_

#include <string>

// Port of server facing client_sensor
#define PORT_SERVER_SENSOR 3006

// Port of server facing client_display
#define PORT_SERVER_DISPLAY 3007

// Struct to send and receive sensor readings
typedef struct sensor_readings_t {
    float temp;
    float rel_hum;
} sensor_readings_t;

const char SENSOR_READINGS_T_LEN = sizeof(sensor_readings_t);

// String to display on 16x2 LCD
// client_display will handle wraparound
std::string lcd_display_text_default = "TEMP:    XX.XX FHUM:     XX.XX %";
char temp_val_start_idx = 9;
char hum_val_start_idx = 25;
const char LCD_DISPLAY_TEXT_LEN = lcd_display_text_default.length() + 1;

// Message from server to client_sensor to request measurements
const char MSG_MEAS_REQ = 0xB1;
const char MSG_MEAS_REQ_LEN = sizeof(MSG_MEAS_REQ);

const char MSG_ACK_BYTE = 0xA2;
const char MSG_ACK_LEN = sizeof(char);

#endif  // COMMON_
