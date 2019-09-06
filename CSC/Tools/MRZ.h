#pragma once
#pragma execution_character_set("utf-8")

#include <stdio.h>
#include <string.h>
namespace kekxv
{
/*
	 * MRZ 工具类
	 */
class MRZ
{
  public:
	/*
			 * 获取校验后的MRZ字符串
			 * 参数：
			 * 	Mrz 		出参 校验后MRZ字符串
			 * 	serialNumber 	入参 卡号
			 * 	dateOfBirth 	入参 生日 年月日 各两位
			 * 	dateOfExpiry 	入参 有效期 年月日 各两位
			 */
	static void GetMRZ(char *Mrz, int MrzSize, char *serialNumber, char *dateOfBirth, char *dateOfExpiry);
	/**
	 * 检查 校验
	 */
	static bool Check(const char *src, int len, char MRZ);
	/**
	 * 获得 MRZ 校验
	 */
	static char GetMRZChar(const char *src, int len);

  private:
	/*
			 * 固定 排列
			 */
	static int weightingNumbers[];
	/*
			 * 获取对应 数值
			 */
	static int getNumericValues(char c);
};
} // namespace kekxv