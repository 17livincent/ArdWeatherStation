/**
 * @file weather_server.h
 * @author Vincent Li (li.vincent0@gmail.com)
 * @brief Defines the server module for the weather station.
 */

#include "common.h"

#define INTERVAL_MEASUREMENT 1000 * 60 * 5  // 5 minutes
#define INTERVAL_DISPLAY 1000 * 60 * 5  // 5 minutes

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