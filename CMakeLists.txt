cmake_minimum_required(VERSION 3.18)
project(radon)
 
set(CMAKE_CXX_STANDARD 11)
# set(OpenCV_DIR "XXX") 
# xxxx目录包含OpenCVConfig.cmake
set(OpenCV_DIR "D:\\opencv-4.5.2")

# 寻找OpenCV库
find_package( OpenCV REQUIRED )
 
# 添加头文件
include_directories(radon PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include> ${OpenCV_INCLUDE_DIRS}
)
 
# 链接OpenCV库
add_executable(radon src/main.cpp src/radon.cpp)
target_link_libraries(radon ${OpenCV_LIBS})
