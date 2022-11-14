#include "radon.hpp"
#include <iostream>
#include <string>
#include <chrono>

using namespace std;

int main(){
    std::cout << "OpenCV version : " << CV_VERSION << std::endl;
    std::cout << "Radon transform test program" << std::endl;
    int n_threads = 1;
    
    //统计时间
    std::chrono::high_resolution_clock::time_point start, end;
    std::chrono::duration<double, std::milli> fp_ms;
    start = std::chrono::high_resolution_clock::now();

    //批量读取
    string filename = "E:\\OPEN-RADON\\src\\input_pic\\*.jpg";
    vector<cv::String> filesVec;
    cv::glob(filename, filesVec);
    for(int i=0;i<filesVec.size();i++){
        cv::Mat img = cv::imread(filesVec[i], cv::IMREAD_GRAYSCALE);
        //这里开始处理
        cv::resize(img, img, cv::Size(1024,1024));
        cv::Mat rad = cv::sinogram(img, 1024, n_threads);
        cv::imwrite("E:\\OPEN-RADON\\src\\output_pic\\"+ cv::format("%04d", i) + ".jpg", rad);
    }
   
    end = std::chrono::high_resolution_clock::now();
    fp_ms = end - start;
    std::cout << "Radon transform with " << n_threads << " threads took " << fp_ms.count() << " ms" << std::endl;

    //cv::Mat recon = cv::reconstruct(rad, cv::Size(img.cols, img.rows), n_threads);

    // cv::Mat compare(img.rows * 2, img.cols * 2, img.type());
    // cv::hconcat(img, rad, compare);

    //带窗口显示
    cv::namedWindow("Sinogram", cv::WINDOW_NORMAL);
    // cv::imshow("Sinogram", rad);
    // cv::imshow("Sinogram", recon);
    cv::waitKey(0);
    return 0;
}
