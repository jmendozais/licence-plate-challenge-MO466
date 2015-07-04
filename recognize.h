#include <string>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

char recognize_char(cv::Mat char_img);

std::string recognize_plate(cv::Mat plate_img);
