
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

    long ok_H_Num = 0;
    long ok_T_Num = 0;
    long ok_P_Num = 0;
    long failNum = 0;

    unsigned int ProcMem, ProcVirtualmem;
    float ProcCpu;
    WINDOW *my_window = NULL;

  public:
    long long OnceTimes = 0;
    bool isRun = false;
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
    long Add_H_Ok()
    {
        return ++ok_H_Num;
    }
    long Add_T_Ok()
    {
        return ++ok_T_Num;
    }
    long Add_P_Ok()
    {
        return ++ok_P_Num;
    }
    /**
     * 设置单次时间
     */
    void SetOnceTime(long long times)
    {
        if (OnceTimes == 0)
        {
            OnceTimes = times;
        }
        else
        {
            OnceTimes += times;
            OnceTimes = OnceTimes / 2;
        }
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
                ok_H_Num = 0;
                ok_T_Num = 0;
                ok_P_Num = 0;
                failNum = 0;
                OnceTimes = 0;
            case 'c':
            case 'C':
                memset(serialNumber, 0x00, sizeof(serialNumber));
                memset(dateOfBirth, 0x00, sizeof(dateOfBirth));
                memset(dateOfExpiry, 0x00, sizeof(dateOfExpiry));
                break;

            case 's':
            case 'S':
                isRun = true;
                break;
            case 'e':
            case 'E':
                isRun = false;
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
        my_window = initscr(); //curses初始化
        noecho();              //默认不将输入的字符显示在屏幕上

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
        move(12, 1);
        hline('-', 78);
        move(MAX_Y - 1, 1);
        hline('-', 78);

        mvprintw(1, 2, "CSC Demo");
        mvprintw(3, 2, "Number          :%11s", serialNumber);
        mvprintw(4, 2, "Birth           :%11s", dateOfBirth);
        mvprintw(5, 2, "Expiry          :%11s", dateOfExpiry);

        long long sum = ok_H_Num + ok_T_Num + ok_P_Num + failNum;
        long okNum = ok_H_Num + ok_T_Num + ok_P_Num;

        mvprintw(3, 42, "SuccessRate    :%9.2f%%", sum > 0 ? (((double)okNum) * 100 / sum) : 0);
        mvprintw(4, 42, "Success        :%10ld", okNum);
        mvprintw(5, 42, "TestTotal      :%10ld", sum);

        mvprintw(7, 2, "Passport        :%10ld", ok_P_Num);
        mvprintw(8, 2, "PassCheck       :%10ld", ok_H_Num);
        mvprintw(9, 2, "ReturnHomeCard  :%10ld", ok_T_Num);

        mvprintw(11, 2, "OCR Once Time : %10.lld ms", OnceTimes);

        mvprintw(13, 2, "Key Tip:");
        mvprintw(14, 4, "C : Clear Number , Birth and Expiry");
        mvprintw(15, 4, "S : Start Test");
        mvprintw(16, 4, "E : End Test");
        mvprintw(17, 4, "R : Reset Test");

        mvprintw(MAX_Y - 2, 4, "Q : Exit Test");
        mvprintw(MAX_Y, 2, "Test is in progress...");
        refresh();
        // wrefresh(my_window);
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
    int index = 0;

    int ret;
    ret = pthread_create(&id, NULL, WaitForKey, NULL); //创建线程
    if (ret != 0)
    {
        exit(1);
    }

    CSC_Init();
    // CSC_SetConsoleLevel(3);

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
        if (csc.isRun)
        {
            int width = frame.size().width;
            int height = frame.size().height;

            Rect rect(0, height / 25, width - width / 40, height - height / 25);

            Mat frame1(frame, rect);

            cv::flip(frame1, frame, 1);

            char serialNumber[15], dateOfBirth[9], dateOfExpiry[9];
            int flag = 0;

            long long startTime = CSC_GetTime(), endTime;
            int type = CSC_GetKey(frame.data, frame.cols, frame.rows,frame.channels(), serialNumber, dateOfBirth, dateOfExpiry);
                if (type>0)
                {
                    csc.updateData(serialNumber, dateOfBirth, dateOfExpiry);
                    switch(type){
                        case 1:csc.Add_P_Ok();break;
                        case 2:csc.Add_T_Ok();break;
                        case 3:csc.Add_H_Ok();break;
                        default:csc.AddFail();break;
                    }
                }

            endTime = CSC_GetTime();
            csc.SetOnceTime(endTime - startTime);
        }

        csc.RefreshUI();
        usleep(10 * 000);
    }
    csc.keepRun = false;
    return 0;
}
