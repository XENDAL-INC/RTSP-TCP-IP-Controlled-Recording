#include <chrono>
#include <cstring>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#endif
#include "Camera.h"

struct Records {
    std::vector<Camera> cameras;
    cv::VideoCapture* capCam1;
    cv::VideoCapture* capCam2;
    string recordName;
    bool isRecording = true; bool isStopped = false; bool isDrop = false;

    Records(string stream_url[], string record_name, cv::VideoCapture* capture_1, cv::VideoCapture* capture_2)
        : recordName(record_name), capCam1(capture_1), capCam2(capture_2) {
        std::string camName1 = "Camera_1";
        Camera newCamera1(recordName, camName1, stream_url[0], &isRecording, &isStopped, &isDrop, capCam1);
        cameras.push_back(newCamera1);

        std::string camName2 = "Camera_2";
        Camera newCamera2(recordName, camName2, stream_url[1], &isRecording, &isStopped, &isDrop, capCam2);
        cameras.push_back(newCamera2);
    }

    void startRecording()
    {
        if (cameras.size() > 0)
        {
            std::thread threadRec1(&Camera::recordCamera, &cameras[0]);
            std::thread threadRec2(&Camera::recordCamera, &cameras[1]);
            
            threadRec1.detach();
            threadRec2.detach();
        }
    }

    void stopRecording(bool stop, bool drop)
    {
        isStopped = stop;
        isDrop = drop;

        std::thread threadStop1(&Camera::stopRecording, &cameras[0], stop, drop);
        std::thread threadStop2(&Camera::stopRecording, &cameras[1], stop, drop);

        threadStop1.detach();
        threadStop2.detach();
    }
};

int handleConnections(int port, std::string rtsp_url[])
{
    #ifdef _WIN32
        // Initialize Winsock
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        {
            printf("Failed to initialize Winsock.\n");
            return 1;
        }
    #endif

        // Create a socket
        SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket == -1)
        {
            printf("Failed to create socket.\n");
    #ifdef _WIN32
            WSACleanup();
    #endif
            return 1;
        }

        // Bind the socket to the specified IP and port
        struct sockaddr_in serverAddress;
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_port = htons(port);
        serverAddress.sin_addr.s_addr = INADDR_ANY;

        if (::bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) == -1)
        {
            printf("Failed to bind socket.\n");
#ifdef _WIN32
            closesocket(serverSocket);
            WSACleanup();
#else
            close(serverSocket);
#endif
            return 1;
        }

        // Listen for incoming connections
        if (listen(serverSocket, 1) == -1)
        {
            printf("Failed to listen on socket.\n");
    #ifdef _WIN32
            closesocket(serverSocket);
            WSACleanup();
    #else
            close(serverSocket);
    #endif
            return 1;
        }

        std::cout << "Listening on Port 5300..." << std::endl;

        // Accept incoming connections
        struct sockaddr_in clientAddress;
        int clientAddressLength = sizeof(clientAddress); // Modified this line
        SOCKET clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientAddressLength);
        if (clientSocket == -1)
        {
            printf("Failed to accept connection.\n");
            #ifdef _WIN32
                    closesocket(serverSocket);
                    WSACleanup();
            #else
                    close(serverSocket);
            #endif
            return 1;
        }
        else
        {
            std::cout << "Client Connected..." << std::endl;
        }

        // Receive data from the client
        char buffer[30];
        int bytesRead;
        std::string msgReceived;

        int recording = 0; int recordCount = 1;
        std::vector<Records> records;

        cv::VideoCapture capCam1(rtsp_url[0]);
        cv::VideoCapture capCam2(rtsp_url[1]);

        while ((bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0)
        {
            // Process the received data
            msgReceived.append(buffer, bytesRead);

            if (msgReceived == "START" && recording == 0)
            {
                std::cout << "Start Recording..." << std::endl;
                recording = 1;

                std::string recordName = "Record_" + std::to_string(recordCount);

                Records newRecord(rtsp_url, recordName, &capCam1, &capCam2);
                records.push_back(newRecord);
                
                if (records.size() > 0)
                {
                    records[records.size() - 1].startRecording();
                }

                recordCount++;
            }
            else if (msgReceived == "NO_READ" && recording == 1)
            {
                std::cout << "Stop Recording..." << std::endl;
                recording = 0;
                if (records[records.size() - 1].isStopped == false)
                {
                    records[records.size() - 1].stopRecording(true, false);
                }
            }
            else if (recording == 1)
            {
                std::cout << "Dropping Recording..." << std::endl;
                recording = 0;
                if (records[records.size()-1].isStopped == false)
                {
                    records[records.size() - 1].stopRecording(true, true);
                }
            }
            msgReceived = "";
        }

        // Close the sockets
    #ifdef _WIN32
        closesocket(clientSocket);
        closesocket(serverSocket);
        WSACleanup();
    #else
        close(clientSocket);
        close(serverSocket);
    #endif

    return 0;
}

int main()
{
    //setup rtsp
    std::string username = "admin";
    std::string password = "Pass4acce$$";
    std::string rtsp_url[2];
    for (int i = 0; i < 2; i++)
    {
        rtsp_url[i] = "rtsp://";
        rtsp_url[i] += username;
        rtsp_url[i] += ":";
        rtsp_url[i] += password;
    }
    rtsp_url[0] += "@192.168.10.202";
    rtsp_url[1] += "@192.168.10.203";

    int port = 5300;

    handleConnections(port, rtsp_url);

    std::cout<<"Client Disconnected..."<<std::endl;

    return 0;
}