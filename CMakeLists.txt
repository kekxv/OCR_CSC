PROJECT(OCR_CSC)

cmake_minimum_required(VERSION 2.8)
# add_definitions(-g)
add_definitions(
	-Wunused-result 
	-pthread 
	-std=c++11
	-fPIC
	)

SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin)



find_library(dl dl)


find_package(OpenCV REQUIRED)
include_directories(. ${OpenCV_INCLUDE_DIRS})


# find_package(Tesseract REQUIRED)
# include_directories(${Tesseract_INCLUDE_DIRS})

# message( ${Tesseract_LIBRARIES})

find_package(Leptonica REQUIRED)
include_directories(. ${Leptonica_INCLUDE_DIRS})
find_package(Curses REQUIRED)
include_directories(. ${Curses_INCLUDE_DIRS})



add_custom_target(Example ALL DEPENDS CSC)
ADD_SUBDIRECTORY(Example)

ADD_SUBDIRECTORY(CSC)



# 复制源码中的指定的头文件到 include下
install(FILES ${PROJECT_SOURCE_DIR}/CSC/CSC.h DESTINATION include)

# 复制 Example 代码到 Example 下
install(DIRECTORY Example/ DESTINATION Example
	PATTERN "Example/*"
	PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
	GROUP_EXECUTE GROUP_READ)
