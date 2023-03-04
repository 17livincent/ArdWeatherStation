/**
 * @file weather_server.h
 * @author Vincent Li (li.vincent0@gmail.com)
 * @brief Defines the server module for the weather station.
 */

#ifndef WEATHER_SERVER
#define WEATHER_SERVER

#include "common.h"

#define INTERVAL_MEASUREMENT 30000000  // microseconds
#define INTERVAL_DISPLAY 30000000  // microseconds

// State machine states for the sensor read process
typedef enum sm_sensor_comm_state {
    SM_SENSOR_UNKNOWN_STATE,    // Uknown state, or connection is pending
    SM_SENSOR_WAITING_FOR_INTERVAL, // Waiting for measurement interval
    SM_SENSOR_PENDING_MES_RSP,  // Waiting for measurement from client_sensor
} sm_sensor_comm_state;

typedef enum sm_display_comm_state {
    SM_DISPLAY_UNKNOWN_STATE,   // Uknown state, or connection is pending
    SM_DISPLAY_WAITING_FOR_INTERVAL,    // Waiting for display interval
    SM_DISPLAY_PENDING_ACK, // Waiting for ACK from client_display
} sm_display_comm_state;

/**
 * @brief Handler function for the server facing the client_sensor
 * 
 * @param server pointer to the SocketServer object
 * @param instance_id of this server instance
 * @param socket of this server instance
 */
void server_sensor_handler(const SocketServer* server, const uint8_t instance_id, const int socket);

/**
 * @brief Handler function for the server facing the client_display
 * 
 * @param server pointer to the SocketServer object
 * @param instance_id of this server instance
 * @param socket of this server instance
 */
void server_display_handler(const SocketServer* server, const uint8_t instance_id, const int socket);

#endif  // WEATHER_SERVER