cmake_minimum_required(VERSION 2.8)
PROJECT(Tools)


file(GLOB SOURCE_FILES "*.cpp")
file(GLOB System_FILES "System/*.*pp")

# add_definitions("-DLogShow")
# add_definitions("-fPIC")

add_library(Tools ${SOURCE_FILES} ${System_FILES})         #生成静态链接库 libTools.a

find_package(OpenCV REQUIRED)
include_directories(. ${OpenCV_INCLUDE_DIRS})
