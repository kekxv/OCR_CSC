
#include <string.h>
#include <stdio.h>
#include "CSC.h"
#include "Tools/Tools.h"
#include "Tools/Log.h"

using namespace std;
using namespace cv;
using namespace kekxv;

long goodNum = 0, badNum = 0;

#ifdef __cplusplus
extern "C"
{
#endif
    int CSC_SetLevel(int level)
    {
        Log::setLevel(level);
    }
    int CSC_SetConsoleLevel(int level)
    {
        Log::setConsoleLevel(level);
    }
    /**
     * 初始化
     */
    int CSC_Init(const char* lang ,const char *faceXml)
    {
        int num_devices = cv::cuda::getCudaEnabledDeviceCount();
        LogI("CSC", "开启 GPU %d", num_devices);
        LogI("CSC", "CSC_Init");
        if (Tool::InitTesseract(lang) != 0)
        {
            return -1;
        }
        // InitFaceCascade(faceXml);

        Log::AddBlacklist("CscTime");
        return 0;
    }
    /**
     * 获取时间戳
     */
    long long CSC_GetTime()
    {
        return Log::GetTime();
    }


    int CSC_GetKey(
        unsigned char *image, int imageWidth, int imageHeight,int channels,
        char serialNumber[15], char dateOfBirth[9], char dateOfExpiry[9])
    {
        long long s = Log::GetTime();
        Mat frame = Mat(imageHeight, imageWidth, CV_MAKETYPE(CV_8U,channels), image);

        int type = Tool::FindCSCText(frame,serialNumber, dateOfBirth, dateOfExpiry,10);
        // if(type > 0){
        //     LogI("CSC", "T[%4s] N[%s] B[%s] E[%s]", (type>1?(type>2?"通行证":"回乡证"):"护照"),serialNumber, dateOfBirth, dateOfExpiry);
        // }

        LogI("CscTime", "TesseractOcr \t\t\t%lld ms", Log::GetTime() - s);
        return type;
    }

    /**
     * 获取当前进程 内存，虚拟内存，CPU占用
     */
    bool CSC_GetSystemInfo(unsigned int &ProcMem, unsigned int &ProcVirtualmem, float &ProcCpu)
    {
        unsigned int pid = Tool::SystemGetPid();
        if (pid == 0)
        {
            return false;
        }

        ProcMem = Tool::SystemGetProcMem(pid);
        ProcVirtualmem = Tool::SystemGetProcVirtualmem(pid);
        ProcCpu = Tool::SystemGetProcCpu(pid);
        return true;
    }
#ifdef __cplusplus
}
#endif