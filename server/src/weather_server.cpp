/**
 * @file weather_server.cpp
 * @author Vincent Li (li.vincent0@gmail.com)
 * @brief Defines the executable for the server module.
 */

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <cstring>

#include "socket_server.h"
#include "socket_common.h"
#include "socket_messages.h"

#include "weather_server.h"

// Saved temperature and humidity values
sensor_readings_t latest_readings;

void server_sensor_handler(const SocketServer* server, const uint8_t instance_id, const int socket) {
    char* instance_recv_buffer = &server->instance_recv_buffers[instance_id * server->recv_buffer_max_len];
    char* instance_send_buffer = &server->instance_send_buffers[instance_id * server->recv_buffer_max_len];

    std::cout << "INSTANCE #" << (int)instance_id << " CONNECTED WITH SENSOR CLIENT " << socket << std::endl;

    while(server->active && server->instance_running[instance_id]) {
        server->instance_recv_buffer_len[instance_id] = ::read(socket, (void*)instance_recv_buffer, server->recv_buffer_max_len);

        std::cout << "INSTANCE #" << (int)instance_id << " RECEIVED: " << server->instance_recv_buffer_len[instance_id] << " ";
        print_buffer(instance_recv_buffer, server->instance_recv_buffer_len[instance_id]);
        std::cout << std::endl;

        if(server->instance_recv_buffer_len[instance_id] > 0) {
            // Process request...
            //
            switch((uint8_t)instance_recv_buffer[0]) {
                case ((uint8_t)MSG_STOP):
                    // Recieved STOP message
                    std::cout << "INSTANCE #" << (int)instance_id << " RECEIVED STOP" << std::endl;
                    server->instance_running[instance_id] = false;
                    break;
                // HANDLE MESSAGES HERE
                default:
                    break;
            }

            // Send back
            server->instance_send_buffer_len[instance_id] = server->instance_recv_buffer_len[instance_id];
            memcpy((void*)instance_send_buffer, (void*)instance_recv_buffer, server->instance_recv_buffer_len[instance_id]);

            send(socket, (void*)instance_send_buffer, server->instance_send_buffer_len[instance_id], 0);
                    
            std::cout << "INSTANCE #" << (int)instance_id <<" ECHOED" << std::endl;
            //
        }
        else {
            std::cout << "INSTANCE #" << (int)instance_id << " CONNECTION CLOSED" << std::endl;
            server->instance_running[instance_id] = false;
        }
    }
}

void server_display_handler(const SocketServer* server, const uint8_t instance_id, const int socket) {
    char* instance_recv_buffer = &server->instance_recv_buffers[instance_id * server->recv_buffer_max_len];
    char* instance_send_buffer = &server->instance_send_buffers[instance_id * server->recv_buffer_max_len];

    std::cout << "INSTANCE #" << (int)instance_id << " CONNECTED WITH DISPLAY CLIENT " << socket << std::endl;

    while(server->active && server->instance_running[instance_id]) {

        server->instance_recv_buffer_len[instance_id] = ::read(socket, (void*)instance_recv_buffer, server->recv_buffer_max_len);

        std::cout << "INSTANCE #" << (int)instance_id << " RECEIVED: " << server->instance_recv_buffer_len[instance_id] << " ";
        print_buffer(instance_recv_buffer, server->instance_recv_buffer_len[instance_id]);
        std::cout << std::endl;

        if(server->instance_recv_buffer_len[instance_id] > 0) {
            // Process request...
            //
            switch((uint8_t)instance_recv_buffer[0]) {
                case ((uint8_t)MSG_STOP):
                    // Recieved STOP message
                    std::cout << "INSTANCE #" << (int)instance_id << " RECEIVED STOP" << std::endl;
                    server->instance_running[instance_id] = false;
                    break;
                // HANDLE MESSAGES HERE
                default:
                    break;
            }

            // Send back
            server->instance_send_buffer_len[instance_id] = server->instance_recv_buffer_len[instance_id];
            memcpy((void*)instance_send_buffer, (void*)instance_recv_buffer, server->instance_recv_buffer_len[instance_id]);

            send(socket, (void*)instance_send_buffer, server->instance_send_buffer_len[instance_id], 0);
                    
            std::cout << "INSTANCE #" << (int)instance_id <<" ECHOED" << std::endl;
            //
        }
        else {
            std::cout << "INSTANCE #" << (int)instance_id << " CONNECTION CLOSED" << std::endl;
            server->instance_running[instance_id] = false;
        }
    }
}

int main(int argc, char** argv) {
    std::cout << "STARTING WEATHER SERVER" << std::endl;

    // Server buffers
    char client_display_recv_buffer[32];
    char client_display_send_buffer[32];
    char client_sensor_recv_buffer[32];
    char client_sensor_send_buffer[32];

    memset((void*)client_display_recv_buffer, 0, 32);
    memset((void*)client_display_send_buffer, 0, 32);
    memset((void*)client_sensor_recv_buffer, 0, 32);
    memset((void*)client_sensor_send_buffer, 0, 32);

    // Create server to communicate with the display
    // Max 1 connection, port 80
    SocketServer server_display = SocketServer(1, (char*)client_display_recv_buffer, 32, (char*)client_display_send_buffer, 32, 80, NULL);

    // Create server to communicate with the sensor
    // Max 1 connection, port 3007
    SocketServer server_sensor = SocketServer(1, (char*)client_sensor_recv_buffer, 32, (char*)client_sensor_send_buffer, 32, 3006, NULL);

    std::cout << "STOPPING WEATHER SERVER" << std::endl;
    return 0;
}