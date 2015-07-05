#include <iostream>
#include <vector>
#include <string>

#include "detect.h"
#include "segment.h"
#include "recognize.h"

using namespace std;
using namespace cv;
void show_result(Mat img, Point2f pts[], string result) {
	cout << "Str: " << result << endl;
	for (int i = 0; i < 4; i++)
    	line(img, pts[i], pts[(i+1)%4], Scalar(0,255,0));
	imshow("Location", img);
	waitKey();
}
int main(int argc, char* argv[]) {
    Mat img, scaled_img, roi, resized_roi;
    vector<RotatedRect> plates;
	Point2f pts[4];
	double factor_img, factor_plate;
	string result;
    if (argc < 2) 
        cout << "Usage: ./lpdetect img_path" << endl;
	
	cout << "Detecting" << endl;
    img = imread(argv[1], 1),

    scaled_img = scale(img, factor_img);

	cout << "factor img " << factor_img << endl;
    plates = detect(scaled_img);

    write_plates_to_file(scaled_img, plates, "test_detect");

	// Scale plate to be compatible with segmenting algorithm	
	roi = scaled_img(plates[0].boundingRect());
	factor_plate = max(roi.size().width, roi.size().height)/300.0;
 	resize(roi, resized_roi, Size(0, 0), 1/factor_plate, 1/factor_plate, CV_INTER_CUBIC);
	
	cout << "Segmenting" << endl;
    // Preprocess
    Mat preproc = preprocess(resized_roi);

    // Segment
    vector<Rect> digits = segment(preproc);

    // Write to file
    write_digits_to_file(digits, preproc, "test_segment");

	cout << "Recognizing chars" << endl;
	// Recognize
	for (int i = 0; i < digits.size(); ++ i) {
		result += recognize_char(preproc(digits[i]));
	}

	plates[0].points(pts);

	// look for tl point
	int offset = 0;
	int mins = 12345;

	for (int i = 0; i < 4; ++ i) 
		if(mins > (pts[i].x + pts[i].y)) {
			mins = pts[i].x + pts[i].y;
			offset = i;
		}
	// scale to the input size
	for (int i = 0; i < 4; ++ i) {
		pts[i].x *= factor_img;
		pts[i].y *= factor_img;
	}

	show_result(img, pts, result);

	// output
	for (int i = 0; i < 4; ++ i) 
		cout << (int)pts[(i+offset)%4].x << "," << (int)pts[(i+offset+1)%4].y << ",";
	cout << result << endl;

    return 0;
}


