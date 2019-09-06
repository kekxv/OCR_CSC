#include <opencv2/opencv.hpp>

#include "Log.h"
#include "CSC/CSC.h"

using namespace cv;
using namespace std;
using namespace kekxv;

int GetFileList(vector<string> *data, const char *path = "./image") {
    char cmd[200];
    sprintf(cmd, "ls %s/* 2>/dev/null", path);
    FILE *fp = nullptr;
    char comName[256];
    if ((fp = popen(cmd, "r")) != nullptr) {
        while (fgets(comName, sizeof(comName), fp) != nullptr) {
            comName[strlen(comName) - 1] = 0;
            data->push_back(comName);
            // LogI("File","%s",comName);
        }
        pclose(fp);
    }
}

/**
 * @brief 多线程
 * 
 * @param arg 
 * @return void* 
 */
void *OcrThread(void *arg) {
    pthread_exit(nullptr);
    return nullptr;
}

int main(int argc, char **argv) {
    char serialNumber[15];
    char dateOfBirth[9];
    char dateOfExpiry[9];
    int isOk = 0;

    Log::setConsoleLevel(3);
    CSC_SetConsoleLevel(3);
    long long s = Log::GetTime(), se;
    CSC_Init("OCR_B");
    LogI("Test", "CSC_Init \t\t\t%lld ms", Log::GetTime() - s);
    int t1 = 0, s1 = 0;
#ifdef v4l2Tool
    v4l2Tool video;
    VideoDevice vd[15];
    int l = kekxv::v4l2Tool::GetDevices(vd, 15);
    if (l == 0)return 0;

    video.width = 1024;
    video.height = 768;
    video.fps = 10;
    cv::namedWindow("camera", cv::WindowFlags::WINDOW_NORMAL ^ cv::WindowFlags::WINDOW_KEEPRATIO);
    cv::resizeWindow("camera", 500, 500);

    video.Open(vd[0].path);

    if (video.isClose)return 0;
    video.SetBrightness(6);
    int w = video.dev->width, h = video.dev->height, b = 3;
    auto *data = new unsigned char[w * h * b];
    if (data == nullptr)return 0;
    do {
        if (0 != video.grabFrame(data, b))break;
        Mat frame(h,w, CV_MAKETYPE(CV_8U,b),data);
        cv::flip(frame, frame, -1);

        int type = CSC_GetKey(frame.data, frame.cols, frame.rows, frame.channels(), serialNumber, dateOfBirth,
                              dateOfExpiry);
        if (type > 0) {
            LogD("CSC", "T[%6s] N[%s] B[%s] E[%s]", (type > 1 ? (type > 2 ? "通行证" : "回乡证") : "护照"), serialNumber,
                 dateOfBirth, dateOfExpiry);
            s1++;
        }

        imshow("camera",frame);
    } while (waitKey(50) !=27);
    delete[] data;

#endif

#if 0
    VideoCapture cap(0); //capture the video from web cam
    if (!cap.isOpened()) // if not success, exit program
    {
        cout << "Cannot open the web cam" << endl;
        return -1;
    }
    int b = 21;
    int c = 33;
    // namedWindow("Control", cv::WINDOW_AUTOSIZE);
    // cv::createTrackbar("LowH", "Control", &b, 179); //Hue (0 - 179)
    // createTrackbar("HighH", "Control", &c, 179);
    while (true)
    {
        Mat frame;
        bool bSuccess = cap.read(frame); // read a new frame from video

        if (!bSuccess) //if not success, break loop
        {
            cout << "Cannot read a frame from video stream" << endl;
            break;
        }
//        cv::flip(frame, frame, 1);
        imshow("t", frame);
        se = s = Log::GetTime();

        s = Log::GetTime();
        
        char serialNumber[15], dateOfBirth[9], dateOfExpiry[9];
        int type = CSC_GetKey(frame.data,frame.cols,frame.rows,frame.channels() ,serialNumber, dateOfBirth, dateOfExpiry);
        if(type > 0){
            LogD("CSC", "T[%4s] N[%s] B[%s] E[%s]", (type>1?(type>2?"通行证":"回乡证"):"护照"),serialNumber, dateOfBirth, dateOfExpiry);
            s1++;
        }

        LogI("CSC", "findContours \t%lld ms", Log::GetTime() - s);
        // LogD("CSC", "\t%lld ", Tool::SystemGetProcMem(Tool::SystemGetPid()) + Tool::SystemGetProcVirtualmem(Tool::SystemGetPid()) );

        waitKey(100);

        // waitKey(10);
        // se = s = Log::GetTime();
        // bool flag = CSC_GetPassport(frame.data,frame.cols,frame.rows,serialNumber, dateOfBirth, dateOfExpiry,&isOk);
        // LogI("Test", "CSC_GetPassport \t\t%4lld ms %s", Log::GetTime() - s,flag?"OK":"Fail");
        // t1++;
        // s1+=flag?1:0;
        // usleep(500 * 1000);
    }
#endif
#if 1
    vector<string> data;
    GetFileList(&data, "/home/caesar/Desktop/ocr_image");
    for (int index = 0; index < data.size() && index < data.size(); index++) {
//        if (index == data.size() - 1)
//        {
//            index = 0;
//        }
        Mat frame = imread(data[index]);

        se = s = Log::GetTime();
        t1++;
//        char serialNumber[15], dateOfBirth[9], dateOfExpiry[9];
        int type = CSC_GetKey(frame.data, frame.cols, frame.rows, frame.channels(), serialNumber, dateOfBirth,
                              dateOfExpiry);
        if (type > 0) {
            LogD("CSC", "T[%6s] N[%s] B[%s] E[%s]", (type > 1 ? (type > 2 ? "通行证" : "回乡证") : "护照"), serialNumber,
                 dateOfBirth, dateOfExpiry);
            s1++;
        }
        LogI("CSC", "findContours \t%lld ms", Log::GetTime() - s);

//        imshow("frame", frame);
//        waitKey();
//        sleep(1);
    }
#endif
    LogI("CSC", "T : %d ", t1);
    LogI("CSC", "S : %d ", s1);
    LogI("CSC", "R : %2.2f %%", (float) s1 * 100 / t1);
    return 0;
}