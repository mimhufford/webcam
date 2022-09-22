#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <stdio.h>

#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")

using namespace cv;
using namespace std;

const String window = " ";
int ox = 0;
int oy = 0;

void mouseCallback(int event, int x, int y, int flags, void *userdata)
{
    // Normalise x and y to window size
    auto r = getWindowImageRect(window);
    x = x * r.width / 640;
    y = y * r.height / 480;

    if (event == EVENT_LBUTTONDOWN)
    {
        ox = x;
        oy = y;
    }

    if (flags == EVENT_FLAG_LBUTTON)
    {
        moveWindow(window, r.x+x-ox, r.y+y-oy);
    }
}

int main()
{
    Mat frame;
    VideoCapture webcam;

    webcam.open(0);

    if (!webcam.isOpened()) return -1;

    while (true)
    {
        webcam.read(frame);

        if (frame.empty()) break;

        namedWindow(window, WINDOW_NORMAL | WINDOW_GUI_EXPANDED);
        setWindowProperty(window, WND_PROP_TOPMOST, 1);
        setMouseCallback(window, mouseCallback);
        imshow(window, frame);

        // Keep 4:3 aspect ratio
        auto rect = getWindowImageRect(window);
        resizeWindow(window, rect.width, rect.width * 3 / 4);

        // Exit if a key is pressed
        if (waitKey(5) >= 0) break;
    }

    return 0;
}