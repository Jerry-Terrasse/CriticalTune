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
    imshow("w", gray);
    waitKey(30);
    return;
}
int main()
{
    img = imread("a.webp");
    assert(!img.empty());
    imshow("w", img);
    waitKey(30);

    Tuner tuner("tuneeee", callback);
    tuner.addWidgets(NW("thresh", 255));
    tuner.start();
    
    this_thread::sleep_for(chrono::seconds(10000));
    return 0;
}