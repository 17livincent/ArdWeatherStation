/**
 * @file common.h
 * @author Vincent Li (li.vincent0@gmail.com)
 * @brief Common structs and functions for servers and clients.
 */

// Struct to send and receive sensor readings
typedef struct sensor_readings_t {
    float temp;
    float rel_hum;
} sensor_readings_t;

