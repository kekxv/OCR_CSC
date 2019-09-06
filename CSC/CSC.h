#ifndef CSC_H
#define CSC_H

#ifdef __cplusplus
extern "C"
{
#endif
    int CSC_SetLevel(int level = 0);
    int CSC_SetConsoleLevel(int level = 0);

    /**
     * 获取时间戳
     */
    long long CSC_GetTime();

    /**
     * 初始化
     */
    int CSC_Init(const char* lang = "OCR_B",const char *faceXml = "./haarcascades/haarcascade_frontalface_default.xml");

    /**
     * 获取 证件号码，出生日期，密码等
     */
    int CSC_GetKey(
        unsigned char *image, int imageWidth, int imageHeight,int channels,
        char serialNumber[15], char dateOfBirth[9], char dateOfExpiry[9]);
    /**
     * 获取当前进程 内存，虚拟内存，CPU占用
     */
    bool CSC_GetSystemInfo(unsigned int &ProcMem, unsigned int &ProcVirtualmem, float &ProcCpu);

#ifdef __cplusplus
}
#endif
#endif