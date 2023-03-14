#!/bin/bash

# Build executable
./pre_build.sh
cd build && make
cd ..
# Create and run the Docker container
docker build -t weather_server_img .
#docker rm -f weather_server_cont
docker run -d -p 3006:3006 -p 3007:3007 --restart=always --name weather_server_cont weather_server_img
docker system prune