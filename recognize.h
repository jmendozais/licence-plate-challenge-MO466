#include <string>
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/ml/ml.hpp"

// Old mocv stufff

char recognize_char(cv::Mat char_img);

std::string recognize_plate(cv::Mat plate_img);

// Read img paths

void read_set(char list_path[], std::vector<std::string> &img_paths, cv::Mat &responses);

// Extract features

cv::Mat normalize(cv::Mat img);

cv::Mat features(cv::Mat img);

cv::Mat features_from_paths(std::vector<std::string> img_paths);

// KNearest neighbors persistency

cv::KNearest read_knn(std::string fname, int &k);

void write_knn(std::string fname, cv::Mat x, cv::Mat y, int k);

// Evaluate

double evaluate(cv::Mat real, cv::Mat pred);


