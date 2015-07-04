#include <iostream>
#include <vector>
#include <string>

#include "detect.h"
#include "segment.h"
#include "recognize.h"

using namespace std;
using namespace cv;

int main(int argc, char* argv[]) {
    Mat img, scaled_img, roi, resized_roi;
    vector<Rect> plates;
	double factor;
	string result;
    if (argc < 2) 
        cout << "Usage: ./lpdetect img_path" << endl;
	
	cout << "Detecting" << endl;
    img = imread(argv[1], 1),

    scaled_img = scale(img);

    plates = detect(scaled_img);

    write_plates_to_file(scaled_img, plates, "test_detect");

	// Scale plate to be compatible with segmentating algorithm	
	roi = scaled_img(plates[0]);
	factor = max(roi.size().width, roi.size().height)/300.0;
 	resize(roi, resized_roi, Size(0, 0), 1/factor, 1/factor, CV_INTER_CUBIC);
	
	cout << "Segmenting" << endl;
    // Preprocess
    Mat preproc = preprocess(resized_roi);

    // Segment
    vector<Rect> digits = segment(preproc);

    // Write to file
    write_digits_to_file(digits, preproc, "test_segment");

	cout << "Recognizing chars" << endl;
	// Recognition
	for (int i = 0; i < digits.size(); ++ i) {
		result += recognize_char(preproc(digits[i]));
	}

	cout << argv[1] << " " << plates[0].x << ", " << plates[0].y << ", " << plates[0].x + plates[0].width << ", " << plates[0].y + plates[0].height << ", " << result << endl;
    return 0;
}


