#pragma once

#include <iostream>
#include <string>
#include <chrono>
#include <ctime>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

class Camera
{
private:
    bool* isRecording; bool* isStopped; bool* isDrop;
    string record_name; string name; string stream_url; string output_file;
    int frame_written;
    chrono::time_point<chrono::system_clock> start_time;
    cv::VideoCapture* cap;

public:
    Camera(string recordName, string camName, string streamUrl, bool* rec, bool* stop, bool* drop, cv::VideoCapture *capture)
        : cap(capture) {
        isRecording = rec; isStopped = stop; isDrop = drop;
        record_name = recordName; name = camName; stream_url = streamUrl;
        frame_written = 0;
        output_file = "dump_vid/" + camName + "_" + getTimestamp() + ".avi";
    }

    void recordCamera()
    {
        //cv::VideoCapture cap(stream_url);

        if (!cap->isOpened()) {
            std::cout << "Failed to open RTSP camera." << std::endl;
        }

        int frame_width = static_cast<int>(cap->get(cv::CAP_PROP_FRAME_WIDTH));
        int frame_height = static_cast<int>(cap->get(cv::CAP_PROP_FRAME_HEIGHT));

        cv::VideoWriter out(output_file, cv::VideoWriter::fourcc('X', 'V', 'I', 'D'), 25, cv::Size(frame_width, frame_height));
        cout << record_name << "_" << name << " has started recording!" << endl;

        start_time = std::chrono::system_clock::now(); // Initialize start_time
        
        while (*isRecording == true) {
            cv::Mat frame;

            *cap >> frame;

            if (!frame.empty())
            {
                out.write(frame);
                frame_written++;
            }
        }

        //cap.release();
        out.release();

        if (*isDrop == true) 
        {
            if (std::remove(output_file.c_str()) != 0) {
                std::cerr << "Failed to delete " << record_name << "_" << name << endl;
            }
            else {
                cout << record_name << "_" << name << " has been dropped successfully!" << endl;
            }
        }
        else
        {
            cout << record_name << "_" << name << " has finished recording!" << endl;
        }
    }

    void stopRecording(bool stop, bool drop)
    {
        *isStopped = stop;
        *isDrop = drop;

        double elapsed_seconds = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - start_time).count() / 1000.0;
        int expectedFrames = static_cast<int>(25 * elapsed_seconds);

        cout << "expectedFrames:" << expectedFrames << " frames_written:" << frame_written << " Elapsed-Time:" << elapsed_seconds << endl;

        while (frame_written < expectedFrames)
        {
            *isRecording = true;
        }
        *isRecording = false;
    }

private:
    string getTimestamp()
    {
        const time_t now = chrono::system_clock::to_time_t(chrono::system_clock::now());
        struct tm timeinfo;
        char timestamp[20];
        if (localtime_s(&timeinfo, &now) == 0)
        {
            strftime(timestamp, sizeof(timestamp), "%Y-%m-%d_%H-%M-%S", &timeinfo);
            return string(timestamp);
        }
        else
        {
            return "error";
        }
    }
};
