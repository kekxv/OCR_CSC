cmake_minimum_required(VERSION 2.8)
PROJECT(CSC)

# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -std=c++11")
add_definitions(
	-Wunused-result 
	-pthread 
    -std=c++11 
    -fPIC
    # -fopenmp
    )
set(CMAKE_CXX_STANDARD 14)

find_package(Tesseract REQUIRED)
include_directories(. ${Tesseract_INCLUDE_DIRS})



find_package(OpenCV REQUIRED)
include_directories(. ${OpenCV_INCLUDE_DIRS})


ADD_SUBDIRECTORY(Tools)

add_library(CSC
    SHARED
    CSC.cpp
)

if(DEFINED ARM64)
    set(libTesseract lib)
else()
#    set(libTesseract lib)
ENDIF(DEFINED ARM64)

target_link_libraries(CSC pthread Tools ${libTesseract}${Tesseract_LIBRARIES} ${OpenCV_LIBS})



set_target_properties(CSC PROPERTIES PREFIX "")



set(COPYITEM data)

########安装脚本##########
# 将生成的target安装到指定目录下，这里 HLST_Read_Device 是动态库,Example 是可执行程序,
# cmake会自动根据target的类型将可执行程序(如.exe)和动态库(如.dll)作为RUNTIME类型复制到bin下,
# 将动态库的导入库(.lib .a)作为ARCHIVE类型复制lib/static目录下
install(TARGETS CSC
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static)
install(TARGETS CSC
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION bin
        ARCHIVE DESTINATION bin)
# 复制 Example 代码到 Example 下
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${COPYITEM}/haarcascades DESTINATION bin
	PATTERN "${CMAKE_CURRENT_SOURCE_DIR}/${COPYITEM}/haarcascades/*"
	PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
	GROUP_EXECUTE GROUP_READ
    )
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${COPYITEM}/tessdata DESTINATION bin
    PATTERN "${CMAKE_CURRENT_SOURCE_DIR}/${COPYITEM}/tessdata/*"
    PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
    GROUP_EXECUTE GROUP_READ
    )

# 复制 Example 代码到 Example 下
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${COPYITEM}/haarcascades DESTINATION data
	PATTERN "${CMAKE_CURRENT_SOURCE_DIR}/${COPYITEM}/haarcascades/*"
	PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
	GROUP_EXECUTE GROUP_READ
    )
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${COPYITEM}/tessdata DESTINATION data
    PATTERN "${CMAKE_CURRENT_SOURCE_DIR}/${COPYITEM}/tessdata/*"
    PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
    GROUP_EXECUTE GROUP_READ
    )
