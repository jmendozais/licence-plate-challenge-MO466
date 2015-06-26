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
        cout << "Ussage: ./detect img_path" << endl;
    string input_path = argv[1];
    Mat input_img = imread(input_path, 1);
    detectAndDisplay(input_img);
}

void detectAndDisplay( Mat frame )
{
    std::vector<Rect> faces;
    Mat frame_gray;

    cvtColor( frame, frame_gray, COLOR_BGR2GRAY );
    equalizeHist( frame_gray, frame_gray );

    cout << "detecting" << endl;
    plate_cascade.detectMultiScale( frame_gray, faces, 1.1, 2, 0|CASCADE_SCALE_IMAGE, Size(30, 30) );

    cout << "end detection" << endl;
    for ( size_t i = 0; i < faces.size(); i++ )
    {
        cout << "found " << i << endl;
        Point center( faces[i].x + faces[i].width/2, faces[i].y + faces[i].height/2 );
        ellipse( frame, center, Size( faces[i].width/2, faces[i].height/2 ), 0, 0, 360, Scalar( 255, 0, 255 ), 4, 8, 0 );
    }
    imwrite("output.jpg", frame);
}
