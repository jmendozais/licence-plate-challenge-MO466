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
//    Plate(cv::Mat img, cv::Rect pos);
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

class OCR{
    public:
        bool DEBUG;
        bool saveSegments;
        std::string filename;
        static const int numCharacters;
        static const char strCharacters[];
        OCR(std::string trainFile);
        OCR();
        std::string run(Plate *input);
        char run_on_char(cv::Mat ch);
        int charSize;
        cv::Mat preprocessChar(cv::Mat in);
        int classify(cv::Mat f);
        void train(cv::Mat trainData, cv::Mat trainClasses, int nlayers);
        int classifyKnn(cv::Mat f);
        void trainKnn(cv::Mat trainSamples, cv::Mat trainClasses, int k);
        cv::Mat features(cv::Mat input, int size);

    private:
        bool trained;
        std::vector<CharSegment> segment(Plate input);
        cv::Mat Preprocess(cv::Mat in, int newSize);        
        cv::Mat getVisualHistogram(cv::Mat *hist, int type);
        void drawVisualFeatures(cv::Mat character, cv::Mat hhist, cv::Mat vhist, cv::Mat lowData);
        cv::Mat ProjectedHistogram(cv::Mat img, int t);
        bool verifySizes(cv::Mat r);
        CvANN_MLP  ann;
        CvKNearest knnClassifier;
        int K;
};
