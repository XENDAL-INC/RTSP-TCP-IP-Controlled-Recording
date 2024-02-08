# RTSP Camera Recording System

This repository contains code for a simple RTSP camera recording system with the combination of socket programming to control the system automatically.
It allows you to connect to multiple RTSP cameras, start recording streams, stop recording, and even drop recorded files if needed.

## Requirements

- OpenCV (version X.X.X)

[Clone the repository]:
git clone https://github.com/XENDAL-INC/RTSP-TCP-IP-Controlled-Recording.git

## Installation
[Installation steps]

1. Modify the `main` function in `rtsp_camera_record.cpp` to set the RTSP URLs for your cameras and the desired port for the server to listen on.
2. Build the project.
3. Run the executable:
4. Connect to the server using a TCP client (e.g., Telnet) and send commands to start, stop, or drop recordings.

## Structure

- `Camera.h`: Class definition for the `Camera` object used to handle individual cameras.
- `rtsp_camera_record.cpp`: Main source file containing the implementation of the recording system and the server that listens for commands.
