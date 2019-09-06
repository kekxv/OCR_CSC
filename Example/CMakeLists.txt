
cmake_minimum_required(VERSION 2.8)


find_package(OpenCV REQUIRED)
include_directories(. ${OpenCV_INCLUDE_DIRS})

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})




# ADD_DEFINITIONS(-DVERSION=1)



add_executable(TestConsoleUI TestConsoleUI.cpp)
TARGET_LINK_LIBRARIES(TestConsoleUI CSC pthread ${CURSES_LIBRARIES} dl ${OpenCV_LIBS})


add_executable(CSC_Demo CSC_Demo.cpp)
TARGET_LINK_LIBRARIES(CSC_Demo CSC pthread ${CURSES_LIBRARIES} dl ${OpenCV_LIBS})


include_directories(
        .
        ../CSC/Tools/
)
add_executable(cscTest test.cpp)
TARGET_LINK_LIBRARIES(cscTest CSC ${CURSES_LIBRARIES} dl ${OpenCV_LIBS})


########安装脚本##########
# 将生成的target安装到指定目录下，这里 HLST_Read_Device 是动态库,Example 是可执行程序,
# cmake会自动根据target的类型将可执行程序(如.exe)和动态库(如.dll)作为RUNTIME类型复制到bin下,
# 将动态库的导入库(.lib .a)作为ARCHIVE类型复制lib/static目录下
install(TARGETS cscTest CSC_Demo TestConsoleUI
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static)
install(TARGETS cscTest CSC_Demo TestConsoleUI
		RUNTIME DESTINATION bin
		LIBRARY DESTINATION bin
		ARCHIVE DESTINATION bin)