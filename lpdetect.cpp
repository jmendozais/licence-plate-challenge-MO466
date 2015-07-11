#include <iostream>
#include <vector>
#include <string>

#include "detect.h"
#include "segment.h"
#include "recognize.h"

using namespace std;
using namespace cv;
void show_result(Mat img, Point2f pts[], string str) {
	cout << "Str: " << str << endl;
	for (int i = 0; i < 4; i++)
    	line(img, pts[i], pts[(i+1)%4], Scalar(0,255,0));
	imshow("Location", img);
	waitKey();
}

void output(Mat img, Point2f pts[], string str) {
	int offset = 0;
	int mins = 12345;
	Size size = img.size();

	// look for tl point
	for (int i = 0; i < 4; ++ i)
		if(mins > (pts[i].x + pts[i].y)) {
			mins = pts[i].x + pts[i].y;
			offset = i;
		}

	// fix if invalid coordinate
	for (int i = 0; i < 4; ++ i) {
		pts[i].x = max((int)pts[i].x, 0);
		pts[i].y = max((int)pts[i].y, 0);
		pts[i].x = min((int)pts[i].x, size.width);
		pts[i].y = min((int)pts[i].y, size.height);
	}

	for (int i = 0; i < 4; ++ i)
		cout << pts[(i+offset)%4].x << "," << pts[(i+offset)%4].y << ",";
	cout << str << endl;
}

int main(int argc, char* argv[]) {
    Mat img, scaled_img, roi, resized_roi;
    vector<RotatedRect> plates;
	vector<Rect> right_plates;
	Point2f pts[4];
	double factor_img, factor_plate;
	string result, model_path;

    if (argc < 2) 
        cout << "Usage: ./lpdetect img_path [model_path]" << endl;
	if (argc > 2)
		model_path = argv[2];
	else 
		model_path = "models/latest/cascade.xml";

    img = imread(argv[1], 1),
    scaled_img = scale(img, factor_img);
	
	//cout << "Detecting" << endl;
    plates = detect(scaled_img, model_path);
	right_plates = get_bounding_rects(scaled_img, plates);

	// No plates detected
	if (plates.size() == 0) {
		cout << "None" << endl;
		return 0;
	}

    //write_plates_to_file(scaled_img, right_plates, "test_detect");

	// Scale plate to be compatible with segmenting algorithm		
	roi = scaled_img(right_plates[0]);
	factor_plate = max(roi.size().width, roi.size().height)/500.0;
 	resize(roi, resized_roi, Size(0, 0), 1/factor_plate, 1/factor_plate, CV_INTER_CUBIC);
	
	//cout << "Segmenting" << endl;
    // Preprocess
    Mat preproc = preprocess(resized_roi);

	//cout << "Segment preproc..." << endl;
    // Segment
    vector<Rect> digits = segment(preproc);
	//show_result(resized_roi, digits);
	//waitKey();

    // Write to file
    //write_digits_to_file(digits, preproc, "test_segment");

	//cout << "Recognizing chars" << endl;
	// Recognize

	int k;
	KNearest knn = read_knn("ocr.xml", k);
	Mat character, feats;
	for (int i = 0; i < digits.size(); ++ i) {
		// digits with side < 4 cause segfault
		if (digits[i].width < 4 || digits[i].height < 4 )
			continue;
		character = preproc(digits[i]);
		feats = features(normalize(character));
		result += (char)knn.find_nearest(feats, k);
	}

	plates[0].points(pts);

	// scale to the input size
	for (int i = 0; i < 4; ++ i) {
		pts[i].x *= factor_img;
		pts[i].y *= factor_img;
	}
	if (result.size() == 0)
		result = "UNKNOWN";

	//show_result(img, pts, result);

	output(img, pts, result);

    return 0;
}
