# OCR_CSC

OpenCV + Tesseract-OCR3.5

## 交叉编译语句
    cmake -DCMAKE_TOOLCHAIN_FILE=~/Work/CompilationChain/ARM64.cmake ..
    
    cmake -DCMAKE_TOOLCHAIN_FILE=~/Work/CompilationChain/ARM64.cmake -DCMAKE_INSTALL_PREFIX=/home/caesar/Work/nfs/HLST_Read_Device -DCMAKE_BUILD_TYPE=Debug ../..

## 安装参数
    -DCMAKE_INSTALL_PREFIX=

## 电子护照 CSC 码识别
* 新增方法 识别率大概 85 有待提高

## 港澳通行证 CSC 码识别
* 识别率大概 95


## 回乡证 CSC 码识别
* 识别率大概 93
