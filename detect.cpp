#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/video/video.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>
#include <stdio.h>

using namespace std;
using namespace cv;

/** Function Headers */
void detectAndDisplay( Mat frame );

/** Global variables */
String plate_cascade_name = "hmodel4.xml";
CascadeClassifier plate_cascade;
String window_name = "LPR";

int main(int args, char* argv[]) 
{
    if (args < 2) 
        cout << "Usage: ./detect img_path" << endl;
    string input_path = argv[1];
    Mat input_img = imread(input_path, 1);
    detectAndDisplay(input_img);
}

void detectAndDisplay( Mat frame )
{
    std::vector<Rect> plates;
    Mat frame_gray;

    cvtColor( frame, frame_gray, COLOR_BGR2GRAY );
    equalizeHist( frame_gray, frame_gray );

    cout << "detecting" << endl;
    plate_cascade.detectMultiScale( frame_gray, plates, 1.1, 2, 0|CASCADE_SCALE_IMAGE, Size(30, 30) );

    cout << "end detection" << endl;
    for ( size_t i = 0; i < plates.size(); i++ )
    {
        stringstream ss;
        ss << "plates";
        ss << i;
        ss << ".jpg";
        string fout_name;
        ss >> fout_name;
        cout << "found " << fout_name << endl;

        Mat plate_roi = frame_gray(plates[i]);
        imwrite(fout_name, plate_roi);
    }
    imwrite("output.jpg", frame);
}

