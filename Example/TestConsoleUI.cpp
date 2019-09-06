
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>

#include <math.h>
#include <iostream>

#include <cstdio>

#include "CSC/CSC.h"

#include <stdio.h>
#include <pthread.h>
#include <sys/time.h>
#include <curses.h> //这个就是我们要用到的额外的ui库
#include <ncurses.h>
#include <locale.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <string.h>

using namespace cv;
using namespace std;

pthread_t id; //声明一个linux线程，按键等待线程

class CscDemoUI
{
  private:
    const int MAX_X = 70;   //场地宽
    const int MAX_Y = 22;   //场地长
    const int CORNER_X = 0; //左上角x坐标
    const int CORNER_Y = 0; //左上角y坐标
    char serialNumber[15], dateOfBirth[15], dateOfExpiry[15];
    char input;

    long okNum = 0;
    long failNum = 0;

    unsigned int ProcMem, ProcVirtualmem;
    float ProcCpu;

  public:
    void updateData(char serialNumber[15], char dateOfBirth[9], char dateOfExpiry[9])
    {
        memcpy(this->serialNumber, serialNumber, 15);
        memcpy(this->dateOfBirth, dateOfBirth, 9);
        memcpy(this->dateOfExpiry, dateOfExpiry, 9);
    }

    long AddFail()
    {
        return ++failNum;
    }
    long AddOk()
    {
        return ++okNum;
    }

    void *WaitForKey(void *para)
    {
        while (keepRun)
        {
            RefreshUI();
            usleep(1000); //为什么要加这个，不知道什么原因，在curses下，如果建了这个线程并且不加这句话的话就会出现花屏现象。很难看
            input = getch();

            switch (input)
            {
            case 'r':
            case 'R':
                okNum = 0;
                failNum = 0;
            case 'c':
            case 'C':
                memset(serialNumber, 0x00, sizeof(serialNumber));
                memset(dateOfBirth, 0x00, sizeof(dateOfBirth));
                memset(dateOfExpiry, 0x00, sizeof(dateOfExpiry));
                break;

            case 'q':
            case 'Q':
                keepRun = false;
                break;
                for (int i = 0; i < 12; i++)
                {
                    mvprintw(MAX_Y, i, " ");
                }
                break;
            }
        }
        return NULL;
    }
    bool keepRun = true;

    CscDemoUI()
    {
        memset(serialNumber, 0x00, sizeof(serialNumber));
        memset(dateOfBirth, 0x00, sizeof(dateOfBirth));
        memset(dateOfExpiry, 0x00, sizeof(dateOfExpiry));

        setlocale(LC_ALL, "");
        initscr(); //curses初始化
        noecho();  //默认不将输入的字符显示在屏幕上

        refresh(); //刷新画布
    }
    ~CscDemoUI()
    {
        move(MAX_Y, 0);
        refresh();
        endwin();
    }

    void RefreshUI()
    {
        box(stdscr, '|', '-');
        move(2, 1);
        hline('-', 78);
        move(3, 40);
        vline('|', 3);
        move(6, 1);
        hline('-', 78);
        move(7, 40);
        vline('|', 3);
        move(10, 1);
        hline('-', 78);
        move(MAX_Y - 7, 1);
        hline('-', 78);
        move(MAX_Y - 1, 1);
        hline('-', 78);

        mvprintw(1, 2, "CSC Demo");
        mvprintw(3, 2, "Number       :%11s", serialNumber);
        mvprintw(4, 2, "Birth        :%11s", dateOfBirth);
        mvprintw(5, 2, "Expiry       :%11s", dateOfExpiry);

        long long sum = okNum + failNum;

        mvprintw(3, 42, "SuccessRate :%9.2f%%", sum > 0 ? (((double)okNum) * 100 / sum) : 0);
        mvprintw(4, 42, "Success     :%10ld", okNum);
        mvprintw(5, 42, "TestTotal   :%10ld", sum);

        // if (sum % 10 == 0)
        // {
        //     CSC_GetSystemInfo(ProcMem, ProcVirtualmem, ProcCpu);
        //     mvprintw(7, 2, "Mem          :%10.4f Mb", (float)ProcMem / 1024);
        //     mvprintw(8, 2, "Virtualmem   :%10.4f Mb", (float)ProcVirtualmem / 1024);
        //     mvprintw(9, 2, "Cpu          :%10.4f %%", ProcCpu * 100);
        // }

        mvprintw(7, 2, "passport     :%10ld", 0);
        mvprintw(8, 2, "PassCheck    :%10ld", 0);
        mvprintw(9, 2, "Taiwan       :%10ld", 0);

        move(MAX_Y, MAX_X);
        mvprintw(MAX_Y - 6, 2, "Key Tip:");
        mvprintw(MAX_Y - 5, 4, "C : Clear Number , Birth and Expiry");
        mvprintw(MAX_Y - 4, 4, "R : Reset Test");

        mvprintw(MAX_Y - 2, 4, "Q : Exit Test");
        mvprintw(MAX_Y, 2, "Test is in progress...");
        refresh();
    }
};

//主函数

CscDemoUI csc;
void *WaitForKey(void *para)
{
    return csc.WaitForKey(para);
}
int main(int argc, char **argv)
{

    int ret;
    ret = pthread_create(&id, NULL, WaitForKey, NULL); //创建线程
    if (ret != 0)
    {
        exit(1);
    }

    CSC_Init();

    VideoCapture cap(0); //capture the video from web cam
    if (!cap.isOpened()) // if not success, exit program
    {
        cout << "Cannot open the web cam" << endl;
        return -1;
    }

    while (csc.keepRun)
    {

        Mat frame;
        bool bSuccess = cap.read(frame); // read a new frame from video

        if (!bSuccess) //if not success, break loop
        {
            cout << "Cannot read a frame from video stream" << endl;
            break;
        }

        int width = frame.size().width;
        int height = frame.size().height;
        // Mat frame1 = frame(Range(height / 20, height / 20), Range(width - height / 20, height - height / 20));

        // Rect rect(width / 25, height / 25, width - width / 25, height - height / 25);
        Rect rect(0, height / 25, width - width / 40, height - height / 25);

        Mat frame1(frame, rect);

        cv::flip(frame1, frame, 1);

        char serialNumber[15], dateOfBirth[9], dateOfExpiry[9];
        int flag = 0;
        if (CSC_GetKey(frame.data, frame.cols, frame.rows,frame.channels(), serialNumber, dateOfBirth, dateOfExpiry))
        {
            csc.updateData(serialNumber, dateOfBirth, dateOfExpiry);
            // printf("识别结果 : \t证件号码：%s    生日：%s    有效期：%s \n", serialNumber, dateOfBirth, dateOfExpiry);
        }
        if (flag == 1)
            csc.AddOk();
        else if (flag == -1)
            csc.AddFail();

        // namedWindow("image", cv::WINDOW_NORMAL);
        // imshow("image", frame);
        // if (waitKey(10) == 27)
        // {
        //     break;
        // }

        csc.RefreshUI();
        usleep(10 * 000);
    }
    csc.keepRun = false;
    return 0;
}
