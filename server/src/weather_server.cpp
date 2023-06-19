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
#include <string>
#include <sstream>
#include <iomanip>
#include <limits>
#include <thread>
#include <semaphore>

#include "socket_server.h"
#include "socket_common.h"
#include "socket_messages.h"

#include "weather_server.h"

// Saved temperature and humidity values
volatile sensor_readings_t latest_readings;

// Global flags set true when it's time to query a client
std::counting_semaphore<1> sem_time_to_measure(0);
std::counting_semaphore<1> sem_time_to_display(0);

// The current state of the sensor
sm_sensor_comm_state sensor_state = SM_SENSOR_UNKNOWN_STATE;

// The current state of the display
sm_display_comm_state display_state = SM_DISPLAY_UNKNOWN_STATE;

void server_sensor_handler(const SocketServer* server, const uint8_t instance_id, const int socket) {
    char* instance_recv_buffer = &server->instance_recv_buffers[instance_id * server->recv_buffer_max_len];
    char* instance_send_buffer = &server->instance_send_buffers[instance_id * server->recv_buffer_max_len];

    std::cout << "SENSOR HANDLER CONNECTED WITH SENSOR CLIENT " << socket << std::endl;

    // By now, server has acknowledged client
    // If client is connected, then instance_running is true

    sensor_state = SM_SENSOR_WAITING_FOR_INTERVAL;

    while(server->active && server->instance_running[instance_id]) {
        // Implement sm_sensor_comm_state
        switch(sensor_state) {
            case SM_SENSOR_WAITING_FOR_INTERVAL: {
                std::cout << "SENSOR HANDLER: STATE SM_SENSOR_WAITING_FOR_INTERVAL" << std::endl;
                usleep(INTERVAL_MEASUREMENT);

                // Send MSG_MEAS_REQ
                server->instance_send_buffer_len[instance_id] = MSG_MEAS_REQ_LEN;
                instance_send_buffer[0] = MSG_MEAS_REQ;
                ssize_t send_size = send(socket, (void*)instance_send_buffer, server->instance_send_buffer_len[instance_id], 0);

                // Check send status
                if(send_size == server->instance_send_buffer_len[instance_id]) {
                    // Send was successful
                    sensor_state = SM_SENSOR_PENDING_MES_RSP;
                }
                else {
                    // Send was not successful
                    // Lost connection
                    std::cout << "SENSOR HANDLER: CONNECTION CLOSED" << std::endl;
                    server->instance_running[instance_id] = false;
                    sensor_state = SM_SENSOR_UNKNOWN_STATE;
                }

                // Clear flag
                sem_time_to_measure.release();

                break;
            }

            case SM_SENSOR_PENDING_MES_RSP:
                std::cout << "SENSOR HANDLER: STATE SM_SENSOR_PENDING_MES_RSP" << std::endl;
                // Wait for and read sensor reading from socket
                server->instance_recv_buffer_len[instance_id] = ::read(socket, (void*)instance_recv_buffer, server->recv_buffer_max_len);// @TODO need timeout

                // Check read status
                if(server->instance_recv_buffer_len[instance_id] == SENSOR_READINGS_T_LEN) {
                    // Receive was successful

                    // Update latest_readings
                    latest_readings.temp = reinterpret_cast<sensor_readings_t*>(instance_recv_buffer)->temp * 1.8 + 32;
                    latest_readings.rel_hum = reinterpret_cast<sensor_readings_t*>(instance_recv_buffer)->rel_hum;

                    std::cout << "LATEST TEMP: " << latest_readings.temp << " F" << std::endl;
                    std::cout << "LATEST HUM: " << latest_readings.rel_hum << " %" << std::endl;

                    sensor_state = SM_SENSOR_WAITING_FOR_INTERVAL;
                }
                else {
                    // Lost connection

                    // Set temp and rel_hum to max float values to signify a failed read
                    latest_readings.temp = std::numeric_limits<float>::max();
                    latest_readings.rel_hum = std::numeric_limits<float>::max();

                    std::cout << "SENSOR HANDLER: CONNECTION CLOSED" << std::endl;
                    server->instance_running[instance_id] = false;
                    sensor_state = SM_SENSOR_UNKNOWN_STATE;
                }

                break;

            case SM_SENSOR_UNKNOWN_STATE:
                // This shouldn't be reachable, as SM_SENSOR_UNKNOWN_STATE means that there is no connection
                std::cout << "SENSOR HANDLER: CONNECTION CLOSED" << std::endl;
                server->instance_running[instance_id] = false;
                break;
        }
    }
}

void server_display_handler(const SocketServer* server, const uint8_t instance_id, const int socket) {
    char* instance_recv_buffer = &server->instance_recv_buffers[instance_id * server->recv_buffer_max_len];
    char* instance_send_buffer = &server->instance_send_buffers[instance_id * server->recv_buffer_max_len];

    std::cout << "DISPLAY HANDLER CONNECTED WITH DISPLAY CLIENT " << socket << std::endl;

    // By now, server has acknowledged client
    // If client is connected, then instance_running is true

    display_state = SM_DISPLAY_WAITING_FOR_INTERVAL;

    while(server->active && server->instance_running[instance_id]) {
        // Implement sm_display_comm_state
        switch(display_state) {
            case SM_DISPLAY_WAITING_FOR_INTERVAL: {
                std::cout << "DISPLAY HANDLER: STATE SM_DISPLAY_WAITING_FOR_INTERVAL" << std::endl;
                usleep(INTERVAL_DISPLAY);
                // Send LCD display text with current values
                std::string display_text = "";

                if((latest_readings.temp != std::numeric_limits<float>::max())
                    && (latest_readings.rel_hum != std::numeric_limits<float>::max())) {
                    // Fill display_text with values
                    // 4 digits each
                    std::stringstream stream;
                    stream << std::fixed << std::setprecision(2) << latest_readings.temp;
                    if(latest_readings.temp >= 100) {
                        display_text += ("TEMP:   ");
                    }
                    else {
                        display_text += ("TEMP:    ");
                    }
                    display_text += (stream.str() + " F");
                    stream.str(std::string());
                    stream << std::fixed << std::setprecision(2) << latest_readings.rel_hum;
                    display_text += ("HUM:     " + stream.str() + " %");
                }
                else {
                    // If the latest measurement was faulty, send the default text
                    display_text = lcd_display_text_default;
                }

                std::cout << "DISPLAY HANDLER: Sending \"" << display_text << "\"" << std::endl;

                server->instance_send_buffer_len[instance_id] = LCD_DISPLAY_TEXT_LEN;
                memcpy((void*)instance_send_buffer, display_text.c_str(), LCD_DISPLAY_TEXT_LEN);
                ssize_t send_size = send(socket, (void*)instance_send_buffer, server->instance_send_buffer_len[instance_id], 0);

                // Check send status
                if(send_size == server->instance_send_buffer_len[instance_id]) {
                    // Send was successful
                    display_state = SM_DISPLAY_PENDING_ACK;
                }
                else {
                    // Send was not successful
                    // Lost connection
                    std::cout << "DISPLAY HANDLER: CONNECTION CLOSED" << std::endl;
                    server->instance_running[instance_id] = false;
                    display_state = SM_DISPLAY_UNKNOWN_STATE;
                }

                break;
            }

            case SM_DISPLAY_PENDING_ACK:
                std::cout << "DISPLAY HANDLER: STATE SM_DISPLAY_PENDING_ACK" << std::endl;
                // Wait for ACK from socket
                server->instance_recv_buffer_len[instance_id] = ::read(socket, (void*)instance_recv_buffer, server->recv_buffer_max_len);

                // Check read status
                if((server->instance_recv_buffer_len[instance_id] == MSG_ACK_LEN)
                    && (instance_recv_buffer[0] == MSG_ACK_BYTE)) {
                    // Receive was successful

                    display_state = SM_DISPLAY_WAITING_FOR_INTERVAL;
                }
                else {
                    // Lost connection
                    std::cout << "DISPLAY HANDLER: CONNECTION CLOSED" << std::endl;
                    server->instance_running[instance_id] = false;
                    display_state = SM_DISPLAY_UNKNOWN_STATE;
                }

                break;

            case SM_DISPLAY_UNKNOWN_STATE:
                // This shouldn't be reachable, as SM_SENSOR_UNKNOWN_STATE means that there is no connection
                std::cout << "DISPLAY HANDLER: CONNECTION CLOSED" << std::endl;
                server->instance_running[instance_id] = false;
                break;
        }
    }
}

void server_sensor_run_instance(SocketServer* server) {
    server->skt__run_instances();
}

void server_display_run_instance(SocketServer* server) {
    server->skt__run_instances();
}

int main(int argc, char** argv) {
    std::cout << "STARTING WEATHER SERVER" << std::endl;

    // Server buffers
    char client_display_recv_buffer[MSG_ACK_LEN];
    char client_display_send_buffer[LCD_DISPLAY_TEXT_LEN];
    char client_sensor_recv_buffer[SENSOR_READINGS_T_LEN];
    char client_sensor_send_buffer[MSG_MEAS_REQ_LEN];

    memset((void*)client_display_recv_buffer, 0, MSG_ACK_LEN);
    memset((void*)client_display_send_buffer, 0, LCD_DISPLAY_TEXT_LEN);
    memset((void*)client_sensor_recv_buffer, 0, SENSOR_READINGS_T_LEN);
    memset((void*)client_sensor_send_buffer, 0, MSG_MEAS_REQ_LEN);

    // Create server to communicate with the sensor
    SocketServer server_sensor = SocketServer(1, 
                                              (char*)client_sensor_recv_buffer, 
                                              SENSOR_READINGS_T_LEN, 
                                              (char*)client_sensor_send_buffer, 
                                              MSG_MEAS_REQ_LEN, 
                                              PORT_SERVER_SENSOR, 
                                              &server_sensor_handler);

    // Create server to communicate with the display
    SocketServer server_display = SocketServer(1, 
                                               (char*)client_display_recv_buffer, 
                                               MSG_ACK_LEN, 
                                               (char*)client_display_send_buffer, 
                                               LCD_DISPLAY_TEXT_LEN, 
                                               PORT_SERVER_DISPLAY, 
                                               &server_display_handler);

    bool setup_server_sensor_status = server_sensor.skt__socket_setup();
    bool setup_server_display_status = server_display.skt__socket_setup();

    if(setup_server_sensor_status && setup_server_display_status) {
        std::cout << "setup_server_display_status: " << setup_server_display_status << std::endl;
        std::cout << "setup_server_sensor_status: " << setup_server_sensor_status << std::endl;

        server_sensor.skt__set_active(true);
        server_display.skt__set_active(true);

        // Set initial values of temp and hum, representing no measurement yet
        latest_readings.temp = std::numeric_limits<float>::max();
        latest_readings.rel_hum = std::numeric_limits<float>::max();

        // Run servers
        std::thread server_sensor_instance(server_sensor_run_instance, &server_sensor);
        std::thread server_display_instance(server_display_run_instance, &server_display);

        server_sensor_instance.join();
        server_display_instance.join();
    }

    std::cout << "STOPPING WEATHER SERVER" << std::endl;
    return 0;
}
