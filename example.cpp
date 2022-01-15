#include "tuner.h"
#include <thread>
#include <chrono>
#include <opencv2/opencv.hpp>
using namespace std;
using namespace cv;
Mat img;
void callback(const vector<int> &para)
{
    cout<<"Thresh: " << para[0]<<endl;
    Mat gray;
    cvtColor(img, gray, COLOR_BGR2GRAY);
    threshold(gray, gray, para[0], 255, THRESH_BINARY);
    publish(para[1], gray);
    //imshow("w", gray);
    //waitKey(30);
    return;
}
signed main()
{
    img = imread("a.webp");
    assert(!img.empty());
    waitKey(30);
    Tuner tuner("tuneeee", callback);
    tuner.addWidgets(NW("thresh", 255), VW("threshed"), VW("video"));
    VideoCapture cap("aaa.mp4");
    Mat frame;
    if(tuner.start()) {
        for(;;) {
            if(cap.read(frame)) {
                publish(tuner.data[2], frame);
                this_thread::sleep_for(chrono::milliseconds(1000 / 30)); // fps=30
            } else {
                break;
            }
        }
    }
    this_thread::sleep_for(chrono::seconds(10000));
    return 0;
}