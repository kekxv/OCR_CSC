#include <stdlib.h>
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp>

using namespace std;
using namespace cv;

namespace kekxv
{
class Tool
{
public:
  /**
   * @brief 获取 进程 pid
   * 
   * @param process_name 
   * @param user 
   * @return unsigned int 
   */
  static unsigned int SystemGetPid(const char *process_name = nullptr, const char *user = nullptr);
  /**
   * @brief 获取进程占用内存
   * 
   * @param pid 
   * @return unsigned int 
   */
  static unsigned int SystemGetProcMem(unsigned int pid);
  /**
   * @brief 获取进程占用虚拟内存
   * 
   * @param pid 
   * @return unsigned int 
   */
  static unsigned int SystemGetProcVirtualmem(unsigned int pid);
  /**
   * @brief 获取CPU占用率
   * 
   * @param pid 
   * @return float 
   */
  static float SystemGetProcCpu(unsigned int pid);

  static double angle(cv::Point pt1, cv::Point pt2, cv::Point pt0);

  /**
   * @brief 初始化 OCR 识别
   * 
   * @param languagePath 
   * @param languageType 
   * @return int 
   */
  static int InitTesseract(const char *languageType = "OCR_B",const char *tessedit_char_whitelist = "ABCDEFGHIJKLMNOPQRSTUVWXYZ<>0123456789", const char *languagePath = "./tessdata");
  /**
   * @brief 初始化分类器
   * 
   * @param faceXml 
   * @return true 
   * @return false 
   */
  static bool InitFaceCascade(string faceXml);
  /**
   * @brief Get the Cpu Core object 获取 CPU 核心数
   * 
   * @return int 
   */
  static int GetCpuCore();
  /**
   * @brief Set the Run In Cpu Core object 设置在哪个cpu上跑线程
   * 
   * @param index 
   * @return true 
   * @return false 
   */
  static bool SetRunInCpuCore(int index = 0);
  /**
   * @brief 获取相似的数字
   * 
   * @return char 
   */
  static char GetNumAsChar(char c,char max = 0);
  /**
   * @brief 去掉 string 里面的空格
   * 
   * @param sNewTag 
   */
  static void DeleteSpase(string &sNewTag);
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
  static int FindText(Mat inMat,vector<Mat> *ouMats,int NumberOfWords = 22,int blockSize = 25, int constValue = 10);

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
  static int FindCSCText(Mat &inMat, char serialNumber[15], char dateOfBirth[9], char dateOfExpiry[9], int NumberOfWords = 10);
  /**
   * @brief 获取 密码
   * 
   * @param csc 可能存在 csc 码的字符串
   * @param serialNumber 证件号码
   * @param dateOfBirth 生日
   * @param dateOfExpiry 有效期
   * @return int 类型
   */
  static int GetCSCKey(string csc,char serialNumber[15], char dateOfBirth[9], char dateOfExpiry[9]);
  /**
   * @brief 识别文字
   * 
   * @param inMat 被识别对象
   * @param string 识别结果
   * @param tessedit_char_whitelist 白名单 4.0.0 可能无效
   * @return bool 
   */
  static bool TesseractOcr(Mat inMat, string *data,int index = 0);

  /**
   * @brief 多线程识别文字
   * 
   * @param inMat 被识别对象
   * @param string 识别结果
   * @param tessedit_char_whitelist 白名单 4.0.0 可能无效
   * @return bool 
   */
  static bool TesseractOcrThread(vector<Mat> inMat, vector<string> *data);

  /**
   * @brief 检查密码是否正确
   * 
   * @param serialNumber 证件号码
   * @param dateOfBirth 6位 生日
   * @param dateOfExpiry 6位 有效期
   * @return true 正确
   * @return false 错误
   */
  static bool CheckMRZ(char *serialNumber, char *dateOfBirth, char *dateOfExpiry);
};
} // namespace kekxv