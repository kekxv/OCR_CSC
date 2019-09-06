#include "MRZ.h"

#include <string>

namespace kekxv
{
/*
     * 固定 排列
     */
int MRZ::weightingNumbers[] = {7, 3, 1};

/*
     * 获取对应 数值
     */
int MRZ::getNumericValues(char c)
{
    if (c == '<')
    {
        return 0;
    }
    if (c >= 'A')
    {
        return ((int)(c - 'A') + 10);
    }
    return (int)(c - '0');
}

/*
     * 获取校验后的MRZ字符串
     * 参数：
     * 	Mrz 		出参 校验后MRZ字符串
     * 	serialNumber 	入参 卡号
     * 	dateOfBirth 	入参 生日 年月日 各两位
     * 	dateOfExpiry 	入参 有效期 年月日 各两位
     */
void MRZ::GetMRZ(char *Mrz, int MrzSize, char *serialNumber, char *dateOfBirth, char *dateOfExpiry)
{
    int len = 0;
    int sum = 0;
    int serialNumberMrz = 0, dateOfBirthMrz = 0, dateOfExpiryMrz = 0;
    std::string nSerialNumber(serialNumber);
    len = strlen(serialNumber);
    if (len < 9)
    {
        for (int _i = len; _i < 9; _i++)
            nSerialNumber += "<";
    }
    sum = 0;
    for (int i = 0; i < len; i++)
    {
        sum += weightingNumbers[i % 3] * getNumericValues(serialNumber[i]);
    }
    serialNumberMrz = sum % 10;

    len = strlen(dateOfBirth);
    sum = 0;
    for (int i = 0; i < len; i++)
    {
        sum += weightingNumbers[i % 3] * getNumericValues(dateOfBirth[i]);
    }
    dateOfBirthMrz = sum % 10;

    len = strlen(dateOfExpiry);
    sum = 0;
    for (int i = 0; i < len; i++)
    {
        sum += weightingNumbers[i % 3] * getNumericValues(dateOfExpiry[i]);
    }
    dateOfExpiryMrz = sum % 10;
    snprintf(Mrz, MrzSize, "%s%d%s%d%s%d", nSerialNumber.c_str(), serialNumberMrz, dateOfBirth,
             dateOfBirthMrz, dateOfExpiry, dateOfExpiryMrz);
}

/**
	 * 检查 校验
	 */
bool MRZ::Check(const char *src, int len, char MRZ)
{
    int sum = 0;
    for (int i = 0; i < len; i++)
    {
        sum += weightingNumbers[i % 3] * getNumericValues(src[i]);
    }
    return ((MRZ - '0') == (char)(sum % 10));
}
/**
	 * 检查 校验
	 */
char MRZ::GetMRZChar(const char *src, int len)
{
    int sum = 0;
    for (int i = 0; i < len; i++)
    {
        sum += weightingNumbers[i % 3] * getNumericValues(src[i]);
    }
    return ((char)(sum % 10));
}
} // namespace kekxv