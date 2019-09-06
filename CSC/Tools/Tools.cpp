#include <tesseract/baseapi.h>

#include <math.h>
#include <iostream>
#include <unistd.h>
#include <regex>

#include "Tools.h"
#include "System/SystemInfo.hpp"
#include "Log.h"
#include "MRZ.h"

using namespace std;
using namespace cv;

namespace kekxv {
    string faceXml = "./haarcascades/haarcascade_frontalface_default.xml";
    CascadeClassifier faceCascade(faceXml);
    const int tessearct_api_len = 4;
    tesseract::TessBaseAPI tessearct_api[tessearct_api_len];
    pthread_mutex_t tessearct_api_lock[tessearct_api_len];

    bool isInitTesseract = false;

// string languagePath = "./tessdata", languageType = "eng";
    unsigned int Tool::SystemGetPid(const char *process_name, const char *user) {
        if (process_name == nullptr) {
            return getpid();
        }
        return get_pid(process_name, user);
    }

/**
 * 获取进程占用内存
 */
    unsigned int Tool::SystemGetProcMem(unsigned int pid) {
        return get_proc_mem(pid);
    }

/**
 * 获取进程占用虚拟内存
 */
    unsigned int Tool::SystemGetProcVirtualmem(unsigned int pid) {
        return get_proc_virtualmem(pid);
    }

/**
 * 获取CPU占用率
 */
    float Tool::SystemGetProcCpu(unsigned int pid) {
        return get_proc_cpu(pid);
    }

    bool Tool::InitFaceCascade(string faceXml) {
        return faceCascade.load(faceXml);
    }

    double Tool::angle(cv::Point pt1, cv::Point pt2, cv::Point pt0) {
        double dx1 = pt1.x - pt0.x;
        double dy1 = pt1.y - pt0.y;
        double dx2 = pt2.x - pt0.x;
        double dy2 = pt2.y - pt0.y;
        return (dx1 * dx2 + dy1 * dy2) / sqrt((dx1 * dx1 + dy1 * dy1) * (dx2 * dx2 + dy2 * dy2) + 1e-10);
    }

/**
 * 初始化 Tesseract Ocr库
 */
    int Tool::InitTesseract(const char *languageType, const char *tessedit_char_whitelist, const char *languagePath) {
        LogI("Tesseract", "准备初始化字库！");
        LogI("Tesseract", "版本：%s", tesseract::TessBaseAPI::Version());
        // kekxv::languagePath = languagePath;
        // kekxv::languageType = languageType;
        for (int i = 0; i < tessearct_api_len; i++) {
            tessearct_api[i].SetVariable("load_system_dawg", "0");
            tessearct_api[i].SetVariable("load_freq_dawg", "0");
            tessearct_api[i].SetVariable("load_unambig_dawg", "0");
            tessearct_api[i].SetVariable("load_punc_dawg", "0");
            tessearct_api[i].SetVariable("load_number_dawg", "0");
            tessearct_api[i].SetVariable("load_fixed_length_dawgs", "0");
            tessearct_api[i].SetVariable("load_bigram_dawg", "0");
            tessearct_api[i].SetVariable("wordrec_enable_assoc", "0");
            tessearct_api[i].SetVariable("tessedit_enable_bigram_correction", "0");
            tessearct_api[i].SetVariable("assume_fixed_pitch_char_segment", "1");
            tessearct_api[i].SetVariable("tessedit_char_whitelist", tessedit_char_whitelist);

            int nRet = tessearct_api[i].Init(languagePath, languageType);
            if (nRet != 0) {
                LogE("Tesseract", "初始化字库失败！");
                if (i > 0) {
                    for (int j = 0; j < i; j++) {
                        pthread_mutex_destroy(&tessearct_api_lock[i]);
                    }
                }
                isInitTesseract = false;
                return -1;
            }
            // tessearct_api_lock[i] = PTHREAD_MUTEX_INITIALIZER;
            int ret = pthread_mutex_init(&tessearct_api_lock[i], NULL);
            tessearct_api[i].SetPageSegMode(tesseract::PSM_SINGLE_BLOCK);

            // void *data;
            // LogI("Tesseract", "getOpenCLDevice : %d", tessearct_api[i].getOpenCLDevice(&data));
        }
        isInitTesseract = true;
        LogI("Tesseract", "初始化字库成功！");
        return 0;
    }

    void Tool::DeleteSpase(string &sNewTag) {
        int begin = 0;
        begin = sNewTag.find(" ", begin); //查找空格在str中第一次出现的位置
        while (begin != string::npos)     //表示字符串中存在空格
        {
            sNewTag.replace(begin, 1, "");    // 用空串替换str中从begin开始的1个字符
            begin = sNewTag.find(" ", begin); //查找空格在替换后的str中第一次出现的位置
        }
        begin = 0;
        begin = sNewTag.find("\n", begin); //查找空格在str中第一次出现的位置
        while (begin != string::npos)      //表示字符串中存在空格
        {
            sNewTag.replace(begin, 1, "");     // 用空串替换str中从begin开始的1个字符
            begin = sNewTag.find("\n", begin); //查找空格在替换后的str中第一次出现的位置
        }
    }

/**
 * 获取相似的数字
 */
    char Tool::GetNumAsChar(char c, char max) {
        switch (c) {
            case 'O':
            case 'Q':
            case 'U':
            case 'D':
                return '0';
            case 'E':
            case 'S':
                if (max != 0 && '5' > max)
                    return '4';
                return '5';
                break;
            case 'A':
                return '4';
            case 'T':
            case 'I':
                return '1';
            case 'B':
                if (max != 0 && '8' > max)
                    return '0';
                return '8';
            case 'G':
                if (max != 0 && '6' > max)
                    return '0';
                return '6';
            case 'Z':
            case 'P':
                return '2';
        }
        return c;
    }

/**
 * @brief 检查密码是否正确
 * 
 * @param serialNumber 证件号码
 * @param dateOfBirth 6位 生日
 * @param dateOfExpiry 6位 有效期
 * @return true 正确
 * @return false 错误
 */
    bool Tool::CheckMRZ(char *serialNumber, char *dateOfBirth, char *dateOfExpiry) {
        int serialNumberLen = static_cast<int>(strlen(serialNumber));
        int dateOfBirthLen = static_cast<int>(strlen(dateOfBirth));
        int dateOfExpiryLen = static_cast<int>(strlen(dateOfExpiry));
        if (serialNumberLen < 10 || dateOfBirthLen != 7 || dateOfExpiryLen != 7) {
            return false;
        }

        for (int i = 0; i < dateOfBirthLen; i++) {
            switch (i) {
                case 2: {
                    dateOfBirth[i] = Tool::GetNumAsChar(dateOfBirth[i], '1');
                }
                    break;
                case 4: {
                    dateOfBirth[i] = Tool::GetNumAsChar(dateOfBirth[i], '3');
                }
                    break;
                default: {
                    dateOfBirth[i] = Tool::GetNumAsChar(dateOfBirth[i]);
                }
                    break;
            }
        }
        for (int i = 0; i < dateOfExpiryLen; i++) {
            switch (i) {
                case 2: {
                    dateOfExpiry[i] = Tool::GetNumAsChar(dateOfExpiry[i], '1');
                }
                    break;
                case 4: {
                    dateOfExpiry[i] = Tool::GetNumAsChar(dateOfExpiry[i], '3');
                }
                    break;
                default: {
                    dateOfExpiry[i] = Tool::GetNumAsChar(dateOfExpiry[i]);
                }
                    break;
            }
        }
        serialNumber[serialNumberLen - 1] = Tool::GetNumAsChar(serialNumber[serialNumberLen - 1]);
        if (!MRZ::Check(serialNumber, serialNumberLen - 1, serialNumber[serialNumberLen - 1])) {
            for (int i = 1; i < serialNumberLen - 1; i++) {
                switch (serialNumber[i]) {
                    case 'Q':
                    case 'D':
                        serialNumber[i] = '0';
                        break;
                    case 'S':
                        serialNumber[i] = '5';
                        break;
                    case 'B':
                        serialNumber[i] = '8';
                        break;
                }
            }
            if (!MRZ::Check(serialNumber, serialNumberLen - 1, serialNumber[serialNumberLen - 1])) {
                // LogE("Test","%s",serialNumber);
                return false;
            }
        }
        if (!MRZ::Check(dateOfBirth, dateOfBirthLen - 1, dateOfBirth[dateOfBirthLen - 1])) {
            return false;
        }
        if (!MRZ::Check(dateOfExpiry, dateOfExpiryLen - 1, dateOfExpiry[dateOfExpiryLen - 1])) {
            return false;
        }
        return true;
    }

/**
 * @brief 查找文字区域
 * 
 * @param inMat 
 * @param ouMats 
 * @param NumberOfWords 
 * @param blockSize 
 * @param constValue 
 * @return int 
 */
    int Tool::FindText(Mat inMat, vector<Mat> *ouMats, int NumberOfWords, int blockSize, int constValue) {

        long long se, s;
        Mat frame;
        frame = inMat;

        se = s = Log::GetTime();
        Mat gray0(frame.size(), CV_8U), gray;
        if (frame.type() != CV_8U) {
            cvtColor(frame, gray0, COLOR_BGR2GRAY);
            LogI("CscTime", "cvtColor \t\t%lld ms", Log::GetTime() - s);

            s = Log::GetTime();
            cv::adaptiveThreshold(gray0, gray, 255, cv::AdaptiveThresholdTypes::ADAPTIVE_THRESH_MEAN_C,
                                  cv::ThresholdTypes::THRESH_BINARY_INV, blockSize, constValue);
            LogI("CscTime", "adaptiveThreshold \t%lld ms", Log::GetTime() - s);
        } else {
            gray = frame;
        }
        s = Log::GetTime();
        // 膨胀和腐蚀操作核设定
        Mat element1 = getStructuringElement(MORPH_RECT, Size(25, 6));
        //控制高度设置可以控制上下行的膨胀程度，例如3比4的区分能力更强,但也会造成漏检
        Mat element2 = getStructuringElement(MORPH_RECT, Size(20, 8));
        // 膨胀一次，让轮廓突出
        dilate(gray, gray0, element2);

        // 腐蚀一次，去掉细节，表格线等。这里去掉的是竖直的线
        erode(gray0, gray0, element1);

        // 再次膨胀，让轮廓明显一些
        dilate(gray0, gray0, element2);

        LogI("CscTime", "Find Text \t\t%lld ms", Log::GetTime() - s);

        s = Log::GetTime();
        vector<RotatedRect> rects;
        // 查找轮廓
        vector<vector<Point>> contours;
        vector<Vec4i> hierarchy;
        findContours(gray0, contours, hierarchy, RETR_CCOMP, CHAIN_APPROX_SIMPLE, Point(0, 0));
//         imshow("gray0",gray0);
//         waitKey();
        // 筛选那些面积小的
        for (int i = 0; i < contours.size(); i++) {
            RotatedRect rect;
            //计算当前轮廓的面积
            double area = contourArea(contours[i]);

            //面积小于1000的全部筛选掉
            if (area < 1000)
                continue;
            //轮廓近似，作用较小，approxPolyDP函数有待研究
            double epsilon = 0.001 * arcLength(contours[i], true);
            Mat approx;
            approxPolyDP(contours[i], approx, epsilon, true);

            //找到最小矩形，该矩形可能有方向
            rect = minAreaRect(contours[i]);

            //计算高和宽
            int m_width = rect.size.width;
            int m_height = rect.size.height;
            if (m_width < m_height) {
                m_width = rect.size.height;
                m_height = rect.size.width;
            }

            if (m_height * NumberOfWords * 2 / 3 > m_width)
                continue;
            //筛选那些太细的矩形，留下扁的
            if (m_height > m_width * 1.2)
                continue;
            if (m_height < (inMat.cols * 1 / 50))
                continue;
            //符合条件的rect添加到rects集合中
            try {
                s = Log::GetTime();

                //获取旋转矩形的四个顶点
                cv::Point2f vertices[4];
                rect.points(vertices);
                vector<Point2f> corners_trans(4);
                Point2f c = rect.center;

                vector<Point2f> l, r;
                for (int i = 0; i < 4; i++) {
                    if (vertices[i].x < c.x) {
                        l.push_back(vertices[i]);
                    } else {
                        r.push_back(vertices[i]);
                    }
                }
                if (l.size() != 2 || r.size() != 2)
                    continue;
                if (l[0].y < l[1].y) {
                    corners_trans[0] = l[0];
                    corners_trans[1] = l[1];
                } else {
                    corners_trans[1] = l[0];
                    corners_trans[0] = l[1];
                }
                if (r[0].y < r[1].y) {
                    corners_trans[2] = r[0];
                    corners_trans[3] = r[1];
                } else {
                    corners_trans[3] = r[0];
                    corners_trans[2] = r[1];
                }

                Mat out(m_height, m_width, CV_8UC1);

                vector<Point2f> corners(4);
                corners[0] = Point2f(0, 0);
                corners[2] = Point2f(m_width - 1, 0);
                corners[1] = Point2f(0, m_height - 1);
                corners[3] = Point2f(m_width - 1, m_height - 1);

                Mat transform = getPerspectiveTransform(corners_trans, corners);

                warpPerspective(gray, out, transform, out.size());

                // {
                //     Mat out1(m_height, m_width, CV_8UC3);
                //     warpPerspective(inMat, out1, transform, out1.size());
                //     char path[50];
                //     memset(path,0x00,50);
                //     sprintf(path,"image/%lld.bmp",Log::GetTime());
                //     imwrite(path,out1);
                //     LogD("CSC","Save %s",path);
                // }
                {
                    Mat out0 = out.clone();
                    s = Log::GetTime();
                     // 膨胀和腐蚀操作核设定
                    Mat element = getStructuringElement(MORPH_RECT, Size(2, 2));
//                     膨胀一次，让轮廓突出
                    dilate(out0, out0, element);

//                     腐蚀一次，去掉细节，表格线等。这里去掉的是竖直的线
                    erode(out0, out0, element);

////                     再次膨胀，让轮廓明显一些
//                    dilate(out0, out0, element);
//                     查找轮廓
                    vector<vector<Point>> contours1;
                    vector<Vec4i> hierarchy1;
                    findContours(out0, contours1, hierarchy1, RETR_CCOMP, CHAIN_APPROX_SIMPLE, Point(0, 0));

                    LogI("CscTime", "dilate \t\t\t%lld ms", Log::GetTime() - s);
                    if (contours1.size() < 10 + 7 + 7 || contours1.size() > 80) {
                        continue;
                    }
//                    imshow("out" + to_string(contours1.size()), out);
//                    imshow("out" + to_string(contours1.size()), out0);
//                    waitKey();
//                    destroyAllWindows();
                }

                // Mat img2 = gray(rect.boundingRect());
                LogI("CscTime", "Cut \t\t\t%lld ms", Log::GetTime() - s);
                ouMats->push_back(out);

                Mat out1;
                cv::flip(out, out1, 0);
                // imshow("out1",out1);
                // waitKey();
                ouMats->push_back(out1);
            }
            catch (...) {
            }
        }
        // imshow("gray0",gray0);
        // imshow("inMat",inMat);
        // waitKey();

        LogI("CscTime", "findContours \t\t%lld ms", Log::GetTime() - s);

        LogI("CscTime", "End \t\t\t%lld ms", Log::GetTime() - se);
        return static_cast<int>(ouMats->size());
    }

/**
 * @brief 查找 CSC 码
 * 
 * @param inMat 传入图片
 * @param serialNumber 证件号码
 * @param dateOfBirth 生日
 * @param dateOfExpiry 有效期
 * @param NumberOfWords 最小文字
 * @return true 
 * @return false 
 */
    int
    Tool::FindCSCText(Mat &inMat, char serialNumber[15], char dateOfBirth[9], char dateOfExpiry[9], int NumberOfWords) {
        Mat gray0;
        if (inMat.type() != CV_8U) {
            cvtColor(inMat, gray0, COLOR_BGR2GRAY);
        } else {
            gray0 = inMat;
        }

        blur(gray0, gray0, cv::Size(1, 1));
        //利用阈yu值二值化
        // cv::threshold(gray0, gray0, 55, 255, cv::THRESH_BINARY);
        // cv::adaptiveThreshold(gray0, gray0, 255, cv::AdaptiveThresholdTypes::ADAPTIVE_THRESH_MEAN_C,
        //                       cv::ThresholdTypes::THRESH_BINARY_INV, 21, 33);
        cv::adaptiveThreshold(gray0, gray0, 255, cv::AdaptiveThresholdTypes::ADAPTIVE_THRESH_MEAN_C,
                              cv::ThresholdTypes::THRESH_BINARY_INV, 25, 37);
        // cv::adaptiveThreshold(gray0, gray0, 255, cv::AdaptiveThresholdTypes::ADAPTIVE_THRESH_MEAN_C,
        //                       cv::ThresholdTypes::THRESH_BINARY_INV, 23, 35);
//        imshow("adaptiveThreshold", gray0);
//        waitKey(100);
        vector<Mat> ouMats;
        int size = Tool::FindText(gray0, &ouMats, NumberOfWords);
        vector<string> data;


        if (TesseractOcrThread(ouMats, &data)) {
            // char serialNumber[15], dateOfBirth[9], dateOfExpiry[9];
            for (auto &i : data) {
                LogD("CSC", "%s", i.c_str());
                int type = GetCSCKey(i, serialNumber, dateOfBirth, dateOfExpiry);
                if (type > 0) {
                    // LogD("CSC", "N[%s] B[%s] E[%s]", serialNumber, dateOfBirth, dateOfExpiry);
                    return type;
                }
            }
        }
        return 0;
    }

/**
 * @brief 港澳通行证
 * 
 */
    static regex HongKongMacauLaissezPasserRegx("[^C]*C\\S(\\S+)[<KLE](\\S{7})[<KLE](\\S{7})<.*\\s*");
/**
 * @brief 回乡证
 * 
 */
    static regex ReturnHomeCardRegx("C\\S(\\S+)\\S{3}(\\S{7})\\S{1}(\\S{7})");

/**
 * @brief 获取 密码
 * 
 * @param csc 可能存在 csc 码的字符串
 * @param serialNumber 证件号码
 * @param dateOfBirth 生日
 * @param dateOfExpiry 有效期
 * @return int 类型
 */
    int Tool::GetCSCKey(string csc, char serialNumber[15], char dateOfBirth[9], char dateOfExpiry[9]) {
        char _serialNumber[100], dateOfBirth1[10], dateOfExpiry1[10];
        memset(_serialNumber, 0x00, sizeof(_serialNumber));
        memset(dateOfBirth1, 0x00, sizeof(dateOfBirth1));
        memset(dateOfExpiry1, 0x00, sizeof(dateOfExpiry1));

        smatch r1;
        if (regex_match(csc, r1, HongKongMacauLaissezPasserRegx)) {
            memcpy(_serialNumber, r1[1].str().c_str(), r1[1].str().size());
            memcpy(dateOfBirth1, r1[3].str().c_str(), r1[3].str().size());
            memcpy(dateOfExpiry1, r1[2].str().c_str(), r1[2].str().size());

            int _serialNumberLen = static_cast<int>(strlen(_serialNumber));
            int dateOfBirth1Len = static_cast<int>(strlen(dateOfBirth1));
            int dateOfExpiry1Len = static_cast<int>(strlen(dateOfExpiry1));

            char n[15];
            memset(n, 0x00, sizeof(n));
            memcpy(n, &_serialNumber[strlen(_serialNumber) - 10], 10);

            if (Tool::CheckMRZ(n, dateOfBirth1, dateOfExpiry1)) {
                memset(serialNumber, 0x00, 15 * sizeof(char));
                memcpy(serialNumber, n, 9);

                memset(dateOfBirth, 0x00, 9 * sizeof(char));
                memcpy(dateOfBirth, dateOfBirth1, 6);

                memset(dateOfExpiry, 0x00, 9 * sizeof(char));
                memcpy(dateOfExpiry, dateOfExpiry1, 6);
                return 3;
            }
        }

        if (regex_match(csc, r1, ReturnHomeCardRegx) && r1.size() == 4) {
            memcpy(_serialNumber, r1[1].str().c_str(), r1[1].str().size());
            memcpy(dateOfBirth1, r1[3].str().c_str(), r1[3].str().size());
            memcpy(dateOfExpiry1, r1[2].str().c_str(), r1[2].str().size());

            int _serialNumberLen = strlen(_serialNumber);
            int dateOfBirth1Len = strlen(dateOfBirth1);
            int dateOfExpiry1Len = strlen(dateOfExpiry1);

            char n[15];
            memset(n, 0x00, sizeof(n));
            memcpy(n, &_serialNumber[strlen(_serialNumber) - 10], 10);

            if (Tool::CheckMRZ(n, dateOfBirth1, dateOfExpiry1)) {
                memset(serialNumber, 0x00, 15 * sizeof(char));
                memcpy(serialNumber, n, 9);

                memset(dateOfBirth, 0x00, 9 * sizeof(char));
                memcpy(dateOfBirth, dateOfBirth1, 6);

                memset(dateOfExpiry, 0x00, 9 * sizeof(char));
                memcpy(dateOfExpiry, dateOfExpiry1, 6);
                return 2;
            }
        }

        {
            char serialNumber1[60], _otherNumber[60], dateOfBirth2[10], dateOfExpiry2[10], n1[10], n2[10], n3[10];
            memset(n1, 0x00, sizeof(n1));
            memset(n2, 0x00, sizeof(n2));
            memset(n3, 0x00, sizeof(n3));
            memset(serialNumber1, 0x00, sizeof(serialNumber1));
            memset(dateOfBirth2, 0x00, sizeof(dateOfBirth2));
            memset(dateOfExpiry2, 0x00, sizeof(dateOfExpiry2));

            int ret = sscanf(csc.c_str(), "%10s%3s%7s%1s%7s%s", serialNumber1, n2, dateOfBirth2, n3, dateOfExpiry2,
                             _otherNumber);
            if (ret >= 5 && Tool::CheckMRZ(serialNumber1, dateOfBirth2, dateOfExpiry2)) {
                memcpy(serialNumber, serialNumber1, strlen(serialNumber1) - 1);
                serialNumber[strlen(serialNumber1) - 1] = 0;
                memcpy(dateOfBirth, dateOfBirth2, strlen(dateOfBirth2) - 1);
                dateOfBirth[strlen(dateOfBirth2) - 1] = 0;
                memcpy(dateOfExpiry, dateOfExpiry2, strlen(dateOfExpiry2) - 1);
                dateOfExpiry[strlen(dateOfExpiry2) - 1] = 0;
                return 1;
            }
        }

        return 0;
    }

    struct TesseractOcrThreadData {
        Mat *inMat;
        bool flag = false;
        string data;
        pthread_mutex_t *testlock = NULL;
        int index = 0;
    };

/**
 * @brief 多线程
 * 
 * @param arg 
 * @return void* 
 */
    void *TesseractOcrThread(void *arg) {
        TesseractOcrThreadData *data = (TesseractOcrThreadData *) arg;
        // Tool::SetRunInCpuCore(data->index);
        pthread_mutex_t *testlock = data->testlock;

        int ret = pthread_mutex_lock(testlock);
        if (ret != 0) {
            return nullptr;
        }
        data->flag = Tool::TesseractOcr(*data->inMat, &data->data, data->index);
        pthread_mutex_unlock(testlock);

        return nullptr;
    }

    bool Tool::TesseractOcrThread(vector<Mat> inMat, vector<string> *data) {
        if (inMat.empty())
            return false;
        auto *test_thread = new pthread_t[inMat.size()];
        auto da = new TesseractOcrThreadData[inMat.size()];
        int i;
        int l = static_cast<int>(inMat.size());
        for (i = 0; i < l; i++) {
            da[i].inMat = &inMat[i];
            da[i].flag = false;
            da[i].index = i % tessearct_api_len;
            da[i].testlock = &(tessearct_api_lock[i % tessearct_api_len]);
            pthread_create(&test_thread[i], nullptr, kekxv::TesseractOcrThread, (void *) &da[i]);
        }
        for (i = 0; i < l; i++) {
            void *status;
            int ret = pthread_join(test_thread[i], &status);
            if (ret != 0)
                LogE("pthread_join", "%d", ret);
            if (da[i].flag) {
                // LogD("TesseractOcr", "%s", da[i].data.c_str());
                data->push_back(da[i].data);
            }
        }

        delete[] test_thread;
        delete[] da;
        return !data->empty();
    }

/**
 * @brief 识别文字
 * 
 * @param inMat 被识别对象
 * @param inMat 识别结果
 * @param tessedit_char_whitelist 白名单 4.0.0 可能无效
 * @return bool 
 */
    bool Tool::TesseractOcr(Mat inMat, string *data, int index) {
        // imshow("inMat",inMat);
        // waitKey(0);

        *data = "";
        // tessearct_api[index].Clear();
        long s1 = Log::GetTime();
        tessearct_api[index].SetImage(inMat.data, inMat.cols, inMat.rows, 1, inMat.cols * 1);
        LogI("CscTime", "SetImage \t\t%lld ms", Log::GetTime() - s1);

        s1 = Log::GetTime();
        char *text = tessearct_api[index].GetUTF8Text();
        LogI("CscTime", "GetUTF8Text \t\t%lld ms", Log::GetTime() - s1);
        if (text == nullptr)return false;
        // const char *text = "E689451425CHN8404291M2602224MNPFASDSADSAD";

        string out = text;

        delete[] text;
        text = nullptr;
        tessearct_api[index].ClearAdaptiveClassifier();
        tessearct_api[index].Clear();

        // LogI("TesseractOcr", "%s", out.c_str());
        if (out.find('>') == string::npos) {
            DeleteSpase(out);
            *data = out;
            return true;
        }
        return false;
    }

/**
 * @brief 获取 CPU 核心数
 * 
 * @return int 
 */
    int Tool::GetCpuCore() {
        return static_cast<int>(sysconf(_SC_NPROCESSORS_CONF));
    }

/**
 * @brief Set the Run In Cpu Core object 设置在哪个cpu上跑线程
 * 
 * @param index 
 * @return true 
 * @return false 
 */
    bool Tool::SetRunInCpuCore(int index) {
        int CpuNum = GetCpuCore();
        index = index % CpuNum;
        cpu_set_t mask;                                      //CPU核的集合
        cpu_set_t get;                                       //获取在集合中的CPU
        CPU_ZERO(&mask);                                     //置空
        CPU_SET(index, &mask);                               //设置亲和力值
        if (sched_setaffinity(0, sizeof(mask), &mask) == -1) //设置线程CPU亲和力
        {
            LogE("Tool", "warning: could not set CPU affinity, continuing...");
            return false;
        }
        return true;
    }


} // namespace kekxv
