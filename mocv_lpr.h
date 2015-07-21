#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/video/video.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/ml/ml.hpp"

#include <vector>
#include <string>

/* Mastering OpenCV */
class Plate{
public:
    Plate();
    Plate(cv::Mat img, cv::RotatedRect rot);
    std::string str();
    cv::Rect position;
    cv::RotatedRect rotRect;
    cv::Mat plateImg;
    std::vector<char> chars;
    std::vector<cv::Rect> charsPos;
};

class DetectRegions{
public:
    DetectRegions();
    std::string filename;
    void setFilename(std::string f);
    bool saveRegions;
    bool showSteps;
    std::vector<Plate> run(cv::Mat input);
private:
    std::vector<Plate> segment(cv::Mat input);
    bool verifySizes(cv::RotatedRect mr);
    cv::Mat histeq(cv::Mat in);
};

#define HORIZONTAL    1
#define VERTICAL    0

class CharSegment{
	public:
    	CharSegment();
    	CharSegment(cv::Mat i, cv::Rect p);
    	cv::Mat img;
    	cv::Rect pos;
};
