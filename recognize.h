#include "opencv2/core/core.hpp"
#include "opencv2/ml/ml.hpp"

#include <string>
#include <vector>

// Read img paths

void read_set(char list_path[], std::vector<std::string> *img_paths, cv::Mat *responses);

// Extract features

cv::Mat normalize(cv::Mat img);

cv::Mat features(cv::Mat img);

cv::Mat features_from_paths(std::vector<std::string> img_paths);

// KNearest neighbors

cv::KNearest read_knn(std::string fname, int *k);

void write_knn(std::string fname, cv::Mat x, cv::Mat y, int k);

std::string recognize_with_knn(std::vector<cv::Rect> rois, cv::Mat plate);

// SVM

std::string recognize(std::vector<cv::Rect> rois, cv::Mat plate);

// Evaluate

double evaluate(cv::Mat real, cv::Mat pred);


