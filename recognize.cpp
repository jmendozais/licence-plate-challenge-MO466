#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <queue>
#include <stack>
#include <string>
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <iomanip>
#include <iterator>

#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "mocv_lpr.h"

using namespace std;
using namespace cv;

char recognize_char(Mat char_img) {
	Mat img_gray, rz_img;

    OCR ocr("OCR.xml");    
    ocr.saveSegments=true;
    ocr.DEBUG=false;
    ocr.filename="ocr_filenamewithoutext";
	//cvtColor(char_img, img_gray, CV_BGR2GRAY);
	img_gray = 255 - char_img;

    return ocr.run_on_char(img_gray);
}

string recognize_plate(Mat plate_img) {
	Mat plate_gray;
	Plate plate;

    OCR ocr("OCR.xml");    
    ocr.saveSegments=true;
    ocr.DEBUG=false;
    ocr.filename="ocr_filenamewithoutext";
	cvtColor(plate_img, plate_gray, CV_BGR2GRAY);
	plate_gray.convertTo(plate_gray, CV_8UC1);
	imshow("converted", plate_gray);
	waitKey();

	plate.plateImg = plate_gray;
    return ocr.run(&plate);
}

/*
int main(int argc, char* argv[])  {
    Mat img;
    if (argc < 2) 
        cout << "Usage: ./recognize img_path" << endl;

    img = imread(argv[1], 1);
	
    char c = recognize_char(img);
	//string results = recognize_plate(img);
    cout << "Result: " << c << endl;

    return 0;
}
*/
