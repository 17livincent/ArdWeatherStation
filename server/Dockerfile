# syntax=docker/dockerfile:1

FROM ubuntu:latest

# Expose ports
EXPOSE 3006 3007

RUN apt update
RUN  apt upgrade -y
ADD build/src/weather_server /usr/local/bin/
CMD ./usr/local/bin/weather_server