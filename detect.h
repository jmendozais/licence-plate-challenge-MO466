#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/video/video.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>
#include <stdio.h>

cv::Mat scale(cv::Mat img);

std::vector<cv::Rect> detect(cv::Mat img);

void write_plates_to_file(cv::Mat img, std::vector<cv::Rect> plates, char name[]);

void show_detect_result(cv::Mat img, std::vector<cv::Rect> plates);

